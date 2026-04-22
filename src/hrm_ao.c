#include "hrm_ao.h"

#define HRM_STACK_SIZE 1024
#define HRM_AO_THREAD_PRIO 1

LOG_MODULE_REGISTER(hrm_ao, LOG_LEVEL_DBG);

K_THREAD_STACK_DEFINE(hrm_stack, HRM_STACK_SIZE);
static hrm_active_object hrm_ao;

static uint8_t queue_buf[sizeof(ao_event) * MAX_QUEUE_MSGS];

// TODO - change this to INT gpio for MAX30102
static const struct gpio_dt_spec int_gpio = GPIO_DT_SPEC_GET(DT_NODELABEL(st_lsm6dso), irq_gpios);
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
    err = gpio_pin_interrupt_configure_dt(&int_gpio, GPIO_INT_EDGE_RISING);
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

void hrm_ao_init(void) {
    LOG_INF("Initializing HRM AO.");

    ao_subscribe(&hrm_ao.ao, HRM_INIT_EVT);
    ao_subscribe(&hrm_ao.ao, HRM_DATA_READY);
    hrm_ao.state = HRM_INIT;

    active_object_init(&hrm_ao.ao, &hrm_ao_dispatch, HRM_AO_THREAD_PRIO, (k_thread_stack_t*)&hrm_stack, K_THREAD_STACK_SIZEOF(hrm_stack), queue_buf, sizeof(ao_event), 1);
    k_thread_name_set(&hrm_ao.ao.thread, "hrm_ao");

    ao_publish(&(const ao_event) { .id = HRM_INIT_EVT, .data = 1 });
}


void hrm_ao_dispatch(void *self, ao_event const *evt) {
    hrm_active_object *ao = (hrm_active_object *) self;
    // LOG_INF("hrm state: %d", ao->state);
    int err = 0;
    switch(ao->state) {
        case HRM_INIT:
            switch (evt->id) {
                case HRM_INIT_EVT:
                    LOG_INF("Initializing HRM.");
                    err = init_int_isr();
                    if (err < 0) {
                        LOG_ERR("Failed to initialize INT1");
                        return;
                    }
                    ao->state = HRM_ACQUIRE;
                    break;
                default:
                    break;
            }
            break;
        case HRM_ACQUIRE:
            switch (evt->id) {
                case HRM_DATA_READY:
                    // TODO - add real data pass to BLE
                    break;
                default:
                    break;
            }
            break;
        default: 
            LOG_ERR("HRM AO is not in a valid state. State: %d", ao->state);
    }
}
