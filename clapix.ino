#include <ESP8266WiFi.h>
#include "credentials.h"
#include "src/lifx/lifx.h"

int SENSOR = D7;
int clap = 0;
long detection_range_start = 0;
long detection_range = 0;
boolean status_lights = false;

void setup() 
{
  pinMode(SENSOR, INPUT);

  Serial.begin(115200);
  Serial.println();

  WiFi.begin(SSID, PASS);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
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
      LifxProtocol lp;

      if (!status_lights)
      {
        status_lights = true;
        uint16_t payload = POWER_ON;
        lp.BuildLifxHeader(SET_POWER);
        lp.BroadcastMessage(&payload, sizeof(uint16_t));
      }
      else if (status_lights)
      {
        status_lights = false;
        uint16_t payload = POWER_OFF;
        lp.BuildLifxHeader(SET_POWER);
        lp.BroadcastMessage(&payload, sizeof(uint16_t));
      }
    }
    clap = 0;
  }
}
