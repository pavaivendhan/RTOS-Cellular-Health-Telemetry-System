<div align="center">

# RTOS-Driven LoRaWAN Health Telemetry System 🩺📡

[![PlatformIO](https://img.shields.io/badge/PlatformIO-00979D?style=for-the-badge&logo=PlatformIO&logoColor=white)](https://platformio.org/)
[![C++](https://img.shields.io/badge/Language-C++-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)](https://isocpp.org/)
[![LoRaWAN](https://img.shields.io/badge/Telemetry-LoRaWAN-FF6A00?style=for-the-badge)](https://lora-alliance.org/)
[![FreeRTOS](https://img.shields.io/badge/OS-FreeRTOS-8A2BE2?style=for-the-badge)](#)

An enterprise-grade, embedded IoT health monitoring system designed to track vital signs in real-time and stream telemetry securely over long distances using **LoRaWAN**.

📖 **[Read the Full Project Summary & Architecture Here](PROJECT_SUMMARY.md)**

</div>

---

## 📌 Abstract & Overview
Traditional health monitoring is often confined to clinical environments. This project bridges the gap by providing a continuous, remote tracking solution using an **ESP32** 32-bit microcontroller running **FreeRTOS**. The system actively monitors physiological parameters—Heart Rate, Blood Oxygen (SpO2), Body Temperature, and ECG—and displays them locally.

Leveraging an **SPI LoRa Transceiver** (e.g., RFM95W/SX1276), the edge device compresses the vitals into a highly efficient hex payload and transmits them asynchronously over free radio bands to a local gateway via **The Things Network (TTN)**. This eliminates cellular data costs and massively extends battery life.

---

## ✨ Key Features & Innovations

- **Concurrent Execution (FreeRTOS):** Abandons the legacy single-threaded loop. Sensors are polled, displays are refreshed, and telemetry is published simultaneously via distinct FreeRTOS tasks and thread-safe queues.
- **LoRaWAN Telemetry:** Utilizes the `LMIC` (LoRa MAC in C) library to establish a secure, low-power, long-range radio connection to TTN gateways.
- **Comprehensive Vitals Tracking:** Fuses data from the MAX30100 (Pulse Oximeter), DS18B20 (Temperature), and AD8232 (ECG).
- **Dual Local Displays:** Integrates both a 20x4 I2C LCD and a 128x64 OLED display for versatile, real-time graphical and textual data visualization.
- **CI/CD Pipeline:** Includes a GitHub Actions workflow that automatically verifies the C++ compilation on every push.

---

## 📁 System Architecture

```text
RTOS-Driven LoRaWAN Health Telemetry System/
│
├── src/                      # Source Code
│   ├── main.cpp              # FreeRTOS Tasks (Sensors, Display, Telemetry)
│   └── config.h              # Pinouts, SPI Settings, TTN Credentials
│
├── platformio.ini            # Build system and LMIC dependency configuration
├── PROJECT_SUMMARY.md        # Technical breakdown of problems, sensors, & protocols
├── LEARNING_GUIDE.md         # Educational walkthrough of the codebase
└── README.md                 # This file
```

---

## 🔌 Hardware Connections (ESP32 Pinout)

| Component | Pin / Protocol | ESP32 GPIO Pin |
| :--- | :--- | :--- |
| **MAX30100 (Pulse Oximeter)** | SCL / SDA / INT | GPIO 22 / GPIO 21 / GPIO 23 |
| **DS18B20 (Temperature)** | DATA | GPIO 4 (requires 4.7kΩ pull-up) |
| **AD8232 (ECG)** | OUTPUT / LO- / LO+ | GPIO 36 (ADC) / GPIO 33 / GPIO 32 |
| **I2C Displays (LCD/OLED)** | SDA / SCL | GPIO 21 / GPIO 22 (Shared Bus) |
| **LoRa Radio (RFM95W)** | SCK / MISO / MOSI / NSS / DIO0 | GPIO 5 / 19 / 27 / 18 / 26 |

---

## 🚀 Setup & Deployment

1. **Install PlatformIO:**  
   Install [PlatformIO](https://platformio.org/) as a VS Code extension.

2. **Configure the System:**  
   Open `src/config.h` and paste your `APPEUI`, `DEVEUI`, and `APPKEY` from your TTN Application Console.

3. **Compile & Flash:**  
   Connect your ESP32-WROOM-32 via USB. Click the **PlatformIO Build (✓)** button to compile the code, then click **Upload (➔)** to flash the firmware.

---

## ⚙️ How It Works

1. **Initialization:** The `setup()` routine initializes the I2C bus, SPI bus, sensors, displays, and spawns three independent FreeRTOS tasks.
2. **TaskReadSensors:** Every 2 seconds, this task polls the MAX30100, DS18B20, and AD8232. The fused data struct is pushed to a FreeRTOS `Queue`.
3. **TaskUpdateDisplay:** This task blocks on the queue. As soon as new data arrives, it immediately updates the local LCD and OLED screens.
4. **TaskTelemetry:** Running in the background, this task connects to the LoRaWAN network via LMIC. It compresses the latest struct from the queue into a 6-byte hexadecimal payload and broadcasts it over the radio to The Things Network.
