
#include <Adafruit_MAX31865.h>

// Use software SPI: CS, DI, DO, CLK
Adafruit_MAX31865 thermo = Adafruit_MAX31865(10, 11, 12, 13); //สำหรับตู้  3,4,Assy
//Adafruit_MAX31865 thermo = Adafruit_MAX31865(2, 3, 4, 5); //สำหรับตู้  1
//Adafruit_MAX31865 thermo = Adafruit_MAX31865(6, 7, 8, 9); //สำหรับตู้  2
// use hardware SPI, just pass in the CS pin
//Adafruit_MAX31865 thermo = Adafruit_MAX31865(10);

#define RREF      430.0
#define RNOMINAL  100.0

void setup() {
  Serial.begin(115200);
  Serial.println("Adafruit MAX31865 PT100 Sensor Test!");

  thermo.begin(MAX31865_4WIRE);  // set to 2WIRE or 4WIRE as necessary
}


void loop() {
  uint16_t rtd = thermo.readRTD();

  float ratio = rtd;
  ratio /= 32768;
  // Serial.print("Resistance = "); Serial.println(RREF*ratio,8);
  Serial.print("Temperature = "); Serial.println(thermo.temperature(RNOMINAL, RREF)-2.9);


  // Check and print any faults
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
  Serial.println();
  delay(300000);
  //delay(2000);
  //delay(120000);
}
