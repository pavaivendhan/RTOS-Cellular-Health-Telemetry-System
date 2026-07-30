#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "MAX30100_PulseOximeter.h"
#include "config.h"

// --- Components Initialization ---

// LCD: Address 0x27, 20 columns, 4 rows
LiquidCrystal_I2C lcd(0x27, 20, 4);

// OLED: Address 0x3C
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// GSM Module
SoftwareSerial gsm(GSM_RX, GSM_TX);

// Temperature Sensor
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature tempSensor(&oneWire);

// Pulse Oximeter
PulseOximeter pox;
uint32_t tsLastReport = 0;

// Variables
float bodyTemperature = 0.0;
float heartRate = 0.0;
float spO2 = 0.0;
int ecgValue = 0;
bool alertSent = false;

// Callback for Pulse Oximeter beat detection
void onBeatDetected()
{
    // Heartbeat detected
}

void setup() {
    Serial.begin(115200);
    
    // Initialize LCD
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Health Monitor");
    lcd.setCursor(0, 1);
    lcd.print("Initializing...");

    // Initialize OLED
    if(!oled.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
    } else {
        oled.clearDisplay();
        oled.setTextSize(1);
        oled.setTextColor(SSD1306_WHITE);
        oled.setCursor(0, 10);
        oled.println("Health Monitor");
        oled.println("Initializing...");
        oled.display();
    }

    // Initialize GSM
    gsm.begin(9600);
    delay(1000);
    
    // Initialize Temperature Sensor
    tempSensor.begin();

    // Initialize ECG Pins
    pinMode(ECG_LO_PLUS, INPUT);
    pinMode(ECG_LO_MINUS, INPUT);

    // Initialize Pulse Oximeter
    if (!pox.begin()) {
        Serial.println("Pulse sensor init failed!");
        lcd.setCursor(0, 2);
        lcd.print("POX INIT FAILED");
        oled.println("POX INIT FAILED");
        oled.display();
    } else {
        pox.setOnBeatDetectedCallback(onBeatDetected);
    }

    delay(2000);
    lcd.clear();
    oled.clearDisplay();
}

void loop() {
    // 1. Read Pulse & SpO2
    pox.update();
    if (millis() - tsLastReport > 2000) {
        heartRate = pox.getHeartRate();
        spO2 = pox.getSpO2();
        
        // 2. Read Temperature
        tempSensor.requestTemperatures();
        bodyTemperature = tempSensor.getTempCByIndex(0);

        // 3. Read ECG
        if ((digitalRead(ECG_LO_PLUS) == 1) || (digitalRead(ECG_LO_MINUS) == 1)) {
            ecgValue = 0; // Leads off
        } else {
            ecgValue = analogRead(ECG_OUTPUT);
        }

        // 4. Update Displays
        updateDisplays();

        // 5. Check Thresholds and Send SMS
        checkThresholds();

        tsLastReport = millis();
    }
}

void updateDisplays() {
    // Update LCD
    lcd.setCursor(0, 0);
    lcd.print("HR: "); lcd.print(heartRate, 1); lcd.print(" bpm    ");

    lcd.setCursor(0, 1);
    lcd.print("SpO2: "); lcd.print(spO2, 1); lcd.print(" %    ");

    lcd.setCursor(0, 2);
    lcd.print("Temp: "); lcd.print(bodyTemperature, 1); lcd.print(" C    ");

    lcd.setCursor(0, 3);
    lcd.print("ECG: ");
    if (ecgValue == 0) {
        lcd.print("Leads Off ");
    } else {
        lcd.print(ecgValue); lcd.print("       ");
    }

    // Update OLED
    oled.clearDisplay();
    oled.setCursor(0, 0);
    oled.print("HR: "); oled.print(heartRate, 1); oled.println(" bpm");
    oled.print("SpO2: "); oled.print(spO2, 1); oled.println(" %");
    oled.print("Temp: "); oled.print(bodyTemperature, 1); oled.println(" C");
    oled.print("ECG: ");
    if (ecgValue == 0) {
        oled.println("Leads Off");
    } else {
        oled.println(ecgValue);
    }
    oled.display();
}

void checkThresholds() {
    // Only send an alert if one hasn't been sent recently
    if (bodyTemperature > TEMP_THRESHOLD_HIGH || heartRate > HEART_RATE_THRESHOLD_HIGH || (heartRate > 0 && heartRate < HEART_RATE_THRESHOLD_LOW)) {
        if (!alertSent) {
            sendSMSAlert();
            alertSent = true; 
        }
    } else {
        alertSent = false; 
    }
}

void sendSMSAlert() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sending SMS Alert!");
    
    oled.clearDisplay();
    oled.setCursor(0, 10);
    oled.println("Sending SMS Alert!");
    oled.display();
    
    gsm.println("AT+CMGF=1"); // Text mode
    delay(1000);
    gsm.print("AT+CMGS=\"");
    gsm.print(EMERGENCY_PHONE_NUMBER);
    gsm.println("\"");
    delay(1000);
    
    gsm.print("ALERT! Patient Vitals Abnormal.\n");
    gsm.print("Temp: ");
    gsm.print(bodyTemperature);
    gsm.print(" C\n");
    gsm.print("HR: ");
    gsm.print(heartRate);
    gsm.print(" bpm");
    delay(100);
    gsm.write(26); // ASCII code of CTRL+Z
    delay(3000);
    
    lcd.clear();
    oled.clearDisplay();
}
