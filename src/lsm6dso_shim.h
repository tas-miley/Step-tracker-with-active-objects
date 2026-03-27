#pragma once

#include <zephyr/kernel.h>
#include <lsm6dso_reg.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>

void pedometer_init(void);
int read_step_counter(void);