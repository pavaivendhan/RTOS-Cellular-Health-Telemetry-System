#ifndef CONFIG_H
#define CONFIG_H

// --- Pin Definitions ---
// DS18B20 Temperature Sensor
#define ONE_WIRE_BUS 3 

// AD8232 ECG Sensor
#define ECG_OUTPUT A0
#define ECG_LO_PLUS 10
#define ECG_LO_MINUS 11

// MAX30100 Pulse Sensor (I2C)
// SDA = A4 (or SDA pin)
// SCL = A5 (or SCL pin)
// INT = 2

// GSM Module Software Serial
#define GSM_RX 7
#define GSM_TX 8

// --- Thresholds for Alerts ---
#define TEMP_THRESHOLD_HIGH 38.0 // Celsius (approx 100.4 F)
#define HEART_RATE_THRESHOLD_HIGH 100
#define HEART_RATE_THRESHOLD_LOW 60

// --- Alert Phone Number ---
#define EMERGENCY_PHONE_NUMBER "+1234567890" // Replace with actual number

#endif
