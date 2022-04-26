#include "BluetoothSerial.h"
#include "WipItsLED.h"

char btledsetting[7];   //    Buffer to hold mode and setting from bluetooth
BluetoothSerial ESP_BT;
// Parameters for Bluetooth interface
int incoming;

void setup() {
  Serial.begin(115200);  // initialize serial for debugging
  ESP_BT.begin("Chl@B$_WipIts");    //    Initialize Bluetooth and name your Bluetooth interface -> will show up on your phone
  ws2812fx.init();
  ws2812fx.setBrightness(30);
  ws2812fx.setSpeed(1000);
  ws2812fx.setColor(0x007BFF);
  ws2812fx.setMode(FX_MODE_STATIC);
  ws2812fx.start();

  printModes();
  printUsage();
}

void btCommandIncoming() {
// -------------------- Receive Bluetooth signal ----------------------
  if (ESP_BT.available()) {
    incoming = ESP_BT.read(); //Read what we receive 

    // separate button ID from button value -> button ID is 10, 20, 30, etc, value is 1 or 0
    int ledSetting = floor(incoming / 10);
    int ledSettingValue = incoming % 10;
    
    switch (ledSetting) {
   //   First int sent from app should be 1 to identify were changing mode. 2 if changing speed, 3 if changing brightness
   //   Next numbers should be mode, speed or brightness values
   //   I can adjust this later to get a 6 or 10 digit value to identify all settings at once  
      case 1:
        ws2812fx.setMode(ledSettingValue);
        Serial.print(F("Set mode to: "));
        Serial.print(ws2812fx.getMode());
        Serial.print(" - ");
        Serial.println(ws2812fx.getModeName(ws2812fx.getMode()));
        break;
      case 2:  
        Serial.print("Setting speed to: "); Serial.println(ledSettingValue);
        ws2812fx.setSpeed(ledSettingValue);
        Serial.print(F("Set Speed to: "));
        Serial.print(ws2812fx.getSpeed());
        Serial.print(" - ");
        Serial.println(ws2812fx.getModeName(ws2812fx.getMode()));
        break;
      case 3:  
        Serial.print("Setting brightness to: "); Serial.println(ledSettingValue);
        ws2812fx.setBrightness(ledSettingValue);
        Serial.print(F("Set Brightness to: "));
        Serial.print(ws2812fx.getBrightness());
        Serial.print(" - ");
        Serial.println(ws2812fx.getModeName(ws2812fx.getMode()));
        break;
    }
    
  scmd[0] = '\0';         // reset the commandstring
  scmd_complete = false;  // reset command complete
  }
}

void loop() {
  ws2812fx.service();
  btCommandIncoming();
}
