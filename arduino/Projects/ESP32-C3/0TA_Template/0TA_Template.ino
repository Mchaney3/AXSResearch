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
#include <led_strip.h>

//const int ledPin = 8;

void setup() {
  delay(100); // power-up safety delay
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

}

void loop() {
  //digitalWrite(ledPin, !digitalRead(ledPin));
  ArduinoOTA.handle();
  delay(1000);
}
/*
void noWifiBlink() {
  int i;
  for (i = 0; i < 4; i++) {
    digitalWrite(Wifi_led, LOW);
    delay(100);
    digitalWrite(Wifi_led, HIGH);
    delay(200);
    digitalWrite(Wifi_led, LOW);
    delay(50);
    digitalWrite(Wifi_led, HIGH);
    delay(500);
    digitalWrite(Wifi_led, LOW);
    delay(50);
    digitalWrite(Wifi_led, HIGH);
    delay(600);
  }
}

void fastBlink() {
  int fastBlinkDelay = 25;
  digitalWrite(Data_led, !digitalRead(Data_led));
  delay(fastBlinkDelay);
  digitalWrite(Data_led, !digitalRead(Data_led));
  delay(fastBlinkDelay);
}

void slowBlink() {
  int slowBlinkDelay = 50;
  digitalWrite(Data_led, !digitalRead(Data_led));
  delay(slowBlinkDelay);
  digitalWrite(Data_led, !digitalRead(Data_led));
  delay(slowBlinkDelay);
}

void wifiConnectedBlink() {
  int wifiConnectDelay = 25;
  int i;
  for (i = 0; i < 20; i++) {
    digitalWrite(Wifi_led, !digitalRead(Wifi_led));
    delay(wifiConnectDelay);
    digitalWrite(Wifi_led, !digitalRead(Wifi_led));
    delay(wifiConnectDelay);
  }
}

void otaBlink() {
    digitalWrite(Wifi_led, LOW);
    digitalWrite(Data_led, !digitalRead(Wifi_led));
    delay(100);
    digitalWrite(Data_led, !digitalRead(Data_led));
    digitalWrite(Wifi_led, !digitalRead(Data_led));
    delay(100);
}
*/
