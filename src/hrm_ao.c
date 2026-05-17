#include "hrm_ao.h"

#define HRM_STACK_SIZE 1024
#define HRM_AO_THREAD_PRIO 1

LOG_MODULE_REGISTER(hrm_ao, LOG_LEVEL_DBG);

K_THREAD_STACK_DEFINE(hrm_stack, HRM_STACK_SIZE);
static hrm_active_object hrm_ao;

static uint8_t queue_buf[sizeof(ao_event) * MAX_QUEUE_MSGS];

static uint32_t red_data[MAX_SAMPLES];
static uint32_t ir_data[MAX_SAMPLES];
static uint8_t counted_samples = 0;

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
    int err = 0;
    switch(ao->state) {
        case HRM_INIT:
            switch (evt->id) {
                case HRM_INIT_EVT:
                    LOG_INF("Initializing HRM.");
                    max30102_init();
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
                    LOG_INF("DATA READY!");
                    max30102_get_fifo(red_data, ir_data, &counted_samples);
                    LOG_INF("Just read data. Samples counted: %d", counted_samples);
                    for (int i = 0; i < counted_samples; ++i) {
                        LOG_INF("FIFO Data. Red: %d, IR: %d, position %d", red_data[i], ir_data[i], i);
                    }
                    break;
                default:
                    break;
            }
            break;
        default: 
            LOG_ERR("HRM AO is not in a valid state. State: %d", ao->state);
    }
}