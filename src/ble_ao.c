#include "ble_ao.h"

#define BLE_STACK_SIZE 1024
#define BLE_AO_THREAD_PRIO -1
#define DEV_NAME CONFIG_BT_DEVICE_NAME
#define DEV_NAME_LEN (sizeof(DEV_NAME)-1)

LOG_MODULE_REGISTER(ble_ao, LOG_LEVEL_DBG);

/*
Zephyr BLE Stack Setup
*/

static struct bt_conn *current_conn;

void on_connected(struct bt_conn *conn, uint8_t ret) {
    if (ret) {
        LOG_ERR("Connection error! Retval: %d", ret);
        return;
    }
    LOG_INF("BT Connected!");
    current_conn = bt_conn_ref(conn);
    ao_publish(&(ao_event) { .id = BLE_CONNECTED_EVT, .data = 1 });
}

void on_disconnected(struct bt_conn *conn, uint8_t reason) {
    LOG_INF("BT disconnected (reason: %d)", reason);
    if (current_conn) {
        bt_conn_unref(current_conn);
        current_conn = NULL;
    }
    ao_publish(&(ao_event) { .id = BLE_DISCONNECTED_EVT, .data = 1 });
}

BT_CONN_CB_DEFINE(bt_cbs) = {
    .connected = on_connected,
    .disconnected = on_disconnected
};

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, DEV_NAME, DEV_NAME_LEN)
};

int start_advertising(void) {
    int err = bt_le_adv_start(BT_LE_ADV_CONN_FAST_1, ad, ARRAY_SIZE(ad), NULL, 0);
    if (err) {
        LOG_ERR("Could not start advertising the device via BLE. Retval: %d", err);
    }
    return err;
}

static void bt_ready_cb(int err) {
    if (err) {
        LOG_ERR("bt_enable failed. Retval: %d", err);
        return;
    }
    ao_publish(&(ao_event) { .id = BLE_START_ADVERTISING_EVT, .data = 1 });
}

/*
Active Object
*/

K_THREAD_STACK_DEFINE(ble_stack, BLE_STACK_SIZE);
static ble_active_object ble_ao;

static uint8_t queue_buf[sizeof(ao_event) * MAX_QUEUE_MSGS];

void ble_ao_init(void) {
    LOG_INF("Initializing BLE AO.");

    ao_subscribe(&ble_ao.ao, BLE_INIT_EVT);
    ao_subscribe(&ble_ao.ao, BLE_START_ADVERTISING_EVT);
    ao_subscribe(&ble_ao.ao, BLE_CONNECTED_EVT);
    ao_subscribe(&ble_ao.ao, BLE_DISCONNECTED_EVT);

    ble_ao.state = BLE_UNINITIALIZED_STATE;
    LOG_INF("After initial setting of state %d", ble_ao.state);

    active_object_init(&ble_ao.ao, &ble_ao_dispatch, BLE_AO_THREAD_PRIO, (k_thread_stack_t*)&ble_stack, K_THREAD_STACK_SIZEOF(ble_stack), queue_buf, sizeof(ao_event), 1);
    k_thread_name_set(&ble_ao.ao.thread, "ble_ao");

    ao_publish(&(ao_event) { .id = BLE_INIT_EVT, .data = 1 });
}


void ble_ao_dispatch(void *self, ao_event const *evt) {
    ble_active_object *ao = (ble_active_object *) self;
    LOG_INF("ble state: %d. Event ID: %d", ao->state, evt->id);
    int err = 0;
    switch(ao->state) {
        case BLE_UNINITIALIZED_STATE:
            switch(evt->id) {
                case BLE_INIT_EVT:
                    err = bt_enable(bt_ready_cb);
                    if (err < 0) {
                        LOG_ERR("Failed to Initialize BT Stack. Err: %d", err);
                        return;
                    }
                    ao->state = BLE_INITIALIZING_STATE;
                    break;
                default:
                    LOG_ERR("Unrecognized event in BLE_UNINITIALIZED_STATE state. Event id: %d", evt->id);
                    break;
            }
            break;
        case BLE_INITIALIZING_STATE:
            switch (evt->id) {
                case BLE_START_ADVERTISING_EVT:
                    err = start_advertising();
                    if (err < 0) {
                        LOG_ERR("Failed to start advertising. Err: %d", err);
                        return;
                    }
                    ao->state = BLE_ADVERTISING_STATE;
                    break;
                default:
                    LOG_ERR("Unrecognized event in BLE_INITIALIZING_STATE state. Event id: %d", evt->id);
            }
            break;
        case BLE_ADVERTISING_STATE:
            switch (evt->id) {
                case BLE_CONNECTED_EVT:
                    ao->state = BLE_CONNECTED_STATE;
                    break;
                default:
                    LOG_ERR("Unrecognized event in BLE_ADVERTISING_STATE state. Event id: %d", evt->id);
            }
            break;
        case BLE_CONNECTED_STATE:
            switch (evt->id) {
                case BLE_DISCONNECTED_EVT:
                    err = start_advertising();
                    if (err < 0) {
                        LOG_ERR("Failed to start advertising. Err: %d", err);
                        return;
                    }
                    ao->state = BLE_ADVERTISING_STATE;
                    break;
                default:
                    LOG_ERR("Unrecognized event in BLE_CONNECTED_STATE state. Event id: %d", evt->id);
            }
            break;
        default: 
            LOG_ERR("Ble AO is not in a valid state. State: %d", ao->state);
    }
}
