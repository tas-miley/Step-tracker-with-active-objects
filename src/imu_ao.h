#pragma once

#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include "active_object.h"
#include "ao_events.h"
#include "lsm6dso_shim.h"

typedef enum {
    IMU_INIT = 1,
    IMU_ACQUIRE,
} imu_state;

typedef struct imu_active_object {
    active_object ao;
    int id;
    imu_state state;
    // add other stuff here
} imu_active_object;

void imu_ao_init(void);
void imu_ao_dispatch(void *self, ao_event const *evt);
