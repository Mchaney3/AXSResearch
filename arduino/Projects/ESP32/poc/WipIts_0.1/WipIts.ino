#include <WiFiMulti.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ESPmDNS.h>
WiFiMulti wifiMulti; // Used for managing wifi connection
int wifiStatus = WiFi.status();  // the Wifi radio's status. This will also be used later to see if we're cvonnected to wifi or not
const uint32_t connectTimeoutMs = 5000;  // WiFi connect timeout per AP. Increase when connecting takes longer.
char netid[33];       // Can hold an SSID up to 32 characters
#include <ArduinoOTA.h>
#include "credentials.h"

// this header is needed for Bluetooth Serial -> works ONLY on ESP32
#include "BluetoothSerial.h"
// init Class:
BluetoothSerial ESP_BT;
// init PINs: assign any pin on ESP32
//int led_pin_1 = 25;
//int led_pin_2 = 26;
//int led_pin_3 = 27;
// Parameters for Bluetooth interface
int incoming;

#include <WS2812FX.h>
#include "WipItsLED.h"

void InitWiFi() {
  Serial.print("Connecting to WiFi");
  while (wifiMulti.run(connectTimeoutMs) != WL_CONNECTED) {  // Loop until we're reconnected
    Serial.print(".");
    delay(200);
  }
  Serial.println("\n");
  Serial.print("WiFi connected to: ");
  Serial.print(WiFi.SSID());
  Serial.print(" with local IP of ");
  Serial.println(WiFi.localIP().toString());
}

void setup() {
  Serial.begin(115200);  // initialize serial for debugging
  ESP_BT.begin("Chl@B$_WipIts");    //    Initialize Bluetooth and name your Bluetooth interface -> will show up on your phone
  WiFi.persistent(false);  // Disable storing WiFi settings in flash
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(ssid, ssid_passphrase);
  wifiMulti.addAP(ssid2, ssid2_passphrase); 
  InitWiFi();
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
  Serial.print("OTA Ready! OTA Hostname = ");
  Serial.print(espHostName);
  Serial.println("\n");
  //pinMode (led_pin_1, OUTPUT);
  //pinMode (led_pin_2, OUTPUT);
  //pinMode (led_pin_3, OUTPUT);
  ws2812fx.init();
  ws2812fx.setBrightness(30);
  ws2812fx.setSpeed(1000);
  ws2812fx.setColor(0x007BFF);
  ws2812fx.setMode(FX_MODE_STATIC);
  ws2812fx.start();

  printModes();
  printUsage();
}

void btCommandIncomingTest() {
// -------------------- Receive Bluetooth signal ----------------------
  if (ESP_BT.available()) {
    incoming = ESP_BT.read(); //Read what we receive

    // separate button ID from button value -> button ID is 10, 20, 30, etc, value is 1 or 0
    int button = floor(incoming / 10);
    int value = incoming % 10;
    
    switch (button) {
      case 1:  
        Serial.print("Button 1:"); Serial.println(value);
        ws2812fx.setMode(30);
    Serial.print(F("Set mode to: "));
    Serial.print(ws2812fx.getMode());
    Serial.print(" - ");
    Serial.println(ws2812fx.getModeName(ws2812fx.getMode()));
        break;
      case 2:  
        Serial.print("Button 2:"); Serial.println(value);
        ws2812fx.setMode(17);
    Serial.print(F("Set mode to: "));
    Serial.print(ws2812fx.getMode());
    Serial.print(" - ");
    Serial.println(ws2812fx.getModeName(ws2812fx.getMode()));
        break;
      case 3:  
        Serial.print("Button 3:"); Serial.println(value);
        ws2812fx.setMode(38);
    Serial.print(F("Set mode to: "));
    Serial.print(ws2812fx.getMode());
    Serial.print(" - ");
    Serial.println(ws2812fx.getModeName(ws2812fx.getMode()));
        break;
    }
    
    if(scmd_complete) {
      process_command();
    }
  }
}

void btCommandIncoming() {
// -------------------- Receive Bluetooth signal ----------------------
  if (ESP_BT.available()) 
  {
    incoming = ESP_BT.read(); //Read what we receive 

    // separate button ID from button value -> button ID is 10, 20, 30, etc, value is 1 or 0
    int button = floor(incoming / 10);
    int value = incoming % 10;
    
    switch (button) {
      case 1:  
        Serial.print("Button 1:"); Serial.println(value);
        //digitalWrite(led_pin_1, value);
        break;
      case 2:  
        Serial.print("Button 2:"); Serial.println(value);
        //digitalWrite(led_pin_2, value);
        break;
      case 3:  
        Serial.print("Button 3:"); Serial.println(value);
        //digitalWrite(led_pin_3, value);
        break;
    }
  }
}

void loop() {
  ws2812fx.service();
  ArduinoOTA.handle();
//  btCommandIncoming();
  btCommandIncomingTest();
  
}
