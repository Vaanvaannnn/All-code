
#include "WiFiS3.h"
#include <NTPClient.h>
#include "WiFiUdp.h"
#include "ArduinoHttpClient.h"
#include "Arduino_LED_Matrix.h"
#include "error.h"
#include "success.h"
#include <TimeLib.h>
#include <Adafruit_MAX31865.h>

// MAC Address: 34:B7:DA:66:00:2C

// const char* ssid = "Staff_1";
// const char* password = "S1A7FBKF";


const char* ssid = "BAC_STAFF";
const char* password = "@Bkf2024";

// const char* ssid = "W/V";
// const char* password = "30092001"; 

WiFiClient wifi;
HttpClient httpClient = HttpClient(wifi, "192.168.1.12", 80);

// Use software SPI: CS, DI, DO, CLK
Adafruit_MAX31865 thermo = Adafruit_MAX31865(10, 11, 12, 13); //อย่าลืมเปลี่ยน
#define RREF      430.0
#define RNOMINAL  100.0

WiFiUDP udp;
NTPClient timeClient(udp, "1.pool.ntp.org", 25200, 30000);

ArduinoLEDMatrix matrix1;
ArduinoLEDMatrix matrix2;

unsigned long previousMillis = 0;  // เวลาครั้งล่าสุดที่ส่งข้อมูล
//const unsigned long interval = 2000; // 2 sec
const unsigned long interval = 300000; // 5 นาที

void setup() {
    Serial.begin(9600);
    delay(1000);

    connectToWiFi();

    thermo.begin(MAX31865_4WIRE);

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

    delay(500);
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

    uint16_t rtd = thermo.readRTD();

    float ratio = rtd;

    ratio /= 32768;

    float measuredTemp = thermo.temperature(RNOMINAL, RREF); // ค่าอุณหภูมิที่วัดได้


    checkFault() ;

    char dateBuffer[30];
    sprintf(dateBuffer, "%04d-%02d-%02d%%20%02d:%02d:%02d", year(), month(), day(), hour(), minute(), second());
  
    String path = "/BAC_WebApplications/AdrunoTemperature/TemperaturePost?Temp1=Line-assy&CH1=" + String(measuredTemp) + "&CH2=0&DateCreate=" + String(dateBuffer);

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

void checkFault(void)
{
	uint8_t fault = thermo.readFault();

  if (fault) {

    Serial.print("Fault 0x"); Serial.println(fault, HEX);

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