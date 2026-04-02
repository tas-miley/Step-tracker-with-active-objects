#include "imu_ao.h"

#define IMU_STACK_SIZE 1024
#define IMU_AO_THREAD_PRIO 1

LOG_MODULE_REGISTER(imu_ao, LOG_LEVEL_DBG);

K_THREAD_STACK_DEFINE(imu_stack, IMU_STACK_SIZE);
static imu_active_object imu_ao;

static uint8_t queue_buf[sizeof(ao_event) * MAX_QUEUE_MSGS];

static const struct gpio_dt_spec int1_gpio = GPIO_DT_SPEC_GET(DT_NODELABEL(st_lsm6dso), irq_gpios);
static struct gpio_callback int1_cb;

static void int1_isr(const struct device *port, struct gpio_callback *cb, gpio_port_pins_t pins) {
    static const ao_event evt = { .id = IMU_DATA_READY, .data = 0 };
    ao_publish(&evt);
}

// set up the INT1 gpio
int init_int1_isr(void) {
    int err = 0;
    err = gpio_pin_configure_dt(&int1_gpio, GPIO_INPUT);
    if (err < 0) {
        LOG_ERR("Error configuring INT1 as input. Error: %d", err);
        return err;
    }
    err = gpio_pin_interrupt_configure_dt(&int1_gpio, GPIO_INT_EDGE_RISING);
    if (err < 0) {
        LOG_ERR("Error configuring interrupt. Error: %d", err);
        return err;
    }
    gpio_init_callback(&int1_cb, int1_isr, BIT(int1_gpio.pin));
    err = gpio_add_callback_dt(&int1_gpio, &int1_cb);
    if (err < 0) {
        LOG_ERR("Error adding callback to INT1. Error: %d", err);
        return err;
    }
    return 0;
}

void imu_ao_init(void) {
    LOG_INF("Initializing IMU AO.");

    ao_subscribe(&imu_ao.ao, IMU_INIT_EVT);
    ao_subscribe(&imu_ao.ao, IMU_DATA_READY);
    imu_ao.state = IMU_INIT;

    active_object_init(&imu_ao.ao, &imu_ao_dispatch, IMU_AO_THREAD_PRIO, (k_thread_stack_t*)&imu_stack, K_THREAD_STACK_SIZEOF(imu_stack), queue_buf, sizeof(ao_event), 1);
    k_thread_name_set(&imu_ao.ao.thread, "imu_ao");

    ao_publish(&(const ao_event) { .id = IMU_INIT_EVT, .data = 1 });
}


void imu_ao_dispatch(void *self, ao_event const *evt) {
    imu_active_object *ao = (imu_active_object *) self;
    // LOG_INF("imu state: %d", ao->state);
    int err = 0;
    switch(ao->state) {
        case IMU_INIT:
            switch (evt->id) {
                case IMU_INIT_EVT:
                    LOG_INF("Initializing IMU.");
                    err = init_int1_isr();
                    if (err < 0) {
                        LOG_ERR("Failed to initialize INT1");
                        return;
                    }
                    err = pedometer_init();
                    if (err < 0) {
                        LOG_ERR("Failed to initialize INT1");
                        return;
                    }
                    ao->state = IMU_ACQUIRE;
                    break;
                default:
                    break;
            }
            break;
        case IMU_ACQUIRE:
            switch (evt->id) {
                case IMU_DATA_READY:
                    LOG_INF("IMU DATA READY");
                    int steps = read_step_counter();
                    LOG_INF("Steps: %d", steps);
                    if (steps < 0) {
                        LOG_ERR("Failed to read steps. Error %d", steps);
                        return;
                    }
                    ao_publish(&(ao_event){ .id = BLE_DATA_READY, .data = steps});
                    break;
                default:
                    break;
            }
            break;
        default: 
            LOG_ERR("IMU AO is not in a valid state. State: %d", ao->state);
    }
}
