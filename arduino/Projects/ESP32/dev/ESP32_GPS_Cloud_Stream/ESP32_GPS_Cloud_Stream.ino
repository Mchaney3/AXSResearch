#include <BlynkSimpleEsp32_BT_WF.h>

/*************************************************************
  Download latest Blynk library here:
    https://github.com/blynkkk/blynk-library/releases/latest

  Blynk is a platform with iOS and Android apps to control
  Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build graphic interfaces for all your
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: http://www.blynk.cc
    Sketch generator:           http://examples.blynk.cc
    Blynk community:            http://community.blynk.cc
    Follow us:                  http://www.fb.com/blynkapp
                                http://twitter.com/blynk_app

  Blynk library is licensed under MIT license
  This example code is in public domain.

 *************************************************************

  App project setup:
    GPS Stream widget on V1.
 *************************************************************/

#include <BlynkSimpleEsp32_BT_WF.h>
#include <WiFi.h>
#include <heltec.h>
#include "images.h"
#include "Adafruit_FONA.h"

#define BLYNK_PRINT SoftwareSerial
#define BLYNK_TEMPLATE_ID "TMPLmYWEJ4As"
#define BLYNK_DEVICE_NAME "ESP32"

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "GMW2HaPUl7iMW-uPZo1cl4QYQwx5DuuR";

#define ArduinoSS_TX 20
#define ArduinoSS_RX 21

#include <SoftwareSerial.h>

Software







BLYNK_WRITE(V1) {
  GpsParam gps(param);

  // Print 6 decimal places for Lat, Lon
  Serial.print("Lat: ");
  Serial.println(gps.getLat(), 7);

  Serial.print("Lon: ");
  Serial.println(gps.getLon(), 7);

  // Print 2 decimal places for Alt, Speed
  Serial.print("Altitute: ");
  Serial.println(gps.getAltitude(), 2);

  Serial.print("Speed: ");
  Serial.println(gps.getSpeed(), 2);

  Serial.println();
}

void setup()
{
  // Debug console
  Serial.begin(9600);

  Blynk.begin(auth);
}

void loop()
{
  Blynk.run();
}
