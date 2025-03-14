#include "Max6675.h"
//so, ss, sck
Max6675 ts1(8, 9, 10); //ตู้2
Max6675 ts2(5, 6, 7); //ตู้3

unsigned long previousMillis = 0; 
//const long interval = 300000;      
const long interval = 2000;
//const long interval = 180000;

void setup()
{
  Serial.begin(9600);
  ts1.setOffset(0);
  ts2.setOffset(0);
}

void loop()
{
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis; 

    // float sensor1 = ts1.getCelsius()-0.325; //cal -1.455
    // float sensor2 = ts2.getCelsius()-3.613; //cal -1.68

    float sensor1 = ts1.getCelsius()-2.315; //cal -1.455
    //float sensor1 = ts1.getCelsius()-4; //cal -1.455
    float sensor2 = ts2.getCelsius()-2.083; //cal -1.68
    //float sensor2 = ts2.getCelsius()+7; //cal -1.68

    Serial.print("Dry 4 : ");
    Serial.print(sensor1); 
   Serial.print("|  Dry 5 : ");
   Serial.println(sensor2); 

  }
}
