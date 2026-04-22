#pragma once

#include "active_object.h"

#define MAX_SUBSCRIPTIONS 10

typedef enum {
    IMU_INIT_EVT = 0,
    IMU_DATA_READY,
    BLE_INIT_EVT,
    BLE_START_ADVERTISING_EVT,
    BLE_CONNECTED_EVT,
    BLE_DISCONNECTED_EVT,
    BLE_DATA_READY,
    HRM_INIT_EVT,
    HRM_DATA_READY,
    MAX_EVENTS
} ao_event_id;

typedef struct ao_subscription {
    active_object *ao;
    ao_event_id evt_id;
} ao_subscription_t;

void ao_subscribe(active_object *self, ao_event_id evt_id);
void ao_unsubscribe(active_object *self, ao_event_id evt_id);
void ao_publish(const ao_event *evt_id);