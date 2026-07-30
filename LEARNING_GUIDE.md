# RTOS-Driven LoRaWAN Health Telemetry System - Learning Guide

This guide breaks down everything you need to know about this project so you can confidently explain how it works to a recruiter, professor, or peer.

## 1. System Overview
The core idea is to continuously monitor a patient's vital signs and stream that data to the cloud. We achieve this by bringing together three types of components:
*   **Sensors**: To read physical data (temperature, heartbeat, ECG).
*   **Microcontroller (ESP32)**: The 32-bit "brain" running FreeRTOS. It processes sensor data concurrently, manages hardware resources, and commands output devices.
*   **Output Devices**: An LCD and an OLED display for local metrics, and a LoRa Radio Transceiver (e.g., RFM95W) for broadcasting telemetry over unlicensed radio bands.

## 2. The Sensors & How They Work

### MAX30100 (Pulse Oximeter & Heart Rate)
*   **What it does:** Measures Heart Rate (BPM) and Blood Oxygen Saturation (SpO2).
*   **How it works:** It uses two LEDs (Red and Infrared) and a photodetector. Blood absorbs different amounts of light depending on its oxygen levels and the pulsing of blood through veins. By measuring the reflected light, it determines both heart rate and SpO2.
*   **Communication:** Uses the **I2C (Inter-Integrated Circuit)** protocol on ESP32 GPIO 21 (SDA) and GPIO 22 (SCL).

### DS18B20 (Temperature Sensor)
*   **What it does:** Measures body temperature.
*   **Communication:** Uses the **1-Wire** protocol. This means it only needs one data pin to communicate with the ESP32, along with VCC and GND. Multiple 1-Wire sensors can be connected to the exact same pin because each has a unique 64-bit ID.

### AD8232 (ECG Sensor)
*   **What it does:** Measures the electrical activity of the heart (Electrocardiogram).
*   **Communication:** It outputs an **Analog** signal. The ESP32 reads this varying voltage using its internal ADC (Analog-to-Digital Converter) on pin `GPIO36`. It also has digital pins (`LO+` and `LO-`) to detect if the electrodes have fallen off the patient ("Leads Off" detection).

## 3. The Output Devices

### 20x4 LCD and 128x64 OLED Displays
*   **What it does:** Shows real-time vitals locally.
*   **Communication:** They both use the **I2C** protocol. They share the SDA and SCL pins with the MAX30100 sensor. They don't interfere with each other because they all have different I2C addresses (e.g., the LCD is usually at address `0x27`, the OLED at `0x3C`, and the pulse sensor at another).

### LoRa Transceiver (e.g., RFM95W)
*   **What it does:** Broadcasts telemetry data securely over long distances using the LoRaWAN protocol.
*   **Communication:** Uses **SPI (Serial Peripheral Interface)**. SPI is a high-speed, synchronized communication bus that allows the ESP32 to quickly push payload data to the radio chip.
*   **Commands:** The `LMIC` (LoRa MAC in C) library handles all the complex radio timing, frequency hopping, and AES encryption required to join and transmit over The Things Network (TTN).

## 4. The Code Logic (FreeRTOS)

The code in `src/main.cpp` abandons the legacy `loop()` paradigm and uses a Real-Time Operating System (**FreeRTOS**).

1.  **Setup (`setup()`):**
    *   Initializes the I2C and SPI buses, LCD, sensors, and FreeRTOS queues.
    *   Spawns three distinct tasks and then deletes the main `loop()`.
2.  **TaskReadSensors:**
    *   Runs indefinitely. It polls the sensors (MAX30100, DS18B20, AD8232) and packages the vitals into a C++ `struct`.
    *   It safely passes this struct to a FreeRTOS `Queue`.
3.  **TaskUpdateDisplay:**
    *   This task "blocks" (waits efficiently) until new data appears in the queue.
    *   When data arrives, it updates the LCD and OLED screens.
4.  **TaskTelemetry:**
    *   Initializes the LoRa radio and handles LMIC radio events.
    *   It peeks at the latest data in the queue, dramatically compresses the struct down to a 6-byte hexadecimal array (because LoRaWAN has very strict payload size limits), and queues the payload for transmission to the nearest TTN gateway.

## 5. Potential Interview Questions to Prepare For

*   **"Why did you use FreeRTOS instead of a standard `loop()`?"**
    *   *Answer:* LoRaWAN transmissions can take unpredictable amounts of time due to radio duty cycles and network joining processes. By using FreeRTOS, the `TaskTelemetry` can yield to the OS while waiting for radio tasks, but the `TaskReadSensors` and `TaskUpdateDisplay` continue to run concurrently, ensuring the local displays and sensor polling never freeze.
*   **"How do the tasks safely share data?"**
    *   *Answer:* I used a FreeRTOS `Queue`. It's thread-safe. `TaskReadSensors` overwrites the single item in the queue with the latest struct, and the other tasks read/peek from it. This prevents race conditions where a task might read a struct while it's only halfway being updated.
*   **"Why did you use LoRaWAN instead of Cellular?"**
    *   *Answer:* The core problem involved continuous monitoring for elderly patients at home. Cellular requires recurring telecom subscription fees and uses a significant amount of battery power. LoRaWAN solves the same telemetry problem but operates over free, unlicensed radio bands and consumes a fraction of the power, solving both the cost and battery-life constraints.
