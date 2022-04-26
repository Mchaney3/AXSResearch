#include <WiFiMulti.h>
#include <WiFiUdp.h>
#include <ESPmDNS.h>
WiFiMulti wifiMulti; // Used for managing wifi connection
int status = WL_IDLE_STATUS;  // the Wifi radio's status. This will also be used later to see if we're cvonnected to wifi or not
const uint32_t connectTimeoutMs = 5000;  // WiFi connect timeout per AP. Increase when connecting takes longer.
char netid[33];       // Can hold an SSID up to 32 characters

#include <ArduinoOTA.h>
#include "credentials.h"

void setup() {
  ArduinoOTA.setHostname(espHostName);
  ArduinoOTA.setPasswordHash(OTA_password_hash);
  ArduinoOTA.onStart([]() { // do a fancy thing with our board led at end
  });
  ArduinoOTA.onEnd([]() { // do a fancy thing with our board led at end
    ESP.restart();
  });
  ArduinoOTA.onError([](ota_error_t error) {
    (void)error;
    ESP.restart();
  });
  ArduinoOTA.begin();
  Serial.println("OTA Ready");
  Serial.print("OTA Hostname = ");
  Serial.println(espHostName);
  Serial.begin(115200);  // initialize serial for debugging
  WiFi.persistent(false);  // Disable storing WiFi settings in flash
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(ssid, ssid_passphrase);
  wifiMulti.addAP(ssid2, ssid2_passphrase);
  //wifiMulti.addAP("ssid_from_AP_3", "your_password_for_AP_3");  
  InitWiFi();
}

void loop() {
  ArduinoOTA.handle();
}

void InitWiFi() {
  while (wifiMulti.run(connectTimeoutMs) != WL_CONNECTED) {  // Loop until we're reconnected
    Serial.println("WiFi Connecting...");
    delay(200);
  } 
  Serial.println("WiFi Connected!");
}
