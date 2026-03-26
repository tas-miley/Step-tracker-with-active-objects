#include "lsm6dso_shim.h"

LOG_MODULE_REGISTER(lsm6dso_shim, LOG_LEVEL_DBG);


static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *buf, uint16_t len) {
    const struct i2c_dt_spec *i2c_handle = (const struct i2c_dt_spec *) handle;
    return i2c_burst_write_dt(i2c_handle, reg, buf, len);
}

static int32_t platform_read(void *handle, uint8_t reg, uint8_t *buf, uint16_t len) {
    const struct i2c_dt_spec *i2c_handle = (const struct i2c_dt_spec *) handle;
    return i2c_burst_read_dt(i2c_handle, reg, buf, len);
}

void platform_init(void) {
    // initialize zephyr's i2c dev. 
}

static const struct i2c_dt_spec i2c_handle = I2C_DT_SPEC_GET(DT_ALIAS(imu));

static stmdev_ctx_t dev_ctx = {
    .write_reg = platform_write,
    .read_reg = platform_read,
    .handle = (void *)&i2c_handle, // cast i2c_dt_spec to void ptr explicitly to suppress warning about 'const' getting dropped.
};

static uint8_t whoamI, rst;
static int32_t err;
static uint16_t steps;


/* 
This initial spool up of the pedometer is derived from the example from ST.
https://github.com/STMicroelectronics/STMems_Standard_C_drivers/blob/master/lsm6dso_STdC/examples/lsm6dso_pedometer.c
*/
void pedometer_init(void) {
    lsm6dso_emb_sens_t emb_sens;

    LOG_INF("Inside of pedometer init");
    if (!i2c_is_ready_dt(&i2c_handle)) {
        LOG_ERR("I2C bus not ready.");
    }

    k_msleep(10); // sleep initial

    lsm6dso_device_id_get(&dev_ctx, &whoamI);

    if (whoamI != LSM6DSO_ID) {
        LOG_ERR("WHOAMI, %d, is not equal to %d", whoamI, LSM6DSO_ID);
    } 
    else {
        LOG_INF("WHOAMI, %d, is equal to %d", whoamI, LSM6DSO_ID);
    }


    do {
        lsm6dso_reset_get(&dev_ctx, &rst);
    } while (rst);

    /* Disable I3C interface */
    err = lsm6dso_i3c_disable_set(&dev_ctx, LSM6DSO_I3C_DISABLE);
    if (err != 0) {
        LOG_ERR("Failed to disable i3c interface.");
    }
    /* Set XL full scale */
    err = lsm6dso_xl_full_scale_set(&dev_ctx, LSM6DSO_2g);
    if (err != 0) {
        LOG_ERR("Failed to set XL full scale.");
    }
    /* Enable Block Data Update */
    err = lsm6dso_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);
    if (err != 0) {
        LOG_ERR("Failed to enable block data update.");
    }
    
    err = lsm6dso_xl_data_rate_set(&dev_ctx, LSM6DSO_XL_ODR_26Hz);
    if (err != 0) {
        LOG_ERR("Failed to enable xl sensor.");
    }
    /* Reset steps of pedometer */
    err = lsm6dso_steps_reset(&dev_ctx);
    if (err != 0) {
        LOG_ERR("Failed to reset the steps on the pedometer.");
    }
    /* Enable pedometer */
    err = lsm6dso_pedo_sens_set(&dev_ctx, LSM6DSO_FALSE_STEP_REJ_ADV_MODE);
    if (err != 0) {
        LOG_ERR("Failed to enable the pedometer.");
    }
    emb_sens.step = PROPERTY_ENABLE;
    emb_sens.step_adv = PROPERTY_ENABLE;
    err = lsm6dso_embedded_sens_set(&dev_ctx, &emb_sens);
    if (err != 0) {
        LOG_ERR("Failed to set the embedded sens.");
    }

    // while (1) {
    //     /* Read steps */
    //     lsm6dso_number_of_steps_get(&dev_ctx, &steps);
    //     LOG_INF("Steps: %d", steps);
    //     k_msleep(1000);
    // }
}