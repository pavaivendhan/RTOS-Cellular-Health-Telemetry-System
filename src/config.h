#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

// --- LoRaWAN / The Things Network (TTN) Credentials ---
// This EUI must be in little-endian format, so least-significant-byte first.
static const uint8_t PROGMEM APPEUI[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// This should also be in little endian format, see above.
static const uint8_t PROGMEM DEVEUI[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply).
static const uint8_t PROGMEM APPKEY[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// --- Hardware Pin Definitions (ESP32) ---

// LoRa Transceiver (SPI) - e.g. RFM95W
#define LORA_SCK  5
#define LORA_MISO 19
#define LORA_MOSI 27
#define LORA_NSS  18
#define LORA_RST  14
#define LORA_DIO0 26
#define LORA_DIO1 35
#define LORA_DIO2 34

// DS18B20 Temperature Sensor
#define ONE_WIRE_BUS 4 // GPIO4

// AD8232 ECG Sensor
#define ECG_OUTPUT 36    // Analog Input
#define ECG_LO_PLUS 32   // Digital Input
#define ECG_LO_MINUS 33  // Digital Input

// MAX30100 Pulse Sensor & Displays (I2C)
// SDA = GPIO 21 (Default ESP32)
// SCL = GPIO 22 (Default ESP32)
#define MAX30100_INT 23

// OLED Display (I2C)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1 
#define SCREEN_ADDRESS 0x3C 

// --- Thresholds for Alerts ---
#define TEMP_THRESHOLD_HIGH 38.0 
#define HEART_RATE_THRESHOLD_HIGH 100
#define HEART_RATE_THRESHOLD_LOW 60

#endif
