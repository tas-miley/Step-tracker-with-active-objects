#pragma once

#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include "active_object.h"
#include "ao_events.h"

typedef enum {
    HRM_INIT = 1,
    HRM_ACQUIRE,
} hrm_state;

typedef struct hrm_active_object {
    active_object ao;
    int id;
    hrm_state state;
    // add other stuff here
} hrm_active_object;

void hrm_ao_init(void);
void hrm_ao_dispatch(void *self, ao_event const *evt);
