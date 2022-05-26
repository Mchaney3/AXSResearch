#include "credentials.h"
#include "Core0.h"
#include <WiFi.h>
#include <WS2812FX.h>
#include <EEPROM.h>

#define LED_PIN     2 // digital pin used to drive the LED strip
#define LED_COUNT 255 // number of LEDs on the strip
#define EEPROM_SIZE 4096
#define PRESETS_MAX_SIZE (EEPROM_SIZE - sizeof(int) - sizeof(preset) - 1) // maximum size of the presets string

WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

TaskHandle_t bt_stream;

void setup() {
  Serial.begin(115200);
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  Serial.print("MAC address of this node is ");
  Serial.println(WiFi.softAPmacAddress());
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  EEPROM.begin(EEPROM_SIZE); // for ESP8266 (comment out if using an Arduino)
  ws2812fx.init();
  ws2812fx.start();
  
  xTaskCreatePinnedToCore(
    btTASK,       /* Task function. */
    "bt_stream",         /* name of task. */
    10000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    2,           /* Always highest priority to assure audio quality */
    &bt_stream,      /* Task handle to keep track of created task */
    0);          /* pin Bluetooth to core o */                  
    
  delay(500);
}

void loop() {

}

#define EEPROM_MAGIC_NUMBER 0x010e0d06
void saveToEEPROM() {
  Serial.println("saving to EEPROM");
  EEPROM.put(sizeof(int) * 0, (int)EEPROM_MAGIC_NUMBER);
  EEPROM.commit(); // for ESP8266 (comment out if using an Arduino)
}

void restoreFromEEPROM() {
  int magicNumber = 0;
  EEPROM.get(sizeof(int) * 0, magicNumber);
  if (magicNumber == EEPROM_MAGIC_NUMBER) {
    Serial.println("restoring from EEPROM");
  }
}
