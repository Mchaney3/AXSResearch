/*
 *                         THIS PROJECT IS COMPLETE WITH THE FOLLOWING COMPONENTS
 * 1. WS2812FX
 * 2. Bluetooth to control LED's
 * 3. Bluetooth Audio
 * 4. ESP-NOW to send wireless info to other board to be displayed on TFT
 * 
 */
#include <NimBLEDevice.h>
#include <WS2812FX.h>
#define LED_COUNT 300
#define LED_PIN 2
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
#include "SerialCommands.h"

void setup() {
  Serial.begin(115200);
  
  //    Init bluetooth here instead of Serial for control
  //    Add Bluetooth audio as well
  Serial.println("Starting NimBLE Server");
  NimBLEDevice::init("Truck_BT_V1.1");
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);
  NimBLEDevice::setSecurityAuth(true, true, true);
  NimBLEDevice::setSecurityPasskey(123456);
  NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);
  NimBLEServer *pServer = NimBLEDevice::createServer();
  NimBLEService *pService = pServer->createService("TruckBtSerial");
  NimBLECharacteristic *pNonSecureCharacteristic = pService->createCharacteristic("1234", NIMBLE_PROPERTY::READ );
  NimBLECharacteristic *pSecureCharacteristic = pService->createCharacteristic("1235", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::READ_ENC | NIMBLE_PROPERTY::READ_AUTHEN);
  pService->start();
  pNonSecureCharacteristic->setValue("Hello Non Secure BLE");
  pSecureCharacteristic->setValue("Hello Secure BLE");
  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID("TruckBtSerial");
  pAdvertising->start();
  
  ws2812fx.init();
  ws2812fx.setBrightness(30);
  ws2812fx.setSpeed(1000);
  ws2812fx.setColor(0x007BFF);
  ws2812fx.setMode(FX_MODE_STATIC);
  ws2812fx.start();

  printModes();
  printUsage();

}

void loop() {
  ws2812fx.service();

  recvChar(); // read serial comm

  if(scmd_complete) {
    process_command();
  }
}
