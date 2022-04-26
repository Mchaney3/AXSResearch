#include <WiFi.h>
#include <WiFiMulti.h>
#include <WS2812FX.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Arduino.h>
//#include "IRpindefinitions.h"
#include <IRremote.hpp>

#define LED_COUNT 300
#define LED_PIN 2
#define DECODE_NEC          // Includes Apple and Onkyo
//#define DEBUG // to see if attachInterrupt is used
//#define TRACE // to see the state of the ISR state machine
#define IR_RECEIVE_PIN          39  // analog pin
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

int OKAY_SHIFT = 0;

WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
WiFiMulti wifiMulti;

/*************************            Bluetooth on core 0         ************************************/

void avrc_metadata_callback(uint8_t data1, const uint8_t *data2) {
  Serial.printf("AVRC metadata rsp: attribute id 0x%x, %s\n", data1, data2);
}

void read_data_stream(const uint8_t *data, uint32_t length)
{
  // Do something with the data packet
}

void btTASK( void * pvParameters ){
  int ota_beginning = 0;
  static i2s_config_t i2s_config = {
    .mode = (i2s_mode_t) (I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 44100, // updated automatically by A2DP
    .bits_per_sample = (i2s_bits_per_sample_t)32,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = (i2s_comm_format_t) (I2S_COMM_FORMAT_STAND_I2S),
    .intr_alloc_flags = 0, // default interrupt priority
    .dma_buf_count = 8,
    .dma_buf_len = 64,
    .use_apll = true,
    .tx_desc_auto_clear = true // avoiding noise in case of data unavailability
  };
  i2s_pin_config_t my_pin_config = {
    .bck_io_num = 4,   //    BCK
    .ws_io_num = 5,    //    LRCK
    .data_out_num = 17,   //    DIN
    .data_in_num = I2S_PIN_NO_CHANGE
  };
  a2dp_sink.set_pin_config(my_pin_config);
  a2dp_sink.set_stream_reader(read_data_stream);
  a2dp_sink.set_avrc_metadata_callback(avrc_metadata_callback);
  a2dp_sink.set_volume(95);
  a2dp_sink.start(btDeviceName);
  ArduinoOTA.setHostname(espHostName);
  ArduinoOTA.setPasswordHash(otaPasswordHash);
  ArduinoOTA
    .onStart([]() {
      noInterrupts();
      a2dp_sink.set_stream_reader(read_data_stream, false);
      a2dp_sink.stop();
      //  vTaskDelete(bt_stream); Upload this as is, then uncomment this and try
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
      ESP.restart();
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

  ArduinoOTA.begin();
//  a2dp_sink.pause(btDeviceName);
  for(;;){
    
    /*
    if( xSemaphore != NULL ){
      if( xSemaphoreTake( xSemaphore, portMAX_DELAY)){
        a2dp_sink.set_stream_reader(read_data_stream, false);
        a2dp_sink.stop();
        Serial.printf("Setting ota_beginning to "); Serial.println(ota_beginning++);
        break;      
      }
    }
    */
    ArduinoOTA.handle();
    delay(100);
  }  
}

/*************************            IR, WIFI, LED's on Core 1         ************************************/

void process_IR(){
  if (IrReceiver.decodedIRData.protocol != NEC) {
    // We have an unknown protocol here, print more info
    //IrReceiver.printIRResultRawFormatted(&Serial, true);
    } else {
      // Print a short summary of received data
      IrReceiver.printIRResultShort(&Serial);
      Serial.println();
    }
  IrReceiver.resume(); // Enable receiving of the next value
  //    0x18 = Up arrow - Increase brightness by 25
  if (IrReceiver.decodedIRData.command == 0x18) {
    if (OKAY_SHIFT == 0) {
      Serial.print("Setting brightness to ");
      Serial.println(ws2812fx.getBrightness() + 25);
      ws2812fx.setBrightness(ws2812fx.getBrightness() + 25);
    } else {
      ws2812fx.setColor(ws2812fx.getColor() + 1);
      Serial.print("Setting color to ");
      Serial.println(ws2812fx.getColor() + 1);
    }
  }
  //    0x52 = Down Arrow - Decrease brightness by 25
  else if (IrReceiver.decodedIRData.command == 0x52) {
    if (OKAY_SHIFT == 0) {
      Serial.print("Setting brightness to ");
      Serial.println(ws2812fx.getBrightness() - 25);
      ws2812fx.setBrightness(ws2812fx.getBrightness() - 25);
      } else {
        ws2812fx.setColor(ws2812fx.getColor() - 1);
        Serial.print("Setting color to ");
        Serial.println(ws2812fx.getColor() - 1);
      }
    }
  //    0x8 = Left arrow
  else if (IrReceiver.decodedIRData.command == 0x8) {
    if (OKAY_SHIFT == 0) {
      int lastMode = ws2812fx.getMode() - 1;
      Serial.print("Settting LED mode to ");
      Serial.println(lastMode);
      ws2812fx.setMode(lastMode);
    } else {
      int speedDecrease = ws2812fx.getSpeed() - 10;
      Serial.print("Settting LED speed mode to ");
      Serial.println(speedDecrease);
      ws2812fx.setMode(speedDecrease);
    }
  }
  //    0x5A = Right arrow
  else if (IrReceiver.decodedIRData.command == 0x5A) {
    if (OKAY_SHIFT == 0) {
      int nextMode = ws2812fx.getMode() + 1;
      Serial.print("Settting LED mode to ");
      Serial.println(nextMode);
      ws2812fx.setMode(nextMode);
    } else {
      int speedIncrease = ws2812fx.getSpeed() + 10;
      Serial.print("Settting LED speed mode to ");
      Serial.println(speedIncrease);
      ws2812fx.setMode(speedIncrease);
    }
  }
  //    0x1C = OK
  else if (IrReceiver.decodedIRData.command == 0x1C) {
    if (OKAY_SHIFT == 0) {
      OKAY_SHIFT++;
    } else if (OKAY_SHIFT == 1) {
      OKAY_SHIFT--;
    }
    Serial.println(OKAY_SHIFT);
  }
  //    0x45 = 1
  else if (IrReceiver.decodedIRData.command == 0x45) {
    ws2812fx.setMode(FX_MODE_CHASE_BLACKOUT_RAINBOW);
    Serial.print("Set LED mode to ");
    Serial.println("FX_MODE_CHASE_BLACKOUT_RAINBOW");
    }
  //    0x46 = 2
  else if (IrReceiver.decodedIRData.command == 0x46) {
    ws2812fx.setMode(FX_MODE_FIREWORKS_RANDOM);
    Serial.print("Set LED mode to ");
    Serial.println("FX_MODE_FIREWORKS_RANDOM");
  }
  //    0x47 = 3
  else if (IrReceiver.decodedIRData.command == 0x47) {
    ws2812fx.setMode(FX_MODE_HYPER_SPARKLE );
    Serial.print("Set LED mode to ");
    Serial.println("FX_MODE_HYPER_SPARKLE");
  }
  //    0x44 = 4
  else if (IrReceiver.decodedIRData.command == 0x44) {
    ws2812fx.setMode(FX_MODE_THEATER_CHASE);
    Serial.print("Set LED mode to ");
    Serial.println("FX_MODE_THEATER_CHASE");
  }
  //    0x40 = 5
  else if (IrReceiver.decodedIRData.command == 0x40) {
    ws2812fx.setMode(FX_MODE_COLOR_SWEEP_RANDOM);
    Serial.print("Set LED mode to ");
    Serial.println("FX_MODE_COLOR_SWEEP_RANDOM");
  }
  //    0x43 = 6
  else if (IrReceiver.decodedIRData.command == 0x43) {
    ws2812fx.setMode(FX_MODE_DUAL_SCAN);
    Serial.print("Set LED mode to ");
    Serial.println("FX_MODE_DUAL_SCAN");
  }
  //    0x7 = 7
  else if (IrReceiver.decodedIRData.command == 0x7) {
    ws2812fx.setMode(FX_MODE_MULTI_DYNAMIC);
    Serial.print("Set LED mode to ");
    Serial.println("FX_MODE_MULTI_DYNAMIC");
  }
  //    0x15 = 8
  else if (IrReceiver.decodedIRData.command == 0x15) {
    ws2812fx.setMode(FX_MODE_COLOR_WIPE_RANDOM);
    Serial.print("Set LED mode to ");
    Serial.println("FX_MODE_COLOR_WIPE_RANDOM");
  }
  //    0x9 = 9
  else if (IrReceiver.decodedIRData.command == 0x9) {
    ws2812fx.setMode(FX_MODE_FIRE_FLICKER);
    Serial.print("Set LED mode to ");
    Serial.println("FX_MODE_FIRE_FLICKER");
  }
  //    0x19 = 0
  else if (IrReceiver.decodedIRData.command == 0x19) {
    ws2812fx.setMode(FX_MODE_MERRY_CHRISTMAS );
    Serial.print("Set LED mode to ");
    Serial.println("FX_MODE_MERRY_CHRISTMAS");
  }
  //    0x16 = Asterisk
  else if (IrReceiver.decodedIRData.command == 0x16) {
    int randMode = random(1,65);
    ws2812fx.setMode(randMode);
    Serial.print("Set LED mode to a rAnDoM mode named ");
    Serial.println(ws2812fx.getModeName(randMode));
  }
  //    0xD = Pound sign. Use this to follow with 0xx for mode number, 1xx for brightness, 2xx for speed, followed by ok
  else if (IrReceiver.decodedIRData.command == 0x44) {
    /*   # Sign followed by 1xx can set the mode
    *    # Sign followed by 2xx can set the brighntess
    *    # Sign followed by 3xx will set speed    
    */
  }
}

unsigned long last_change, now, last_wifi_check = 0;

void ledTASK( void * pvParameters ){
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(ssid1, ssidPassphrase1);
  wifiMulti.addAP(ssid2, ssidPassphrase1);
  if(wifiMulti.run() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    }
  //IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK, USE_DEFAULT_FEEDBACK_LED_PIN);
  IrReceiver.begin(IR_RECEIVE_PIN, DISABLE_LED_FEEDBACK);
  Serial.print(F("\rNEC Protocol initiated. IR ready at hardware pin "));
  Serial.println(IR_RECEIVE_PIN);
  ws2812fx.init();
  ws2812fx.setBrightness(8);
  ws2812fx.setSpeed(1000);
  ws2812fx.setColor(0x007BFF);
  ws2812fx.setMode(FX_MODE_STATIC);
  ws2812fx.start();
  for(;;){
    ws2812fx.service();
    if (IrReceiver.decode()) {
      process_IR();
    }
  }
}
