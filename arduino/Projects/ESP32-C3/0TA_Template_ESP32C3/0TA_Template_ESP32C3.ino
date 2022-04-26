#include <WiFi.h>
#include <WiFiMulti.h>
WiFiMulti wifiMulti; // Used for managing wifi connection
WiFiClient espClient;  // Used for the thingboard object to connect with
int status = WL_IDLE_STATUS;  // the Wifi radio's status. This will also be used later to see if we're cvonnected to wifi or not
const uint32_t connectTimeoutMs = 5000;  // WiFi connect timeout per AP. Increase when connecting takes longer.
char netid[33];       // Can hold an SSID up to 32 characters

#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "credentials.h"

// use first channel of 16 channels (started from zero)
#define LEDC_CHANNEL_0     0

// use 13 bit precission for LEDC timer
#define LEDC_TIMER_13_BIT  13

// use 5000 Hz as a LEDC base frequency
#define LEDC_BASE_FREQ     5000

// fade LED PIN (replace with LED_BUILTIN constant for built-in LED)
#define LED_PIN            8

int brightness = 0;    // how bright the LED is
int fadeAmount = 5;    // how many points to fade the LED by

// Arduino like analogWrite
// value has to be between 0 and valueMax
void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255) {
  // calculate duty, 8191 from 2 ^ 13 - 1
  uint32_t duty = (8191 / valueMax) * min(value, valueMax);

  // write duty to LEDC
  ledcWrite(channel, duty);
}

void setup() {
  delay(100); // power-up safety delay
  ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcAttachPin(LED_PIN, LEDC_CHANNEL_0);
  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.persistent(false);  // Disable storing WiFi settings in flash
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(ssid, ssid_passphrase);
  wifiMulti.addAP(ssid2, ssid2_passphrase);
  //wifiMulti.addAP("ssid_from_AP_3", "your_password_for_AP_3");  
  while (wifiMulti.run(connectTimeoutMs) != WL_CONNECTED) {  // Loop until we're reconnected
    //noWifiBlink();
    for (int rc = 5; rc != 0; rc--){
      Serial.print("Connecting to WiFi in ");
      Serial.println(rc);
      delay(1000);
      }
    } 
  //wifiConnectedBlink();

  ArduinoOTA.setHostname(espHostName);
  ArduinoOTA.setPasswordHash(passwordHash);
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
  
  //pinMode(ledPin, OUTPUT);
  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  ledcAnalogWrite(LEDC_CHANNEL_0, 5);

}

void loop() {
  ArduinoOTA.handle();
  // set the brightness on LEDC channel 0
  

  delay(1000);
}
void ledFadeInOut() {
  // set the brightness on LEDC channel 0
  ledcAnalogWrite(LEDC_CHANNEL_0, brightness);

  // change the brightness for next time through the loop:
  brightness = brightness + fadeAmount;

  // reverse the direction of the fading at the ends of the fade:
  if (brightness <= 0 || brightness >= 255) {
    fadeAmount = -fadeAmount;
  }
  // wait for 30 milliseconds to see the dimming effect
  delay(30);
}
