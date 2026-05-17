#include "max30102.h"

#define MAX_PTR_IDX 31
#define BYTES_PER_SAMPLE 6

LOG_MODULE_REGISTER(max30102, LOG_LEVEL_DBG);

static struct i2c_dt_spec max30102_dev = {
    .bus = DEVICE_DT_GET(DT_NODELABEL(i2c0)),
    .addr = 0x57
};

static const struct gpio_dt_spec int_gpio = GPIO_DT_SPEC_GET(DT_NODELABEL(max30102_int), gpios);
static struct gpio_callback int_cb;

static void int_isr(const struct device *port, struct gpio_callback *cb, gpio_port_pins_t pins) {
    static const ao_event evt = { .id = HRM_DATA_READY, .data = 0 };
    ao_publish(&evt);
}

// set up the INT gpio
int init_int_isr(void) {
    int err = 0;
    err = gpio_pin_configure_dt(&int_gpio, GPIO_INPUT);
    if (err < 0) {
        LOG_ERR("Error configuring INT as input. Error: %d", err);
        return err;
    }
    err = gpio_pin_interrupt_configure_dt(&int_gpio, GPIO_INT_EDGE_FALLING);
    if (err < 0) {
        LOG_ERR("Error configuring interrupt. Error: %d", err);
        return err;
    }
    gpio_init_callback(&int_cb, int_isr, BIT(int_gpio.pin));
    err = gpio_add_callback_dt(&int_gpio, &int_cb);
    if (err < 0) {
        LOG_ERR("Error adding callback to INT. Error: %d", err);
        return err;
    }
    return 0;
}

int max30102_init(void) {
    // general init
    int err = 0;

    err = init_int_isr();
    if (err != 0) {
        LOG_ERR("Unable to initialize GPIO INT pin for MAX30102");
        return err;
    }

    if (!i2c_is_ready_dt(&max30102_dev)) {
        LOG_ERR("Max30102 I2C not ready");
        return -ENODEV;
    }
    
    // reset MAX30102
    uint8_t tx_buf[2];
    tx_buf[0] = MAX30102_REG_MODE_CONFIG;
    tx_buf[1] = 1 << 6; // reset bit
    err = i2c_write_dt(&max30102_dev, tx_buf, sizeof(tx_buf));
    if (err != 0) {
        LOG_ERR("Unable to transmit reset message to MAX30102");
        return err;
    }
    // short delay to allow for reset
    k_msleep(10);

    // read MODE register to ensure reset has worked
    uint8_t rx_reg = MAX30102_REG_MODE_CONFIG;
    uint8_t rx_buf = 0;
    err = i2c_write_read_dt(&max30102_dev, &rx_reg, 1, &rx_buf, 1);
    if (err != 0) {
        LOG_ERR("Unable to transmit MODE CONFIG message to MAX30102");
        return err;
    }
    if (rx_buf != 0) {
        LOG_ERR("Reset of MAX30102 failed.");
        return err;
    }
    
    //check part ID
    rx_reg = MAX30102_REG_PART_ID;
    rx_buf = 0;
    err = i2c_write_read_dt(&max30102_dev, &rx_reg, 1, &rx_buf, 1);
    LOG_INF("Max30102 part ID 0x%02x", rx_buf);
    if (err != 0) {
        LOG_ERR("Unable to transmit read part ID message to MAX30102");
        return err;
    }
    if (rx_buf != 0x15) {
        LOG_ERR("Max30102 part ID not correct");
        return err;
    }

    // write FIFO READ PTR as 0.
    tx_buf[0] = MAX30102_REG_FIFO_READ_POINTER;
    tx_buf[1] = 0; // initialize ptr to zero position
    err = i2c_write_dt(&max30102_dev, tx_buf, sizeof(tx_buf));
    if (err != 0) {
        LOG_ERR("Unable to transmit FIFO READ PTR message to MAX30102");
        return err;
    }
    // check that FIFO READ PTR is 0.
    rx_reg = MAX30102_REG_FIFO_READ_POINTER;
    rx_buf = 0xFF; // ensure different from what is set.
    err = i2c_write_read_dt(&max30102_dev, &rx_reg, 1, &rx_buf, 1);
    if (err != 0) {
        LOG_ERR("Unable to transmit REG FIFO READ PTR message to MAX30102");
        return err;
    }
    if (rx_buf != 0) {
        LOG_ERR("MAX30102 failed to set FIFO READ PTR. Value %d", rx_buf);
        return err;
    }

    // write FIFO WRITE PTR as 0.
    tx_buf[0] = MAX30102_REG_FIFO_WRITE_POINTER;
    tx_buf[1] = 0; // initialize ptr to zero position
    err = i2c_write_dt(&max30102_dev, tx_buf, sizeof(tx_buf));
    if (err != 0) {
        LOG_ERR("Unable to transmit FIFO WRITE PTR message to MAX30102");
        return err;
    }
    // check that FIFO WRITE PTR is 0.
    rx_reg = MAX30102_REG_FIFO_WRITE_POINTER;
    rx_buf = 0xFF; // ensure different from what is set.
    err = i2c_write_read_dt(&max30102_dev, &rx_reg, 1, &rx_buf, 1);
    if (err != 0) {
        LOG_ERR("Unable to transmit REG FIFO WRITE PTR message to MAX30102");
        return err;
    }
    if (rx_buf != 0) {
        LOG_ERR("Max30102 failed to set FIFO WRITE PTR. Value %d", rx_buf);
        return err;
    }

    // write REG OVERFLOW COUNTER as 0.
    tx_buf[0] = MAX30102_REG_OVERFLOW_COUNTER;
    tx_buf[1] = 0; // initialize ptr to zero position
    err = i2c_write_dt(&max30102_dev, tx_buf, sizeof(tx_buf));
    if (err != 0) {
        LOG_ERR("Unable to transmit REG OVERFLOW COUNTER message to MAX30102");
        return err;
    }
    // check that REG OVERFLOW COUNTER is 0.
    rx_reg = MAX30102_REG_OVERFLOW_COUNTER;
    rx_buf = 0xFF; // ensure different from what is set.
    err = i2c_write_read_dt(&max30102_dev, &rx_reg, 1, &rx_buf, 1);
    if (err != 0) {
        LOG_ERR("Unable to transmit REG OVERFLOW COUNTER message to MAX30102");
        return err;
    }
    if (rx_buf != 0) {
        LOG_ERR("MAX30102 failed to set REG OVERFLOW COUNTER. Value %d", rx_buf);
        return err;
    }

    // init for this use case

    /*
    configure FIFO_CONFIG to be:
    8x averaging
    enable rollover
    17 / 32 samples = "almost full" ISR
    */ 
    tx_buf[0] = MAX30102_REG_FIFO_CONFIG;
    tx_buf[1] = (3 << 5) | (1 << 4) | 0x0F;
    err = i2c_write_dt(&max30102_dev, tx_buf, sizeof(tx_buf));
    if (err != 0) {
        LOG_ERR("Unable to transmit FIFO CONFIG message to MAX30102");
        return err;
    }

    /*
    configure SPO2_CONFIG to be:
    4096 nA full scale ADC range
    100 samples per second
    411 us pulse width
    */
    tx_buf[0] = MAX30102_REG_SPO2_CONFIG;
    tx_buf[1] = (2 << 5) | (1 << 2) | 0x03; // 4096nA range, 100sps, 411us pulse width
    err = i2c_write_dt(&max30102_dev, tx_buf, sizeof(tx_buf));
    if (err != 0) {
        LOG_ERR("Unable to transmit SPO2 CONFIG message to MAX30102");
        return err;
    }

    // set LED current draws
    tx_buf[0] = MAX30102_REG_LED_PULSE_1; // red
    tx_buf[1] = 0x24; // ~7 ma
    err = i2c_write_dt(&max30102_dev, tx_buf, sizeof(tx_buf));
    if (err != 0) {
        LOG_ERR("Unable to transmit RED LED PULSE message to MAX30102");
        return err;
    }
    tx_buf[0] = MAX30102_REG_LED_PULSE_2; // IR
    tx_buf[1] = 0x24; // ~7 ma
    err = i2c_write_dt(&max30102_dev, tx_buf, sizeof(tx_buf)); 
    if (err != 0) {
        LOG_ERR("Unable to transmit IR LED PULSE messageto MAX30102");
        return err;
    }

    // configure INT to trigger upon FIFO being almost full (17 samples)
    tx_buf[0] = MAX30102_REG_INTERRUPT_ENABLE_1; 
    tx_buf[1] = (1 << 7); // enable almost full INT
    err = i2c_write_dt(&max30102_dev, tx_buf, sizeof(tx_buf)); 
    if (err != 0) {
        LOG_ERR("Unable to transmit INTERRUPT ENABLE message to MAX30102");
        return err;
    }

    // set MODE to SPO2 mode
    tx_buf[0] = MAX30102_REG_MODE_CONFIG;
    tx_buf[1] = 0x03; // SPO2 mode
    err = i2c_write_dt(&max30102_dev, tx_buf, sizeof(tx_buf));
    if (err != 0) {
        LOG_ERR("Unable to transmit MODE CONFIG message to MAX30102");
        return err;
    }


    // For logging final configuration
    rx_reg = MAX30102_REG_INTERRUPT_STATUS_1;
    rx_buf = 0xFF; // ensure different from what is set.
    err = i2c_write_read_dt(&max30102_dev, &rx_reg, 1, &rx_buf, 1);
    if (err != 0) {
        LOG_ERR("Unable to transmit read INT STAT message to MAX30102");
        return err;
    }
    LOG_INF("INT STAT 1: %d", rx_buf);
    rx_reg = MAX30102_REG_INTERRUPT_ENABLE_1;
    rx_buf = 0xFF; // ensure different from what is set.
    err = i2c_write_read_dt(&max30102_dev, &rx_reg, 1, &rx_buf, 1);
    if (err != 0) {
        LOG_ERR("Unable to transmit read INT ENABLE message to MAX30102");
        return err;
    }
    LOG_INF("INT ENABLE 1: %d", rx_buf);

    rx_reg = MAX30102_REG_FIFO_CONFIG;
    rx_buf = 0xFF; // ensure different from what is set.
    err = i2c_write_read_dt(&max30102_dev, &rx_reg, 1, &rx_buf, 1);
    if (err != 0) {
        LOG_ERR("Unable to transmit read FIFO CONFIG message to MAX30102");
        return err;
    }
    LOG_INF("FIFO CONFIG: %d", rx_buf);

    rx_reg = MAX30102_REG_MODE_CONFIG;
    rx_buf = 0xFF; // ensure different from what is set.
    err = i2c_write_read_dt(&max30102_dev, &rx_reg, 1, &rx_buf, 1);
    if (err != 0) {
        LOG_ERR("Unable to transmit read MODE CONFIG message to MAX30102");
        return err;
    }
    LOG_INF("MODE CONFIG: %d", rx_buf);

    rx_reg = MAX30102_REG_SPO2_CONFIG;
    rx_buf = 0xFF; // ensure different from what is set.
    err = i2c_write_read_dt(&max30102_dev, &rx_reg, 1, &rx_buf, 1);
    if (err != 0) {
        LOG_ERR("Unable to transmit read SPO2 CONFIG message to MAX30102");
        return err;
    }
    LOG_INF("SPO2 CONFIG: %d", rx_buf);

    LOG_INF("Successfully initialized MAX30102");
    return err;
}

int max30102_get_fifo(uint32_t *red, uint32_t *ir, uint8_t *count) {
    LOG_DBG("Starting to read from FIFO");
    int err = 0;
    uint8_t rx_reg = 0;
    uint8_t rx_buf = 0;
    uint8_t rd_ptr = 0;
    uint8_t wr_ptr = 0;
    uint8_t raw[MAX_SAMPLES * BYTES_PER_SAMPLE];

    rx_reg = MAX30102_REG_INTERRUPT_STATUS_1;
    rx_buf = 0xFF; // ensure different from what is set.
    err = i2c_write_read_dt(&max30102_dev, &rx_reg, 1, &rx_buf, 1);
    if (err != 0) {
        LOG_ERR("Unable to transmit read INT STAT message to MAX30102");
        return err;
    }

    // check for missed samples
    rx_reg = MAX30102_REG_OVERFLOW_COUNTER;
    rx_buf = 0xFF; 
    err = i2c_write_read_dt(&max30102_dev, &rx_reg, 1, &rx_buf, 1);
    if (err != 0) {
        LOG_ERR("Unable to transmit REG OVERFLOW COUNTER message to MAX30102");
        return err;
    }
    if (rx_buf != 0) {
        LOG_ERR("MAX30102 internal FIFO overflow. Number of missed samples: %d", rx_buf);
    }

    // read READ PTR location
    rx_reg = MAX30102_REG_FIFO_READ_POINTER;
    rx_buf = 0xFF; // ensure different from what is set.
    err = i2c_write_read_dt(&max30102_dev, &rx_reg, 1, &rx_buf, 1);
    if (err != 0) {
        LOG_ERR("Unable to transmit REG FIFO READ PTR message to MAX30102");
        return err;
    }
    if (rx_buf > MAX_PTR_IDX) {
        LOG_ERR("READ PTR returned at invalid value. %d", rx_buf);
        return -EINVAL;
    }
    rd_ptr = rx_buf;

    // read WRITE PTR location
    rx_reg = MAX30102_REG_FIFO_WRITE_POINTER;
    rx_buf = 0xFF; // ensure different from what is set.
    err = i2c_write_read_dt(&max30102_dev, &rx_reg, 1, &rx_buf, 1);
    if (err != 0) {
        LOG_ERR("Unable to transmit REG FIFO WRITE PTR message to MAX30102");
        return err;
    }
    if (rx_buf > MAX_PTR_IDX) {
        LOG_ERR("WRITE PTR returned at invalid value. %d", rx_buf);
        return -EINVAL;
    }
    wr_ptr = rx_buf;

    // wrap at MAX_PTR_IDX
    *count = (wr_ptr - rd_ptr) & MAX_PTR_IDX;

    // read from FIFO
    rx_reg = MAX30102_REG_FIFO_DATA_REGISTER;
    err = i2c_write_read_dt(&max30102_dev, &rx_reg, 1, &raw, (*count) * BYTES_PER_SAMPLE);
    if (err != 0) {
        LOG_ERR("Unable to transmit REG FIFO READ PTR message to MAX30102");
        return err;
    }
    // each red or IR packet has 3 bytes. This is from bits 0-18, so a mask is applied. 
    for (int i = 0; i < (*count); ++i) {
        red[i] = ((uint32_t)raw[i * 6 + 0] << 16) |
                ((uint32_t)raw[i * 6 + 1] << 8) |
                ((uint32_t)raw[i * 6 + 2] << 0);
        red[i] &= 0x3FFFF;
        ir[i] = ((uint32_t)raw[i * 6 + 3] << 16) | 
                ((uint32_t)raw[i * 6 + 4] << 8) |
                ((uint32_t)raw[i * 6 + 5] << 0);
        ir[i] &= 0x3FFFF;
    }

    return err;
}