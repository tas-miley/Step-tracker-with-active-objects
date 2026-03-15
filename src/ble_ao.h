#pragma once

#include <zephyr/logging/log.h>
#include "active_object.h"
#include "ao_events.h"

typedef enum {
    BLE_IDLE = 1,
    BLE_ADVERTISING,
} ble_state;

typedef struct ble_active_object {
    active_object ao;
    int id;
    ble_state state;
    // add other stuff here
} ble_active_object;

void ble_ao_init(void);
void ble_ao_dispatch(void *self, ao_event const *evt);
