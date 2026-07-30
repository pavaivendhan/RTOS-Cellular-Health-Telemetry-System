# Project Summary: RTOS-Driven LoRaWAN Health Telemetry System

## 1. Core Problems & Solutions

| Problem Identified | How It Is Solved |
| :--- | :--- |
| **Delayed Emergency Response:** Elderly patients living alone suffer from fatal delays in transmitting critical health status during cardiac events or sudden illness. | **Real-Time Telemetry:** The system continuously monitors vitals and instantly transmits data over the air, completely bypassing the need for manual patient intervention. |
| **High Cost of Connectivity:** Traditional remote monitoring relies on expensive cellular subscriptions or broadband Wi-Fi, making long-term use financially restrictive. | **LoRaWAN Integration:** Replaced the cellular modem with a LoRaWAN radio. It transmits data over free, unlicensed radio bands to a local gateway, eliminating all recurring telecom fees. |
| **Frequent Battery Recharging:** Continuous cellular transmission or Wi-Fi drains wearable batteries rapidly, making the device impractical for elderly users who may forget to charge it. | **Low-Power Architecture:** LoRaWAN uses a fraction of the power of cellular. Combined with FreeRTOS, the device sleeps efficiently between transmissions, drastically extending battery life. |
| **System Freezing/Blocking:** Old Arduino loops freeze the entire system while waiting for a network connection, preventing the local display or sensors from updating in real-time. | **FreeRTOS Concurrency:** The software is split into isolated "Tasks" (Sensors, Display, Telemetry). If the network is slow, only the Telemetry task waits; the sensors and display continue updating instantly. |

---

## 2. Hardware & Sensors

| Component | Function | Why it is used |
| :--- | :--- | :--- |
| **ESP32 Microcontroller** | The central "brain" of the device. | It has a 32-bit dual-core processor capable of running an RTOS natively, unlike the 8-bit Arduino UNO. |
| **MAX30100** | Pulse Oximeter. | Accurately reads Heart Rate (BPM) and Blood Oxygen saturation (SpO2) using red/IR light absorption in the blood. |
| **DS18B20** | Temperature Sensor. | A highly precise digital thermometer to detect hypothermia or high fever (infection indicators). |
| **AD8232** | ECG Sensor. | Measures the electrical activity of the heart to detect arrhythmias or cardiac events. Built-in "Leads Off" detection ensures pads are attached. |
| **RFM95W (or similar)** | LoRa Transceiver. | The physical radio chip used to broadcast the telemetry data over long distances using minimal power. |
| **LCD & OLED Screens** | Local UI Displays. | Provides immediate, visual feedback to the patient or nearby caretaker without requiring a smartphone app. |

---

## 3. Communication Protocols

| Protocol | Where it is used | Why it is used |
| :--- | :--- | :--- |
| **LoRaWAN** | ESP32 → TTN (Cloud Gateway) | **Long Range Wide Area Network.** Chosen for its massive range (up to 10 miles), zero transmission cost, and ultra-low power consumption. |
| **I2C** | ESP32 → MAX30100, LCD, OLED | **Inter-Integrated Circuit.** A two-wire bus that allows multiple devices (like the two screens and pulse sensor) to share the same two pins on the ESP32, saving space. |
| **SPI** | ESP32 → LoRa Transceiver | **Serial Peripheral Interface.** A high-speed, synchronized communication bus perfect for pushing data to the radio module quickly. |
| **1-Wire** | ESP32 → DS18B20 Temp | A specialized protocol requiring only a single data wire, simplifying the physical wiring of the temperature probe. |
| **ADC** | AD8232 ECG → ESP32 | **Analog-to-Digital Conversion.** The heart's electrical signals are analog waves; the ADC translates this voltage into a digital number the code can process. |

---

## 4. Software Programs & Frameworks

| Software / Framework | Role in Project | Why it is used |
| :--- | :--- | :--- |
| **PlatformIO (VS Code)** | Build System / IDE | Replaces the basic Arduino IDE. It provides professional dependency management, faster C++ compilation, and a better coding environment. |
| **FreeRTOS** | Operating System | A Real-Time Operating System that allows true multitasking (concurrency). It uses thread-safe Queues to pass data between sensors, displays, and radios safely. |
| **LMIC (LoRa MAC in C)** | LoRaWAN Library | The industry standard library for integrating microcontrollers with The Things Network (TTN). It handles all the complex radio timing and encryption required by LoRaWAN. |
| **GitHub Actions** | CI/CD Pipeline | Automatically compiles the code in the cloud every time a change is pushed, ensuring the software is never broken by a bad update. |
