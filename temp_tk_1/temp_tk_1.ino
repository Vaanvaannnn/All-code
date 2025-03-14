#include "Max6675.h"

//Max6675 ts(5, 6, 7);//2
Max6675 ts(8, 9, 10);//1
//so, ss, sck
unsigned long previousMillis = 0; 
//const long interval = 300000;      
const long interval = 2000;
//const long interval = 180000;
void setup()
{
  Serial.begin(9600);
  ts.setOffset(0);
}

void loop()
{
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis; 

    float sensor = ts.getCelsius()-2.085; //cal -2.085
    float sensor1 = ts.getCelsius()+21; //cal +21
    Serial.print("temperature : ");
    Serial.println(sensor); 
    Serial.print("temperature cal: ");
    Serial.println(sensor1); 
  }
}
