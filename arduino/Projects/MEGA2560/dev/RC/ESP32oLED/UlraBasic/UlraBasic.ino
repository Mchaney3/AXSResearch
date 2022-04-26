#include "Arduino.h"
#include "heltec.h"
#include "images.h"

void setup() {
  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Disable*/, true /*Serial Enable*/);
  Heltec.display->flipScreenVertically();
  Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
  Heltec.display->setFont(ArialMT_Plain_16);
  Heltec.display->drawString(128, 0, "Hello World Chlabs");
  Heltec.display->display();
  Serial.begin(115200);
  delay(3000);
}

void loop() {
  Heltec.display->clear();
  Heltec.display->drawXbm(34, 14, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
  Heltec.display->display();
  delay(1000);
  Serial.print(".");
}
