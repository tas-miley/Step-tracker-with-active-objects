#include "ble_ao.h"

#define BLE_STACK_SIZE 1024
#define BLE_AO_THREAD_PRIO 1

LOG_MODULE_REGISTER(ble_ao, LOG_LEVEL_DBG);

K_THREAD_STACK_DEFINE(ble_stack, BLE_STACK_SIZE);
static ble_active_object ble_ao;

static uint8_t queue_buf[sizeof(ao_event) * MAX_QUEUE_MSGS];

void ble_ao_init(void) {
    LOG_INF("Initializing BLE AO.");
    active_object_init(&ble_ao.ao, &ble_ao_dispatch, BLE_AO_THREAD_PRIO, (k_thread_stack_t*)&ble_stack, K_THREAD_STACK_SIZEOF(ble_stack), queue_buf, sizeof(ao_event), 1);
    k_thread_name_set(&ble_ao.ao.thread, "ble_ao");
    ble_ao.state = BLE_IDLE;
    ao_subscribe(&ble_ao.ao, BLE_EVENT_2);
}


void ble_ao_dispatch(void *self, ao_event const *evt) {
    ble_active_object *ao = (ble_active_object *) self;
    LOG_INF("ble state: %d", ao->state);

    switch(ao->state) {
        case BLE_IDLE:
            switch (evt->id) {
                case BLE_EVENT:
                    LOG_INF("BLE EVENT 1.");
                    k_msleep(1000);
                    break;
                case BLE_EVENT_2:
                    LOG_INF("BLE EVENT 2.");
                    k_msleep(1000);
                    break;
                default:
                    LOG_INF("Unrecognized event. Not doing anything.");
                    k_msleep(1000);
            }
            break;
        case BLE_ADVERTISING:
            break;
        default: 
            LOG_ERR("Ble AO is not in a valid state. State: %d", ao->state);
    }
}