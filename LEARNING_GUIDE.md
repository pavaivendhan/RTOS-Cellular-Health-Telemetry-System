# RTOS-Driven Cellular Health Telemetry System - Learning Guide

This guide breaks down everything you need to know about this project so you can confidently explain how it works to a recruiter, professor, or peer.

## 1. System Overview
The core idea is to continuously monitor a patient's vital signs and automatically alert a caretaker if those signs indicate danger. We achieve this by bringing together three types of components:
*   **Sensors**: To read physical data (temperature, heartbeat, ECG).
*   **Microcontroller (Arduino)**: The "brain" that processes the sensor data, checks it against thresholds, and commands the output devices.
*   **Output Devices**: An LCD and an OLED display for local metrics, and a GSM Modem for remote SMS alerts.

## 2. The Sensors & How They Work

### MAX30100 (Pulse Oximeter & Heart Rate)
*   **What it does:** Measures Heart Rate (BPM) and Blood Oxygen Saturation (SpO2).
*   **How it works:** It uses two LEDs (Red and Infrared) and a photodetector. Blood absorbs different amounts of light depending on its oxygen levels and the pulsing of blood through veins. By measuring the reflected light, it can determine both heart rate and SpO2.
*   **Communication:** It uses the **I2C (Inter-Integrated Circuit)** protocol. I2C uses two wires (SDA for data, SCL for clock) allowing multiple devices to connect to the same pins.

### DS18B20 (Temperature Sensor)
*   **What it does:** Measures body temperature.
*   **How it works:** It’s a digital sensor that measures temperature very accurately and sends the data digitally to the Arduino.
*   **Communication:** It uses the **1-Wire** protocol. This means it only needs one data pin to communicate with the Arduino, along with VCC and GND. Multiple 1-Wire sensors can be connected to the exact same pin because each has a unique 64-bit ID.

### AD8232 (ECG Sensor)
*   **What it does:** Measures the electrical activity of the heart (Electrocardiogram).
*   **How it works:** When the heart beats, it generates a tiny electrical signal. The AD8232 uses electrodes placed on the skin to detect these micro-signals, amplifies them, and filters out noise (like the 50/60Hz noise from power lines).
*   **Communication:** It outputs an **Analog** signal. The Arduino reads this varying voltage using its internal ADC (Analog-to-Digital Converter) on pin `A0`. It also has digital pins (`LO+` and `LO-`) to detect if the electrodes have fallen off the patient ("Leads Off" detection).

## 3. The Output Devices

### 20x4 LCD and 128x64 OLED Displays
*   **What it does:** Shows real-time vitals locally.
*   **Communication:** They both use the **I2C** protocol. Notice how they share the SDA and SCL pins with the MAX30100 sensor. They don't interfere with each other because they all have different I2C addresses (e.g., the LCD is usually at address `0x27`, the OLED at `0x3C`, and the pulse sensor at another).

### GSM Modem (SIM800/SIM900)
*   **What it does:** Connects to the cellular network to send SMS messages.
*   **Communication:** Uses **UART (Serial Communication)**. Since the Arduino UNO only has one hardware serial port (pins 0 and 1, used for USB debugging), we use a library called `SoftwareSerial`. This allows us to use standard digital pins (like 7 and 8) to act as a serial port to talk to the GSM module.
*   **Commands:** The Arduino talks to the GSM module using **AT Commands** (e.g., `AT+CMGF=1` to set text mode, `AT+CMGS` to send a message).

## 4. The Code Logic (The "Brain")

The code inside `HealthMonitor.ino` follows a classic embedded systems loop:

1.  **Setup (`setup()`):**
    *   Starts communication with the PC (for debugging).
    *   Initializes the LCD, GSM module, and sensors.
    *   If a sensor fails to initialize (like the Pulse Oximeter), it will print an error message.
2.  **Continuous Loop (`loop()`):**
    *   It checks the time. We don't want to read sensors every single millisecond (it's inefficient). We use a "non-blocking delay" with `millis()` to check the sensors every 2 seconds.
    *   **Read Phase:** It pulls the latest data from the MAX30100, DS18B20, and AD8232.
    *   **Display Phase:** It updates both the LCD and OLED screens with the fresh numbers.
    *   **Logic Phase:** It compares the readings to the thresholds defined in `config.h`.
        *   *If Temp > 38.0 OR Heart Rate > 100 OR Heart Rate < 60*, it flags an alert.
    *   **Action Phase:** If an alert is flagged and hasn't been sent yet, it executes `sendSMSAlert()`, firing off AT commands to the GSM modem.

## 5. Potential Interview Questions to Prepare For

*   **"Why did you use SoftwareSerial instead of Hardware Serial for the GSM module?"**
    *   *Answer:* The Arduino UNO only has one hardware UART (pins 0 & 1), which is shared with the USB connection. I wanted to keep the USB free for debugging (printing `Serial.println`) and uploading code, so I used SoftwareSerial on pins 7 and 8 for the GSM module.
*   **"How does the system handle multiple I2C devices (the LCD and Pulse Sensor)?"**
    *   *Answer:* I2C is a bus protocol. Every device on the bus has a unique hexadecimal address. The microcontroller acts as the 'master' and specifies which address it wants to talk to, so the devices don't clash.
*   **"What happens if the ECG pads fall off?"**
    *   *Answer:* The AD8232 has built-in "Leads Off" detection. The code reads digital pins connected to `LO+` and `LO-`. If either is HIGH, it means a pad is disconnected, and the code outputs "Leads Off" instead of garbage data.
