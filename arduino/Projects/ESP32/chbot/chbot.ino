//    32-BIT DAC A2DP_Sink with WS2182FX and bluetooth streaming/control

//    TODO: Figure out the symaphore to shut down BT on Core 0, from Core 1
//  Stabilize code with ILI9341 connected. Crashing with BT connected

// Pins 32 through 39 are only unused pins. 33-39 are input only

#include "BluetoothA2DPSink.h"
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

/**********   My Custom Libraries   ***********/

#include "MySdFatFunctions.h"
#include "LED.h"
#include "IR.h"
#include "Core0.h"
#include "Core1.h"
#include "BT.h"
#include "OTA.h"
/********************************************/

TaskHandle_t bt_stream;
TaskHandle_t restofthestory;
SemaphoreHandle_t xSemaphore = NULL;

void setup() {
  
  
  xSemaphore = xSemaphoreCreateMutex();
  
  xTaskCreatePinnedToCore(
                    btTASK,   /* Task function. */
                    "bt_stream",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    2,           /* Always highest priority to assure audio quality */
                    &bt_stream,      /* Task handle to keep track of created task */
                    0);          /* pin Bluetooth to core o */                  
  delay(500);

  xTaskCreatePinnedToCore(
                    CoreOneTasks,   /* Task function. */
                    "restofthestory",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    2,           /* Always highest priority to assure audio quality */
                    &restofthestory,      /* Task handle to keep track of created task */
                    1);          /* pin Bluetooth to core o */                  
  delay(500);

}

void loop() {
}
