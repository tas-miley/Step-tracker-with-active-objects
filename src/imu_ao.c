#include "imu_ao.h"

#define IMU_STACK_SIZE 1024
#define IMU_AO_THREAD_PRIO 1

LOG_MODULE_REGISTER(imu_ao, LOG_LEVEL_DBG);

K_THREAD_STACK_DEFINE(imu_stack, IMU_STACK_SIZE);
static imu_active_object imu_ao;

static uint8_t queue_buf[sizeof(ao_event) * MAX_QUEUE_MSGS];

void imu_ao_init(void) {
    LOG_INF("Initializing IMU AO.");

    ao_subscribe(&imu_ao.ao, IMU_INIT_EVT);
    imu_ao.state = IMU_IDLE;

    active_object_init(&imu_ao.ao, &imu_ao_dispatch, IMU_AO_THREAD_PRIO, (k_thread_stack_t*)&imu_stack, K_THREAD_STACK_SIZEOF(imu_stack), queue_buf, sizeof(ao_event), 1);
    k_thread_name_set(&imu_ao.ao.thread, "imu_ao");

    ao_publish(&(ao_event) { .id = IMU_INIT_EVT, .data = 1 });
}


void imu_ao_dispatch(void *self, ao_event const *evt) {
    imu_active_object *ao = (imu_active_object *) self;
    LOG_INF("imu state: %d", ao->state);

    switch(ao->state) {
        case IMU_IDLE:
            switch (evt->id) {
                case IMU_INIT_EVT:
                    LOG_INF("TRying to call pedometer init");
                    pedometer_init();
                    int count = 0;
                    while (1) {
                        LOG_INF("Publishing data ready");
                        ao_publish(&(ao_event){ .id = IMU_DATA_READY, .data = count});
                        count++;
                        k_msleep(1000);
                    }
                    break;
                case IMU_DATA_READY:
                    LOG_INF("IMU EVENT 2.");
                    k_msleep(1000);
                    break;
                default:
                    LOG_INF("Unrecognized event. Not doing anything.");
                    k_msleep(1000);
            }
            break;
        case IMU_FETCH:
            break;
        default: 
            LOG_ERR("IMU AO is not in a valid state. State: %d", ao->state);
    }
}
