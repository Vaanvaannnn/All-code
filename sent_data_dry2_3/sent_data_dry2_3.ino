
#include "WiFiS3.h"
#include "NTPClient.h"
#include "WiFiUdp.h"
#include "ArduinoHttpClient.h"
#include "Max6675.h"
#include "Arduino_LED_Matrix.h"
#include "error.h"
#include "success.h"
#include "TimeLib.h"

// MAC Address: F0:F5:BD:52:5D:94

// const char* ssid = "Staff_1";
// const char* password = "S1A7FBKF";

const char* ssid = "BAC_STAFF";
const char* password = "@Bkf2024";

WiFiClient wifi;
HttpClient httpClient = HttpClient(wifi, "192.168.1.12", 80);

//so, ss, sck
Max6675 ts1(8, 9, 10);
Max6675 ts2(5, 6, 7);

WiFiUDP udp;
NTPClient timeClient(udp, "1.th.pool.ntp.org", 25200, 30000);

ArduinoLEDMatrix matrix1;
ArduinoLEDMatrix matrix2;

unsigned long previousMillis = 0;  // เวลาครั้งล่าสุดที่ส่งข้อมูล
//const unsigned long interval = 2000; // 2 sec
const unsigned long interval = 300000; // 5 นาที

void setup() {
    Serial.begin(9600);
    delay(1000);

    connectToWiFi();

    ts1.setOffset(0);
    ts2.setOffset(0);

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
    // float temp_dry_2 = ts1.getCelsius()-2.085; //usb-c
    // float temp_dry_3 = ts2.getCelsius()-2.085;
    float temp_dry_2 = ts1.getCelsius()+3.5; //jack
    float temp_dry_3 = ts2.getCelsius()+30;

    char dateBuffer[30];
    sprintf(dateBuffer, "%04d-%02d-%02d%%20%02d:%02d:%02d", year(), month(), day(), hour(), minute(), second());
  
     String path;

    // Sending data for dry-2
    path = "/BAC_WebApplications/AdrunoTemperature/TemperaturePost?Temp1=Dry_2&CH1=" + String(temp_dry_2) + "&CH2=0&DateCreate=" + String(dateBuffer);
    sendHttpRequest(path);

    // Sending data for dry-3
    path = "/BAC_WebApplications/AdrunoTemperature/TemperaturePost?Temp1=Dry_3&CH1=" + String(temp_dry_3) + "&CH2=0&DateCreate=" + String(dateBuffer);
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

