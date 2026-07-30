#ifndef CONFIG_H
#define CONFIG_H

// --- Network & MQTT Settings ---
#define MQTT_BROKER    "broker.hivemq.com" // Free public broker for testing
#define MQTT_PORT      1883
#define MQTT_TOPIC     "telemetry/health_monitor_2026/vitals"

// Replace with actual SIM APN if needed for Cellular
#define TINY_GSM_MODEM_SIM7000
#define APN "your_apn" 
#define GPRS_USER ""
#define GPRS_PASS ""

// --- Hardware Pin Definitions (ESP32) ---

// DS18B20 Temperature Sensor
#define ONE_WIRE_BUS 4 // GPIO4

// AD8232 ECG Sensor
#define ECG_OUTPUT 34    // Analog Input (ADC1_CH6)
#define ECG_LO_PLUS 32   // Digital Input
#define ECG_LO_MINUS 33  // Digital Input

// MAX30100 Pulse Sensor & Displays (I2C)
// SDA = GPIO 21 (Default ESP32)
// SCL = GPIO 22 (Default ESP32)
#define MAX30100_INT 19

// OLED Display (I2C)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1 
#define SCREEN_ADDRESS 0x3C 

// GSM Module Hardware Serial 2
#define GSM_RX 16
#define GSM_TX 17

// --- Thresholds for Alerts ---
#define TEMP_THRESHOLD_HIGH 38.0 
#define HEART_RATE_THRESHOLD_HIGH 100
#define HEART_RATE_THRESHOLD_LOW 60

#endif
