/*
  Streaming data from Bluetooth to internal DAC of ESP32
  
  Copyright (C) 2020 Phil Schatzmann
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// ==> Example to use external 32 bit DAC - the 16 bit A2DP ouptut will be expanded to the indicated bits

#include <WiFi.h>
#include <WiFiMulti.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "BluetoothA2DPSink.h"
#include "credentials.h"

WiFiMulti wifiMulti;
BluetoothA2DPSink a2dp_sink;

TaskHandle_t btStream;
TaskHandle_t wifi_Neo;

void setup() {
  xTaskCreatePinnedToCore(
                    btTASK,   /* Task function. */
                    "btStream",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &btStream,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
  delay(500); 

  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                    wifiTASK,   /* Task function. */
                    "wifi_Neo",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &wifi_Neo,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */
    delay(500);
}


  void btTASK( void * pvParameters ){
//  Serial.print("Task1 running on core ");
//  Serial.println(xPortGetCoreID());

  a2dp_sink.set_bits_per_sample(32);
  i2s_pin_config_t my_pin_config = {
    .bck_io_num = 27,   //    BCK
    .ws_io_num = 26,    //    LRCK
    .data_out_num = 25,   //    DIN
    .data_in_num = I2S_PIN_NO_CHANGE
  };
  a2dp_sink.set_pin_config(my_pin_config);
  a2dp_sink.start(btDeviceName); 
  for(;;){
    delay(1000);
  }
}

  void wifiTASK( void * pvParameters ){
//  Serial.print("Task1 running on core ");
//  Serial.println(xPortGetCoreID());

  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(ssid1, ssidPassphrase1);
  wifiMulti.addAP(ssid2, ssidPassphrase2);
  if(wifiMulti.run() == WL_CONNECTED) {
    ESP.restart();
  }
  ArduinoOTA.setHostname(espHostName);
  ArduinoOTA.setPasswordHash(otaPasswordHash);
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
//      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
//      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
//      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
//      Serial.printf("Error[%u]: ", error);
//      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
//      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
//      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
//      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
//      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();

//  Serial.println("Ready");
//  Serial.print("IP address: ");
//  Serial.println(WiFi.localIP());
  
  for(;;){
  if(wifiMulti.run() != WL_CONNECTED) {
      delay(1000);
    }
    ArduinoOTA.handle();
    delay(1000);
  }
}

void loop() {
  
}
