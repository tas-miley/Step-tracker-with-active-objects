#include "max30102.h"

LOG_MODULE_REGISTER(max30102, LOG_LEVEL_DBG);

static struct i2c_dt_spec max30102_dev = {
    .bus = DEVICE_DT_GET(DT_NODELABEL(i2c0)),
    .addr = 0x57
};

int max30102_init(void) {
    int err = 0;

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
        LOG_ERR("Unable to transmit reset message to MAX30102");
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
        LOG_ERR("Unable to transmit REG FIFO READ PTR to MAX30102");
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
        LOG_ERR("Unable to transmit REG FIFO WRITE PTR to MAX30102");
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
        LOG_ERR("Unable to transmit REG OVERFLOW COUNTER to MAX30102");
        return err;
    }
    if (rx_buf != 0) {
        LOG_ERR("MAX30102 failed to set REG OVERFLOW COUNTER. Value %d", rx_buf);
        return err;
    }

    LOG_INF("Successfully initialized MAX30102");
    return err;
}

