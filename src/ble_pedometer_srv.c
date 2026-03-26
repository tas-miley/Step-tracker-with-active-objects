#include "ble_pedometer_srv.h"

static bool notify_step_count;
static bool notify_cadence_count;

static void step_count_ccc_changed_cb(const struct bt_gatt_attr *attr, uint16_t value) {
    notify_step_count = (value == BT_GATT_CCC_NOTIFY);
}
static void cadence_ccc_changed_cb(const struct bt_gatt_attr *attr, uint16_t value) {
    notify_cadence_count = (value == BT_GATT_CCC_NOTIFY);
}

// TODO - add in read BT_GATT_CHRC_READ & read callbacks so central can sync w this device. 
BT_GATT_SERVICE_DEFINE(pedometer_service,
	BT_GATT_PRIMARY_SERVICE(BT_UUID_PEDOMETER_SERVICE),
	BT_GATT_CHARACTERISTIC(BT_UUID_STEP_COUNT,
					BT_GATT_CHRC_NOTIFY,
					BT_GATT_PERM_READ,
					NULL, NULL, NULL),
	BT_GATT_CCC(step_count_ccc_changed_cb, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
	BT_GATT_CHARACTERISTIC(BT_UUID_CADENCE,
					BT_GATT_CHRC_NOTIFY,
					BT_GATT_PERM_READ,
					NULL, NULL, NULL),
	BT_GATT_CCC(cadence_ccc_changed_cb, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE)
);