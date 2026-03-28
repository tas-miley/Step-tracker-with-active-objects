#include "ble_pedometer_srv.h"

LOG_MODULE_REGISTER(ble_service, LOG_LEVEL_DBG);

static atomic_t step_count;
static atomic_t cadence_spm;
static uint32_t dummy_buf1;
static uint32_t dummy_buf2;

static bool notify_step_count;
static bool notify_cadence;

static void step_count_ccc_changed_cb(const struct bt_gatt_attr *attr, uint16_t value) {
    notify_step_count = (value == BT_GATT_CCC_NOTIFY);
}
static void cadence_ccc_changed_cb(const struct bt_gatt_attr *attr, uint16_t value) {
    notify_cadence = (value == BT_GATT_CCC_NOTIFY);
}

static ssize_t read_step_count(struct bt_conn *conn,
                               const struct bt_gatt_attr *attr,
                               void *buf, uint16_t len, uint16_t offset)
{
    uint32_t steps = (uint32_t)atomic_get(&step_count);
    return bt_gatt_attr_read(conn, attr, buf, len, offset,
                             &steps, sizeof(steps));
}

static ssize_t read_cadence(struct bt_conn *conn,
                             const struct bt_gatt_attr *attr,
                             void *buf, uint16_t len, uint16_t offset)
{
    uint32_t cadence = (uint32_t)atomic_get(&cadence_spm);
    return bt_gatt_attr_read(conn, attr, buf, len, offset,
                             &cadence, sizeof(cadence));
}

BT_GATT_SERVICE_DEFINE(pedometer_service,
	BT_GATT_PRIMARY_SERVICE(BT_UUID_PEDOMETER_SERVICE),
	BT_GATT_CHARACTERISTIC(BT_UUID_STEP_COUNT,
					BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
					BT_GATT_PERM_READ,
					read_step_count, NULL, &dummy_buf1),
	BT_GATT_CCC(step_count_ccc_changed_cb, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
	BT_GATT_CHARACTERISTIC(BT_UUID_CADENCE,
					BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
					BT_GATT_PERM_READ,
					read_cadence, NULL, &dummy_buf2),
	BT_GATT_CCC(cadence_ccc_changed_cb, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE)
);

void ble_pedometer_notify(struct bt_conn *conn, uint32_t steps, uint16_t cadence) {
    atomic_set(&step_count, (atomic_val_t)steps);
    atomic_set(&cadence_spm, (atomic_val_t)cadence);

    if (!notify_step_count || !conn) {
        return;
    }
    LOG_WRN("Notifiying steps, %d", steps);
    int err = bt_gatt_notify(conn,
                             &pedometer_service.attrs[2],
                             &steps,
                             sizeof(steps));
    if (err) {
        LOG_WRN("Step notify failed (err %d)", err);
    }
    if (!notify_cadence || !conn) {
        return;
    }
    err = bt_gatt_notify(conn,
                   &pedometer_service.attrs[5],
                   &cadence,
                   sizeof(cadence));
    if (err) {
        LOG_WRN("Cadence notify failed (err %d)", err);
    }
}

void update_steps(uint32_t steps) {
    atomic_set(&step_count, (atomic_val_t)steps);
}

void update_cadence(uint32_t cadence) {
    atomic_set(&cadence_spm, (atomic_val_t)cadence);
}