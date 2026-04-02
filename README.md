# Step Tracker with Active Objects

This project is an exploration of Active Objects as a firmware architecture technique. I learned about Active Objects through Miro Samek's Modern Embedded Systems Programming Course (https://www.youtube.com/watch?v=hnj-7XwTYRI&list=PLPW8O6W-1chwyTzI3BHwBLbGQoPFxPAPM).

To explore this, I built a BLE connected pedometer on the nRF52833 dk, using the LSM6DSOX IMU. I selected this IMU due to its high learning potential: I could use the built in pedometer, process the raw data on the SoC, or develop my own ML decision tree to put onto the ML core. I built the project on top of Zephyr RTOS because of familiarity, meaning I could jump into the actual Active Objects bit of the project sooner. 

## Hardware

- **MCU:** Nordic nRF52833 DK
- **IMU:** ST LSM6DSOX (I2C), using the hardware step detector with INT1 interrupt output
- **BLE:** onboard nRF52833 radio

## Architecture

The firmware is structured around a lightweight Active Object framework built from scratch on top of Zephyr RTOS primitives (`k_thread`, `k_msgq`). Each AO runs in its own thread and processes events from a message queue via a dispatch function. Because they're waiting for events almost all of the time, the MCU will fall back to Zephyr's Idle thread for the majority of runtime. This saves power, which is important in a device that could be a wearable. The AOs communicate through a publish/subscribe event bus rather than direct function calls, keeping them fully decoupled. 

**Data flow:**
1. LSM6DSOX hardware step detector fires INT1 on each detected step
2. INT1 ISR publishes `IMU_DATA_READY` to the event bus
3. IMU AO receives the event, reads the step counter via I2C, and publishes `BLE_DATA_READY` with the step count
4. BLE AO receives `BLE_DATA_READY` and updates the GATT characteristic, notifying any connected client
5. The BLE client reads the step count.

## Key Components

### Active Object Framework (`active_object.c/h`)
A minimal AO base implementation. Each AO wraps a Zephyr thread and message queue, with a user-supplied dispatch function that handles incoming events. The framework handles thread creation, queue init, and the blocking event loop.

### IMU AO (`imu_ao.c/h`)
Manages the LSM6DSOX lifecycle and step data acquisition. Configures the INT1 GPIO interrupt, initializes the pedometer mode via the `lsm6dso_shim`, and forwards step counts to the event bus on each interrupt. State machine: `IMU_INIT → IMU_ACQUIRE`.

### BLE AO (`ble_ao.c/h`)
Manages the full BLE connection lifecycle — stack init, advertising, connection, and disconnection. Handles reconnection automatically on disconnect. State machine: `BLE_UNINITIALIZED → BLE_INITIALIZING → BLE_ADVERTISING → BLE_CONNECTED`.

### GATT Service (`ble_pedometer_srv.c/h`)
Custom BLE service exposing two characteristics: step count and cadence. Both support read and notify. Shared state between the BLE AO and the GATT service uses `atomic_t` to ensure thread safety across BLE worker threads and the application threads. The GATT characteristic supports read and notify. If notification upon data acquisition is desired, call the ble_pedometer_notify function from the BLE AO.

### LSM6DSO Shim (`lsm6dso_shim.c/h`)
Zephyr does have the lsm6dso as a sensor in tree. However, the built in pedometer is not exposed via this interface. So, I decided to shim ST's `lsm6dso_reg.c/h` (https://github.com/STMicroelectronics/lsm6dso-pid/tree/f55167674138aca789c70a2970187c6322192dde) for use with Zephyr's I2C API. This preserves the already existing ST driver, still uses the Zephyr I2C interface, and allows me to not rewrite the direct LSM6DSO register manipulations.

## Status

Working:
- INT1 step detector interrupt -> ISR -> AO event pipeline
- Step counter read via I2C
- BLE advertising, connection, and reconnection
- GATT step count characteristic (read + notify scaffolding in place)

In progress:
- Testing & evaluation of LSM6DSOX's built in pedometer
- Battery level via ADC (currently stubbed at 100%)
- Cadence calculation
- PCB design for a standalone module

## Build

Developed with NCS v3.2.4. To build for the nRF52833 DK:

```bash
west build -b nrf52833dk/nrf52833
west flash
```