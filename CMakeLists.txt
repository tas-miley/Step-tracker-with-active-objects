cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(StepTrackerProject)

set(ST_SENSOR_DRIVER_DIR ${ZEPHYR_HAL_ST_MODULE_DIR}/sensor/stmemsc/lsm6dso_STdC/driver)

message(${ST_SENSOR_DRIVER_DIR})

target_sources(app PRIVATE src/main.c
    src/ao_events.h src/ao_events.c
    src/active_object.h src/active_object.c 
    src/ble_ao.h src/ble_ao.c
    src/imu_ao.h src/imu_ao.c
    src/lsm6dso_shim.h src/lsm6dso_shim.c
    src/ble_pedometer_srv.h src/ble_pedometer_srv.c
)

target_include_directories(app PRIVATE
    ${ST_SENSOR_DRIVER_DIR}
)