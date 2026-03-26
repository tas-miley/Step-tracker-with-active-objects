#include "ble_pedometer_srv.h"

LOG_MODULE_REGISTER(ble_service, LOG_LEVEL_DBG);

static uint32_t step_count;
static uint32_t cadence_spm;

static bool notify_step_count;
static bool notify_cadence;

static void step_count_ccc_changed_cb(const struct bt_gatt_attr *attr, uint16_t value) {
    notify_step_count = (value == BT_GATT_CCC_NOTIFY);
}
static void cadence_ccc_changed_cb(const struct bt_gatt_attr *attr, uint16_t value) {
    notify_cadence = (value == BT_GATT_CCC_NOTIFY);
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

void ble_pedometer_notify(struct bt_conn *conn, uint32_t steps, uint16_t cadence) {
    step_count = steps;
    cadence_spm = cadence;

    if (!notify_step_count || !conn) {
        return;
    }
    LOG_WRN("Notifiying steps, %d", steps);
    int err = bt_gatt_notify(conn,
                             &pedometer_service.attrs[2],
                             &step_count,
                             sizeof(step_count));
    if (err) {
        LOG_WRN("Step notify failed (err %d)", err);
    }
    if (!notify_cadence || !conn) {
        return;
    }
    err = bt_gatt_notify(conn,
                   &pedometer_service.attrs[5],
                   &cadence_spm,
                   sizeof(cadence_spm));
    if (err) {
        LOG_WRN("Cadence notify failed (err %d)", err);
    }
}
