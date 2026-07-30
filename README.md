<div align="center">

# GSM Based Health Monitoring System 🩺📱

[![Arduino](https://img.shields.io/badge/Platform-Arduino-00979D?style=for-the-badge&logo=arduino&logoColor=white)](https://www.arduino.cc/)
[![C++](https://img.shields.io/badge/Language-C++-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)](https://isocpp.org/)
[![GSM](https://img.shields.io/badge/Telemetry-GSM%2FSMS-FF6A00?style=for-the-badge)](#)
[![I2C](https://img.shields.io/badge/Protocol-I2C-8A2BE2?style=for-the-badge)](#)

An enterprise-grade, embedded IoT health monitoring system designed to track vital signs in real-time and dispatch emergency SMS alerts via cellular networks.

</div>

---

## 📌 Abstract & Overview
Traditional health monitoring is often confined to clinical environments. This project bridges the gap by providing a continuous, remote tracking solution using an **Arduino UNO** microcontroller. The system actively monitors physiological parameters—Heart Rate, Blood Oxygen (SpO2), Body Temperature, and ECG—and displays them locally.

When critical thresholds are breached, the system leverages a **GSM Modem** to asynchronously transmit SMS alerts to designated emergency contacts, ensuring rapid medical response regardless of Wi-Fi or internet availability.

---

## ✨ Key Features & Innovations

- **Comprehensive Vitals Tracking:** Fuses data from the MAX30100 (Pulse Oximeter), DS18B20 (Temperature), and AD8232 (ECG) for a holistic patient overview.
- **Cellular Emergency Alerts:** Utilizes a SIM800L/SIM900 GSM module via AT commands to send instant SMS warnings, creating a fail-safe alert mechanism independent of local networks.
- **Dual Local Displays:** Integrates both a 20x4 I2C LCD and a 128x64 OLED display for versatile, real-time graphical and textual data visualization.
- **Threshold-Based Triggers:** Continuously evaluates sensor data against configurable medical limits (e.g., tachycardia, hypoxia, hyperthermia) to trigger immediate alerts.

---

## 📁 System Architecture

```text
HealthMonitor/
│
├── src/                      # Arduino C++ Source Code
│   ├── HealthMonitor.ino     # Main execution loop & sensor fusion
│   └── config.h              # Thresholds, PINs, & Emergency Contacts
│
├── GSM BASED HEALTH MONITORING SYSTEM (1).pdf # Project Documentation
├── LEARNING_GUIDE.md         # Extended learning and tutorial
└── README.md                 # This file
```

---

## 🔌 Hardware Connections (Pinout)

| Component | Pin / Protocol | Arduino UNO Pin |
| :--- | :--- | :--- |
| **MAX30100 (Pulse Oximeter)** | SCL / SDA / INT | A5 / A4 / D2 |
| **DS18B20 (Temperature)** | DATA | D3 (requires 4.7kΩ pull-up) |
| **AD8232 (ECG)** | OUTPUT / LO- / LO+ | A0 / D11 / D10 |
| **I2C Displays (LCD/OLED)** | SDA / SCL | A4 / A5 (Shared Bus) |
| **GSM Modem** | TX / RX | D7 / D8 (SoftwareSerial) |

> **Note:** The GSM module requires a dedicated 5V/2A external power supply to handle transmission current spikes.

---

## 🚀 Setup & Deployment

1. **Install Dependencies:**  
   Open the Arduino IDE Library Manager and install:
   - `LiquidCrystal_I2C`
   - `OneWire` & `DallasTemperature`
   - `MAX30100lib`
   - `Adafruit_GFX` & `Adafruit_SSD1306`

2. **Configure the System:**  
   Edit `src/config.h` to define your `EMERGENCY_PHONE_NUMBER` (include country code, e.g., `+1234567890`) and adjust medical thresholds.

3. **Compile & Flash:**  
   Connect your Arduino UNO, select the correct COM port/board in the IDE, and upload the `HealthMonitor.ino` sketch.

---

## ⚙️ How It Works

1. **Initialization:** The `setup()` routine initializes the I2C bus, initializes all connected sensors, configures the LCD/OLED, and establishes SoftwareSerial communication with the GSM modem.
2. **Data Acquisition:** During the `loop()`, the system sequentially polls the MAX30100, DS18B20, and AD8232 to collect real-time vitals.
3. **Local Display:** Formats and renders the parsed data onto the local I2C screens for immediate on-site monitoring.
4. **Threshold Evaluation:** Compares the readings against the defined safety boundaries in `config.h`. 
5. **Emergency Dispatch:** If an anomaly is detected, `sendSMSAlert()` is triggered. It issues AT commands (`AT+CMGF=1` and `AT+CMGS`) to the GSM modem, dispatching an SOS message containing the current vitals.
