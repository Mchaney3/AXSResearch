//    TODO: Figure out the symaphore to shut down BT on Core 0, from Core 1
//    Stabilize code with ILI9341 connected. Crashing with BT connected

#include <WiFi.h>
#include <WiFiMulti.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <WS2812FX.h>
#include "soc/soc.h" //disable brownout problems
#include "soc/rtc_cntl_reg.h"  //disable brownout problems
#include <Arduino.h>
#include <IRremote.hpp>
#include "SdFat.h"
#include <Wire.h>
#include "BluetoothA2DPSink.h"

BluetoothA2DPSink a2dp_sink;
SdExFat sd;
ExFile file;

/**********   My Custom Libraries   ***********/
#include "credentials.h"
#include "BT.h"
#include "MySdFatFunctions.h"
#include "LED.h"
#include "IR.h"
#include "OTA.h"
#include "Core1.h"
#include "Core0.h"
/********************************************/

TaskHandle_t bt_stream;
TaskHandle_t restofthestory;
SemaphoreHandle_t xSemaphore = NULL;

void setup() {
  Wire.begin();
  Serial.begin(115200);
//  Serial2.begin(9600);
  delay(1000);
  xSemaphore = xSemaphoreCreateMutex();
  
  xTaskCreatePinnedToCore(
    btTASK,   /* Task function. */
    "bt_stream",     /* name of task. */
    10000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    1,           /* Always highest priority to assure audio quality */
    &bt_stream,      /* Task handle to keep track of created task */
    0);          /* pin Bluetooth and handle OTA updates (Handled here to kill BT when OTA received) to core 0 */                  
delay(500);

  xTaskCreatePinnedToCore(
    CoreOneTasks,   /* Task function. */
    "restofthestory",     /* name of task. */
    10000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    0,           /* Always lowest priority to assure audio quality */
    &restofthestory,      /* Task handle to keep track of created task */
    1);          /* pin WiFi, LED, OTA functions, IR, SD, Camera to core 1 1 */                  
  delay(500);
}

void loop() {
}
