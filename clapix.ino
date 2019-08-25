#include <Arduino.h>

int SENSOR = 7;
int LED = 11;
int clap = 0;
long detection_range_start = 0;
long detection_range = 0;
boolean status_lights = false;

void setup() 
{
  pinMode(SENSOR, INPUT);
  pinMode(LED, OUTPUT);
  Serial.begin(9600);
}

void loop() 
{
  int status_sensor = digitalRead(SENSOR);
  if (status_sensor == HIGH)
  {
    Serial.println(status_sensor);
    if (clap == 0)
    {
      detection_range_start = detection_range = millis();
      clap++;
    }
    else if (clap > 0 && millis() - detection_range >= 50)
    {
      detection_range = millis();
      clap++;
    }
  }
    
  if (millis() - detection_range_start >= 400)
  {
    if (clap == 2)
    {
      if (!status_lights)
      {
        status_lights = true;
        digitalWrite(LED, HIGH);
      }
      else if (status_lights)
      {
        status_lights = false;
        digitalWrite(LED, LOW);
      }
    }
    clap = 0;
  }
}
