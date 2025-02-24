
#include "WiFiS3.h"
#include <NTPClient.h>
#include "WiFiUdp.h"
#include "ArduinoHttpClient.h"
#include "Arduino_LED_Matrix.h"
#include "error.h"
#include "success.h"
#include <TimeLib.h>
#include <Adafruit_MAX31865.h>

// MAC Address: 34:B7:DA:64:31:A8

const char* ssid = "Staff_1";
const char* password = "S1A7FBKF";

// const char* ssid = "BAC_STAFF";
// const char* password = "@Bkf2024";

WiFiClient wifi;
HttpClient httpClient = HttpClient(wifi, "192.168.1.12", 80);

// Use software SPI: CS, DI, DO, CLK
Adafruit_MAX31865 thermo3 = Adafruit_MAX31865(10, 11, 12, 13);
Adafruit_MAX31865 thermo2 = Adafruit_MAX31865(6, 7, 8, 9);
Adafruit_MAX31865 thermo1 = Adafruit_MAX31865(2, 3, 4, 5);

#define RREF      430.0
#define RNOMINAL  100.0

WiFiUDP udp;
NTPClient timeClient(udp, "1.th.pool.ntp.org", 25200, 30000);

ArduinoLEDMatrix matrix1;
ArduinoLEDMatrix matrix2;

unsigned long previousMillis = 0;  // เวลาครั้งล่าสุดที่ส่งข้อมูล
//const unsigned long interval = 2000; // 2 sec
const unsigned long interval = 300000; // 5 นาที
//const unsigned long interval = 10000; // 10 sec

void setup() {
    Serial.begin(9600);
    delay(1000);

    connectToWiFi();

    thermo1.begin(MAX31865_4WIRE);  // set to 2WIRE or 4WIRE as necessary
    thermo2.begin(MAX31865_4WIRE);
    thermo3.begin(MAX31865_4WIRE);

    timeClient.begin();
    
    matrix1.loadSequence(error);
    matrix2.loadSequence(success);

    matrix1.begin();
    matrix2.begin();

    matrix1.play(false);
    matrix2.play(false);
}

void loop() {
    timeClient.update();

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis; // อัปเดตเวลาสำหรับการส่งข้อมูลครั้งล่าสุด

        if (WiFi.status() == WL_CONNECTED) {
            updateCurrentTime();
            sendDataToServer(); // ฟังก์ชันสำหรับส่งข้อมูล
        } else {
            Serial.println("WiFi disconnected! Attempting to reconnect...");
            connectToWiFi();
        }
    }

    // ตรวจสอบการเชื่อมต่อ Wi-Fi อย่างต่อเนื่องในกรณีที่ยังไม่ได้เชื่อมต่อ
    if (WiFi.status() != WL_CONNECTED) {
        connectToWiFi();
    }

    delay(500); // การหน่วงเวลาเล็กน้อยเพื่อให้โปรแกรมทำงานราบรื่น
}

void connectToWiFi() {
    Serial.print("Connecting to WiFi...");
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Connected to WiFi!");
}

void updateCurrentTime() {
    unsigned long epochTime = timeClient.getEpochTime();
    setTime(epochTime);

    Serial.print("Current time: ");
    Serial.print(year());
    Serial.print("-");
    Serial.print(month());
    Serial.print("-");
    Serial.print(day());
    Serial.print(" ");
    Serial.print(hour());
    Serial.print(":");
    Serial.print(minute());
    Serial.print(":");
    Serial.println(second());

        if (year() == 1970 || year() == 2036) {
        Serial.println("Detected time, resending data...");
        sendDataToServer();
    }
}

void sendDataToServer() {
    uint16_t rtd1 = thermo1.readRTD();
    uint16_t rtd2 = thermo2.readRTD();
    uint16_t rtd3 = thermo3.readRTD();

    float ratio1 = rtd1 / 32768.0;
    float ratio2 = rtd2 / 32768.0;
    float ratio3 = rtd3 / 32768.0;

    // Calibration
    float measuredTemp1 = thermo1.temperature(RNOMINAL, RREF) - 1.5;
    float calibratedTemp1 = 0.20 * measuredTemp1 + 5.37;

    float measuredTemp2 = thermo2.temperature(RNOMINAL, RREF);
    float calibratedTemp2 = -0.01 * measuredTemp2 + 18.30;

    float calibratedTemp3 = thermo3.temperature(RNOMINAL, RREF) - 2.55;

    char dateBuffer[30];
    sprintf(dateBuffer, "%04d-%02d-%02d%%20%02d:%02d:%02d", year(), month(), day(), hour(), minute(), second());

    String path;

    // Sending data for MJ-010
    path = "/BAC_WebApplications/AdrunoTemperature/TemperaturePost?Temp1=MJ-010&CH1=" + String(calibratedTemp1) + "&CH2=0&DateCreate=" + String(dateBuffer);
    sendHttpRequest(path);

    // Sending data for MJ-020
    path = "/BAC_WebApplications/AdrunoTemperature/TemperaturePost?Temp1=MJ-020&CH1=" + String(calibratedTemp2) + "&CH2=0&DateCreate=" + String(dateBuffer);
    sendHttpRequest(path);

    // Sending data for MJ-030
    path = "/BAC_WebApplications/AdrunoTemperature/TemperaturePost?Temp1=MJ-030&CH1=" + String(calibratedTemp3) + "&CH2=0&DateCreate=" + String(dateBuffer);
    sendHttpRequest(path);


}

void sendHttpRequest(String path) {
    Serial.println("Sending data...");
    httpClient.beginRequest();
    httpClient.post(path);
    httpClient.sendHeader("Content-Length", "0");
    httpClient.endRequest();

    Serial.print("Sending data to: ");
    Serial.println("http://192.168.1.12" + path);

    int statusCode = httpClient.responseStatusCode();
    String response = httpClient.responseBody();

    Serial.print("Status code: ");
    Serial.println(statusCode);
    Serial.print("Response: ");
    Serial.println(response);

    if (statusCode == 200) {
        matrix1.play(false);
        matrix2.play(true);
        Serial.println("Data sent successfully!");
    } else {
        matrix1.play(true);
        matrix2.play(false);
        Serial.println("Failed to send data. Status code: " + String(statusCode));
    }
}


void checkFault(Adafruit_MAX31865 &thermo, const char *sensorName) {
  uint8_t fault = thermo.readFault();

  if (fault) {
    Serial.print("Fault in "); Serial.print(sensorName); Serial.print(": 0x"); Serial.println(fault, HEX);

    if (fault & MAX31865_FAULT_HIGHTHRESH) {
      Serial.println("RTD High Threshold");
    }

    if (fault & MAX31865_FAULT_LOWTHRESH) {
      Serial.println("RTD Low Threshold");
    }

    if (fault & MAX31865_FAULT_REFINLOW) {
      Serial.println("REFIN- > 0.85 x Bias");
    }

    if (fault & MAX31865_FAULT_REFINHIGH) {
      Serial.println("REFIN- < 0.85 x Bias - FORCE- open");
    }

    if (fault & MAX31865_FAULT_RTDINLOW) {
      Serial.println("RTDIN- < 0.85 x Bias - FORCE- open");
    }

    if (fault & MAX31865_FAULT_OVUV) {
      Serial.println("Under/Over voltage");
    }

    thermo.clearFault();
  }
}