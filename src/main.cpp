#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "MAX30100_PulseOximeter.h"
#include "config.h"
#include <TinyGsmClient.h>
#include <PubSubClient.h>

// --- Global Objects ---
LiquidCrystal_I2C lcd(0x27, 20, 4);
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature tempSensor(&oneWire);
PulseOximeter pox;

HardwareSerial SerialGSM(2); // Use UART2
TinyGsm modem(SerialGSM);
TinyGsmClient client(modem);
PubSubClient mqtt(client);

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
    VitalsData vitals;

    // Initialize Modem
    SerialGSM.begin(115200, SERIAL_8N1, GSM_RX, GSM_TX);
    delay(3000);
    modem.restart();
    
    // Connect to network
    modem.gprsConnect(APN, GPRS_USER, GPRS_PASS);
    
    mqtt.setServer(MQTT_BROKER, MQTT_PORT);

    for (;;) {
        if (!mqtt.connected()) {
            if (mqtt.connect("HealthMonitorESP32")) {
                Serial.println("Connected to MQTT Broker");
            }
        }
        
        if (mqtt.connected()) {
            mqtt.loop();
            // Get latest vitals from queue (without removing them from queue)
            if (xQueuePeek(vitalsQueue, &vitals, 0) == pdPASS) {
                // Publish JSON payload
                char payload[128];
                snprintf(payload, sizeof(payload), "{\"hr\":%.1f,\"spo2\":%.1f,\"temp\":%.1f,\"ecg\":%d}", 
                         vitals.heartRate, vitals.spO2, vitals.temperature, vitals.ecgValue);
                mqtt.publish(MQTT_TOPIC, payload);
            }
        }
        vTaskDelay(5000 / portTICK_PERIOD_MS); // Publish every 5 seconds
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
    xTaskCreatePinnedToCore(TaskTelemetry, "Telemetry", 8192, NULL, 1, &TaskTelemetryHandle, 0);
}

void loop() {
    // Empty loop, FreeRTOS tasks handle everything
    vTaskDelete(NULL); 
}
