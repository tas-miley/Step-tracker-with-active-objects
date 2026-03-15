#pragma once

#include "active_object.h"

#define MAX_SUBSCRIPTIONS 10
#define MAX_EVENTS 4

typedef enum {
    IMU_EVENT = 0,
    IMU_EVENT_2,
    BLE_EVENT,
    BLE_EVENT_2
} ao_event_id;

typedef struct ao_subscription {
    active_object *ao;
    ao_event_id evt_id;
} ao_subscription_t;

void ao_subscribe(active_object *self, ao_event_id evt_id);
void ao_unsubscribe(active_object *self, ao_event_id evt_id);
void ao_publish(ao_event *evt_id);