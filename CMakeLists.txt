cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(StepTrackerProject)

target_sources(app PRIVATE src/main.c
    src/ao_events.h src/ao_events.c
    src/active_object.h src/active_object.c 
    src/ble_ao.h src/ble_ao.c
    src/imu_ao.h src/imu_ao.c
)
