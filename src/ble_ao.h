#pragma once

#include <zephyr/logging/log.h>
#include "active_object.h"
#include "ao_events.h"
#include "ble_pedometer_srv.h"

typedef enum {
    BLE_UNINITIALIZED_STATE = 1,
    BLE_INITIALIZING_STATE,
    BLE_ADVERTISING_STATE,
    BLE_CONNECTED_STATE
} ble_state;

typedef struct ble_active_object {
    active_object ao;
    int id;
    ble_state state;
    // add other stuff here
} ble_active_object;

void ble_ao_init(void);
void ble_ao_dispatch(void *self, ao_event const *evt);
int start_advertising(void);