<div align="center">

# RTOS-Driven Cellular Health Telemetry System 🩺📱

[![PlatformIO](https://img.shields.io/badge/PlatformIO-00979D?style=for-the-badge&logo=PlatformIO&logoColor=white)](https://platformio.org/)
[![C++](https://img.shields.io/badge/Language-C++-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)](https://isocpp.org/)
[![MQTT](https://img.shields.io/badge/Telemetry-MQTT%2FLTE--M-FF6A00?style=for-the-badge)](https://mqtt.org/)
[![FreeRTOS](https://img.shields.io/badge/OS-FreeRTOS-8A2BE2?style=for-the-badge)](#)

An enterprise-grade, embedded IoT health monitoring system designed to track vital signs in real-time and stream telemetry via cellular networks to a cloud broker.

</div>

---

## 📌 Abstract & Overview
Traditional health monitoring is often confined to clinical environments. This project bridges the gap by providing a continuous, remote tracking solution using an **ESP32** 32-bit microcontroller running **FreeRTOS**. The system actively monitors physiological parameters—Heart Rate, Blood Oxygen (SpO2), Body Temperature, and ECG—and displays them locally.

Leveraging a **SIM7000G / Cellular Modem**, the edge device securely packages the vitals into JSON payloads and streams them asynchronously via **MQTT** to a cloud broker (e.g., HiveMQ). This ensures rapid data ingestion and remote monitoring regardless of Wi-Fi availability.

---

## ✨ Key Features & Innovations

- **Concurrent Execution (FreeRTOS):** Abandons the legacy single-threaded loop. Sensors are polled, displays are refreshed, and telemetry is published simultaneously via distinct FreeRTOS tasks and thread-safe queues.
- **Cellular Telemetry (LTE-M/NB-IoT):** Utilizes the `TinyGSM` library to establish a cellular GPRS connection, replacing legacy SMS alerts with modern cloud IoT streaming.
- **Comprehensive Vitals Tracking:** Fuses data from the MAX30100 (Pulse Oximeter), DS18B20 (Temperature), and AD8232 (ECG).
- **Dual Local Displays:** Integrates both a 20x4 I2C LCD and a 128x64 OLED display for versatile, real-time graphical and textual data visualization.
- **CI/CD Pipeline:** Includes a GitHub Actions workflow that automatically verifies the C++ compilation on every push.

---

## 📁 System Architecture

```text
RTOS-Driven Cellular Health Telemetry System/
│
├── src/                      # Source Code
│   ├── main.cpp              # FreeRTOS Tasks (Sensors, Display, Telemetry)
│   └── config.h              # Pinouts, MQTT Credentials, Thresholds
│
├── platformio.ini            # Build system and dependency configuration
├── .github/workflows/        # Automated CI/CD pipeline
├── RTOS-Driven Cellular Health Telemetry System.pdf # Project Documentation
└── README.md                 # This file
```

---

## 🔌 Hardware Connections (ESP32 Pinout)

| Component | Pin / Protocol | ESP32 GPIO Pin |
| :--- | :--- | :--- |
| **MAX30100 (Pulse Oximeter)** | SCL / SDA / INT | GPIO 22 / GPIO 21 / GPIO 19 |
| **DS18B20 (Temperature)** | DATA | GPIO 4 (requires 4.7kΩ pull-up) |
| **AD8232 (ECG)** | OUTPUT / LO- / LO+ | GPIO 34 (ADC) / GPIO 33 / GPIO 32 |
| **I2C Displays (LCD/OLED)** | SDA / SCL | GPIO 21 / GPIO 22 (Shared Bus) |
| **Cellular Modem (e.g., SIM7000)** | TX / RX | GPIO 17 / GPIO 16 (UART2) |

> **Note:** The cellular module requires a dedicated 5V/2A external power supply to handle network transmission current spikes.

---

## 🚀 Setup & Deployment

1. **Install PlatformIO:**  
   Install [PlatformIO](https://platformio.org/) as a VS Code extension.

2. **Configure the System:**  
   Open `src/config.h` to define your APN credentials and configure the `MQTT_BROKER`. By default, it points to the free `broker.hivemq.com` for instant testing.

3. **Compile & Flash:**  
   Connect your ESP32-WROOM-32 via USB. Click the **PlatformIO Build (✓)** button to compile the code, then click **Upload (➔)** to flash the firmware.

---

## ⚙️ How It Works

1. **Initialization:** The `setup()` routine initializes the I2C bus, sensors, displays, and spawns three independent FreeRTOS tasks.
2. **TaskReadSensors:** Every 2 seconds, this task polls the MAX30100, DS18B20, and AD8232. The fused data struct is pushed to a FreeRTOS `Queue`.
3. **TaskUpdateDisplay:** This task blocks on the queue. As soon as new data arrives, it immediately updates the local LCD and OLED screens.
4. **TaskTelemetry:** Running in the background, this task connects to the cellular network via AT commands (handled by `TinyGSM`), establishes a TCP connection to the MQTT broker, and publishes a JSON payload of the latest vitals.
