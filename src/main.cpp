#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "MAX30100_PulseOximeter.h"
#include "config.h"

// LMIC LoRaWAN Library
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

// --- Global Objects ---
LiquidCrystal_I2C lcd(0x27, 20, 4);
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature tempSensor(&oneWire);
PulseOximeter pox;

// --- FreeRTOS Definitions ---
struct VitalsData {
    float heartRate;
    float spO2;
    float temperature;
    int ecgValue;
};

QueueHandle_t vitalsQueue;

// Task Handles
TaskHandle_t TaskSensorsHandle;
TaskHandle_t TaskDisplayHandle;
TaskHandle_t TaskTelemetryHandle;

void onBeatDetected() {
    // Optional beat callback
}

// --- LMIC Callbacks and Config ---
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}
void os_getDevKey (u1_t* buf) { memcpy_P(buf, APPKEY, 16);}

const lmic_pinmap lmic_pins = {
    .nss = LORA_NSS,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = LORA_RST,
    .dio = {LORA_DIO0, LORA_DIO1, LORA_DIO2},
};

static uint8_t telemetryData[6];
static osjob_t sendjob;
const unsigned TX_INTERVAL = 30; // Schedule TX every 30 seconds

void do_send(osjob_t* j) {
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
        VitalsData vitals;
        // Peek latest vitals
        if (xQueuePeek(vitalsQueue, &vitals, 0) == pdPASS) {
            // Compress data to bytes for LoRa transmission
            // e.g. HR: 72 -> 72, SpO2: 98.5 -> 98, Temp: 36.5 -> 365
            telemetryData[0] = (uint8_t)vitals.heartRate;
            telemetryData[1] = (uint8_t)vitals.spO2;
            int16_t tempScaled = (int16_t)(vitals.temperature * 10);
            telemetryData[2] = (tempScaled >> 8) & 0xFF;
            telemetryData[3] = tempScaled & 0xFF;
            telemetryData[4] = (vitals.ecgValue >> 8) & 0xFF;
            telemetryData[5] = vitals.ecgValue & 0xFF;

            LMIC_setTxData2(1, telemetryData, sizeof(telemetryData), 0);
            Serial.println(F("Packet queued"));
        }
    }
}

void onEvent (ev_t ev) {
    switch(ev) {
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
            LMIC_setLinkCheckMode(0);
            break;
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE"));
            // Schedule next transmission
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
        default:
            break;
    }
}

// --- Tasks ---
void TaskReadSensors(void *pvParameters) {
    (void) pvParameters;
    VitalsData currentVitals = {0, 0, 0, 0};
    uint32_t tsLastReport = 0;

    for (;;) {
        pox.update();

        if (millis() - tsLastReport > 2000) {
            currentVitals.heartRate = pox.getHeartRate();
            currentVitals.spO2 = pox.getSpO2();
            
            tempSensor.requestTemperatures();
            currentVitals.temperature = tempSensor.getTempCByIndex(0);

            if ((digitalRead(ECG_LO_PLUS) == 1) || (digitalRead(ECG_LO_MINUS) == 1)) {
                currentVitals.ecgValue = 0; // Leads off
            } else {
                currentVitals.ecgValue = analogRead(ECG_OUTPUT);
            }

            // Send to queue
            xQueueOverwrite(vitalsQueue, &currentVitals);
            tsLastReport = millis();
        }
        vTaskDelay(10 / portTICK_PERIOD_MS); // Yield to other tasks
    }
}

void TaskUpdateDisplay(void *pvParameters) {
    (void) pvParameters;
    VitalsData vitals;

    for (;;) {
        if (xQueueReceive(vitalsQueue, &vitals, portMAX_DELAY) == pdPASS) {
            // Update LCD
            lcd.setCursor(0, 0);
            lcd.printf("HR: %.1f bpm    ", vitals.heartRate);
            lcd.setCursor(0, 1);
            lcd.printf("SpO2: %.1f %%    ", vitals.spO2);
            lcd.setCursor(0, 2);
            lcd.printf("Temp: %.1f C    ", vitals.temperature);
            lcd.setCursor(0, 3);
            if (vitals.ecgValue == 0) lcd.print("ECG: Leads Off    ");
            else lcd.printf("ECG: %d       ", vitals.ecgValue);

            // Update OLED
            oled.clearDisplay();
            oled.setCursor(0, 0);
            oled.printf("HR: %.1f bpm\n", vitals.heartRate);
            oled.printf("SpO2: %.1f %%\n", vitals.spO2);
            oled.printf("Temp: %.1f C\n", vitals.temperature);
            if (vitals.ecgValue == 0) oled.println("ECG: Leads Off");
            else oled.printf("ECG: %d\n", vitals.ecgValue);
            oled.display();
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void TaskTelemetry(void *pvParameters) {
    (void) pvParameters;
    
    // SPI Setup for LoRa
    SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_NSS);

    // Initialize LMIC
    os_init();
    LMIC_reset();

    // Start sending
    do_send(&sendjob);

    for (;;) {
        // Let LMIC handle radio tasks
        os_runloop_once();
        vTaskDelay(10 / portTICK_PERIOD_MS); // Yield
    }
}

void setup() {
    Serial.begin(115200);

    // Initialize Displays
    lcd.init();
    lcd.backlight();
    
    oled.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
    oled.clearDisplay();
    oled.setTextColor(SSD1306_WHITE);

    // Initialize Sensors
    tempSensor.begin();
    pinMode(ECG_LO_PLUS, INPUT);
    pinMode(ECG_LO_MINUS, INPUT);
    pox.begin();
    pox.setOnBeatDetectedCallback(onBeatDetected);

    // Create Queue (size 1, we only need the latest data)
    vitalsQueue = xQueueCreate(1, sizeof(VitalsData));

    // Create FreeRTOS Tasks
    xTaskCreatePinnedToCore(TaskReadSensors, "Sensors", 4096, NULL, 1, &TaskSensorsHandle, 1);
    xTaskCreatePinnedToCore(TaskUpdateDisplay, "Display", 4096, NULL, 1, &TaskDisplayHandle, 1);
    // Increased stack size for LMIC
    xTaskCreatePinnedToCore(TaskTelemetry, "Telemetry", 8192, NULL, 1, &TaskTelemetryHandle, 0);
}

void loop() {
    // Empty loop, FreeRTOS tasks handle everything
    vTaskDelete(NULL); 
}
