# GSM Based Health Monitoring System

This project is a complete Arduino-based Health Monitoring System that remotely tracks a patient's vitals (Heart Rate, SpO2, Temperature, and ECG) and sends SMS alerts via a GSM modem if the vitals cross normal thresholds. It also features a local 20x4 LCD to display real-time data.

## Project Architecture

The system is built around an **Arduino UNO** microcontroller. It interfaces with the following modules:
1.  **MAX30100 Pulse Oximeter & Heart Rate Sensor**: Uses I2C communication to read heart rate (bpm) and blood oxygen levels (SpO2).
2.  **DS18B20 Temperature Sensor**: A 1-wire digital temperature sensor for measuring body temperature.
3.  **AD8232 ECG Sensor**: An analog sensor that measures the electrical activity of the heart.
4.  **20x4 LCD with I2C module**: Displays the vitals locally for the patient or on-site caretaker.
5.  **GSM Modem (e.g., SIM800L or SIM900)**: Communicates with the Arduino via SoftwareSerial to send SMS alerts to a registered emergency contact when thresholds are breached.

## Hardware Connections (Pinout)

### MAX30100 (Pulse Oximeter)
*   **VIN**: 3.3V
*   **GND**: GND
*   **SCL**: A5 (or dedicated SCL)
*   **SDA**: A4 (or dedicated SDA)
*   **INT**: Digital Pin 2

### DS18B20 (Temperature)
*   **VCC**: 5V
*   **GND**: GND
*   **DATA**: Digital Pin 3 (requires a 4.7k pull-up resistor to 5V)

### AD8232 (ECG)
*   **3.3V**: 3.3V
*   **GND**: GND
*   **OUTPUT**: A0
*   **LO-**: Digital Pin 11
*   **LO+**: Digital Pin 10

### I2C LCD 20x4
*   **VCC**: 5V
*   **GND**: GND
*   **SDA**: A4 (shared with MAX30100)
*   **SCL**: A5 (shared with MAX30100)

### GSM Modem
*   **TX**: Digital Pin 7 (Connects to Arduino RX in software)
*   **RX**: Digital Pin 8 (Connects to Arduino TX in software)
*   **GND**: GND
*   **VCC**: External 5V/2A power supply (GSM modules draw peak currents up to 2A during transmission).

## Setup & Installation

1.  **Dependencies**: You will need to install the following libraries in your Arduino IDE (Sketch -> Include Library -> Manage Libraries):
    *   `LiquidCrystal_I2C`
    *   `OneWire`
    *   `DallasTemperature`
    *   `MAX30100lib`
2.  **Configuration**: Open `src/config.h` and change `EMERGENCY_PHONE_NUMBER` to your desired alert number (include the country code, e.g., `+1234567890`). You can also adjust the temperature and heart rate thresholds in this file.
3.  **Upload**: Connect your Arduino UNO, select the correct COM port, and upload `src/HealthMonitor.ino`.

## How It Works

*   The `setup()` function initializes all sensors, the LCD, and the GSM serial connection.
*   The `loop()` function continuously reads the MAX30100, DS18B20, and AD8232 sensors.
*   The data is formatted and printed to the I2C LCD display.
*   The system checks if the vitals are out of the bounds defined in `config.h`. If they are, it triggers the `sendSMSAlert()` function.
*   `sendSMSAlert()` uses AT commands (`AT+CMGF=1` and `AT+CMGS`) to command the GSM modem to dispatch an SMS message with the patient's current vitals to the configured phone number.
