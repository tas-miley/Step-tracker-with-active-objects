# Wearable Health Tracker

This project is an exploration of Active Objects as a firmware architecture technique. I learned about Active Objects through Miro Samek's Modern Embedded Systems Programming Course (https://www.youtube.com/watch?v=hnj-7XwTYRI&list=PLPW8O6W-1chwyTzI3BHwBLbGQoPFxPAPM).

To explore this, I built a BLE connected wearable health tracker on the nRF52833 DK, using the LSM6DSOX IMU for step detection and the MAX30102 PPG sensor for heart rate and SpO2. I built the project on top of Zephyr RTOS because of familiarity, meaning I could focus on the Active Objects architecture rather than platform bring-up.

## Hardware

- **MCU:** Nordic nRF52833 DK
- **IMU:** ST LSM6DSOX (I2C), using the hardware step detector with INT1 interrupt output
- **PPG Sensor:** Maxim MAX30102 (I2C), SPO2 mode for heart rate and SpO2 acquisition
- **BLE:** onboard nRF52833 radio

## Architecture

The firmware is structured around a lightweight Active Object framework built from scratch on top of Zephyr RTOS primitives (`k_thread`, `k_msgq`). Each AO runs in its own thread and processes events from a message queue via a dispatch function. Because they're waiting for events almost all of the time, the MCU will fall back to Zephyr's idle thread for the majority of runtime. This saves power, which is important in a wearable device. The AOs communicate through a publish/subscribe event bus rather than direct function calls, keeping them fully decoupled.

**Step detection data flow:**
1. LSM6DSOX hardware step detector fires INT1 on each detected step
2. INT1 ISR publishes `IMU_DATA_READY` to the event bus
3. IMU AO receives the event, reads the step counter via I2C, and publishes `BLE_DATA_READY` with the step count
4. BLE AO receives `BLE_DATA_READY` and updates the GATT characteristic, notifying any connected client

**HRM data flow:**
1. MAX30102 FIFO almost-full interrupt fires on the INT pin
2. ISR posts `HRM_DATA_READY` event to the HRM AO queue
3. HRM AO reads the FIFO via I2C, receiving raw red and IR ADC samples

TODO:
4. HRM AO runs peak detection on the IR channel to derive heart rate, and a ratio-of-ratios algorithm on red and IR to derive SpO2
5. HRM AO publishes `HRM_RESULT_EVENT` with current HR and SpO2 values
6. BLE AO receives `HRM_RESULT_EVENT` and updates the GATT characteristics

## Key Components

### Active Object Framework (`active_object.c/h`)
A minimal AO base implementation. Each AO wraps a Zephyr thread and message queue, with a user-supplied dispatch function that handles incoming events. The framework handles thread creation, queue init, and the blocking event loop.

### IMU AO (`imu_ao.c/h`)
Manages the LSM6DSOX lifecycle and step data acquisition. Configures the INT1 GPIO interrupt, initializes the pedometer mode via the `lsm6dso_shim`, and forwards step counts to the event bus on each interrupt. State machine: `IMU_INIT → IMU_ACQUIRE`.

### HRM AO (`hrm_ao.c/h`)
Manages the MAX30102 lifecycle and PPG data acquisition. Configures the INT GPIO interrupt, reads raw red and IR samples from the FIFO on each almost-full interrupt, and runs on-device signal processing to derive heart rate and SpO2. 

### MAX30102 Driver (`max30102.c/h`)
Hardware abstraction for the MAX30102 PPG sensor. Handles register configuration (mode, FIFO, SPO2 config, LED amplitudes), FIFO pointer management, and burst FIFO reads. Exposes a clean interface returning arrays of raw 18-bit red and IR ADC counts. Operates in SPO2 mode (red + IR), which provides all data needed for both heart rate and SpO2 without mode switching.

### BLE AO (`ble_ao.c/h`)
Manages the full BLE connection lifecycle — stack init, advertising, connection, and disconnection. Handles reconnection automatically on disconnect. State machine: `BLE_UNINITIALIZED → BLE_INITIALIZING → BLE_ADVERTISING → BLE_CONNECTED`.

### GATT Service (`ble_pedometer_srv.c/h`)
Custom BLE service exposing step count and cadence characteristics. Both support read and notify. Shared state between the BLE AO and the GATT service uses `atomic_t` to ensure thread safety across BLE worker threads and application threads.

### LSM6DSO Shim (`lsm6dso_shim.c/h`)
Zephyr has the LSM6DSO as an in-tree sensor, but its built-in pedometer is not exposed via the sensor API. This shim bridges ST's `lsm6dso_reg.c/h` (https://github.com/STMicroelectronics/lsm6dso-pid/tree/f55167674138aca789c70a2970187c6322192dde) to Zephyr's I2C API, preserving the existing ST driver while avoiding a full register-level rewrite.

## Status

Working:
- INT1 step detector interrupt → ISR → AO event pipeline
- LSM6DSO onboard pedometer read via I2C
- BLE advertising, connection, and reconnection
- GATT step count characteristic (read + notify)
- MAX30102 init and configuration (SPO2 mode, FIFO, LED amplitudes)
- MAX30102 FIFO read via almost-full interrupt → HRM AO event pipeline
- Raw red and IR ADC sample acquisition

In progress:
- On-device HR peak detection from IR channel
- SpO2 algorithm
- HRM GATT characteristic (read + notify)
- Battery level via ADC (currently stubbed at 100%)
- Cadence calculation
- PCB design for a standalone wrist-worn module
