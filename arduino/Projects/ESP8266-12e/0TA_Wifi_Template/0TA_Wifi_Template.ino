#include "credentials.h"

#include <ArduinoOTA.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti; // Used for managing wifi connection
WiFiClient espClient;  // Used for the thingboard object to connect with
int status = WL_IDLE_STATUS;  // the Wifi radio's status. This will also be used later to see if we're cvonnected to wifi or not
const uint32_t connectTimeoutMs = 5000;  // WiFi connect timeout per AP. Increase when connecting takes longer.
char netid[33];       // Can hold an SSID up to 32 characters
int Wifi_led = 2;  // ESP12E builtin LED on Pin 2 for Wifi status
int Data_led = 16; // ESP12E builtin LED on Pin 16 for Data IO notification

void setup() {
  ArduinoOTA.setHostname(espHostName);
  ArduinoOTA.setPasswordHash(OTA_password_hash);
  ArduinoOTA.onStart([]() { // do a fancy thing with our board led at end
    int i;
    for (i = 0; i < 5; i++) {
      otaBlink();
    }
  });
  ArduinoOTA.onEnd([]() { // do a fancy thing with our board led at end
    int i;
    for (i = 0; i < 10; i++) {
      otaBlink();
    }
  });
  ArduinoOTA.onError([](ota_error_t error) {
    (void)error;
    ESP.restart();
  });
  ArduinoOTA.begin();
  Serial.println("OTA Ready");
  Serial.print("OTA Hostname = ");
  Serial.println(espHostName);
  pinMode(Wifi_led, OUTPUT);
  pinMode(Data_led, OUTPUT);
  Serial.begin(115200);  // initialize serial for debugging
  WiFi.persistent(false);  // Disable storing WiFi settings in flash
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(ssid, ssid_passphrase);
  wifiMulti.addAP(ssid2, ssid2_passphrase);
  //wifiMulti.addAP("ssid_from_AP_3", "your_password_for_AP_3");  
  InitWiFi();
  digitalWrite(Data_led, !digitalRead(Data_led));
}

void loop() {
  ArduinoOTA.handle();
  otaBlink();
}

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

void InitWiFi() {
  while (wifiMulti.run(connectTimeoutMs) != WL_CONNECTED) {  // Loop until we're reconnected
    noWifiBlink();
  } 
  wifiConnectedBlink();
}
