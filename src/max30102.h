#pragma once

#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/i2c.h>

#define MAX30102_REG_INTERRUPT_STATUS_1          0x00        /**< interrupt status 1 register */
#define MAX30102_REG_INTERRUPT_STATUS_2          0x01        /**< interrupt status 2 register */
#define MAX30102_REG_INTERRUPT_ENABLE_1          0x02        /**< interrupt enable 1 register */
#define MAX30102_REG_INTERRUPT_ENABLE_2          0x03        /**< interrupt enable 2 register */
#define MAX30102_REG_FIFO_WRITE_POINTER          0x04        /**< fifo write pointer register */
#define MAX30102_REG_OVERFLOW_COUNTER            0x05        /**< overflow counter register */
#define MAX30102_REG_FIFO_READ_POINTER           0x06        /**< fifo read pointer register */
#define MAX30102_REG_FIFO_DATA_REGISTER          0x07        /**< fifo data register */
#define MAX30102_REG_FIFO_CONFIG                 0x08        /**< fifo config register */
#define MAX30102_REG_MODE_CONFIG                 0x09        /**< mode config register */
#define MAX30102_REG_SPO2_CONFIG                 0x0A        /**< spo2 config register */
#define MAX30102_REG_LED_PULSE_1                 0x0C        /**< led pulse amplitude 1 register */
#define MAX30102_REG_LED_PULSE_2                 0x0D        /**< led pulse amplitude 2 register */
#define MAX30102_REG_MULTI_LED_MODE_CONTROL_1    0x11        /**< multi led mode control 1 register */
#define MAX30102_REG_MULTI_LED_MODE_CONTROL_2    0x12        /**< multi led mode control 2 register */
#define MAX30102_REG_DIE_TEMP_INTEGER            0x1F        /**< die temperature integer register */
#define MAX30102_REG_DIE_TEMP_FRACTION           0x20        /**< die temperature fraction register */
#define MAX30102_REG_DIE_TEMP_CONFIG             0x21        /**< die temperature config register */
#define MAX30102_REG_REVISION_ID                 0xFE        /**< revision id register */
#define MAX30102_REG_PART_ID                     0xFF        /**< part id register */

int max30102_init(void);