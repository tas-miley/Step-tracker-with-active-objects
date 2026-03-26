#pragma once

#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
// #include <zephyr/bluetooth/services/bas.h>
// #include <zephyr/bluetooth/services/hrs.h>

// service UUID
#define BT_UUID_PEDOMETER_VALUE BT_UUID_128_ENCODE(0x7387ecd8, 0x012b, 0x4e95, 0x93b9, 0x995d1244b057)

// Step count characteristic
#define BT_UUID_STEP_COUNT_VALUE BT_UUID_128_ENCODE(0x5d6029fc, 0x6696, 0x4232, 0x8e7e, 0x73272edde04d)

// Cadence characteristic
#define BT_UUID_CADENCE_VALUE BT_UUID_128_ENCODE(0x76a115e1, 0x8774, 0x4c1b, 0xb305, 0x69efcc2ab8c6)

#define BT_UUID_PEDOMETER_SERVICE   BT_UUID_DECLARE_128(BT_UUID_PEDOMETER_VALUE)
#define BT_UUID_STEP_COUNT          BT_UUID_DECLARE_128(BT_UUID_STEP_COUNT_VALUE)
#define BT_UUID_CADENCE             BT_UUID_DECLARE_128(BT_UUID_CADENCE_VALUE)

