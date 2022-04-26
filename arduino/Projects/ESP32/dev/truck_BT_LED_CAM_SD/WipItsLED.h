#include <WiFi.h>
#include <WiFiMulti.h>
#include <WS2812FX.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#define LED_COUNT 300
#define LED_PIN 2
#define TIMER_MS 5000
#define WiFiTimer_MS 10000

WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
WiFiMulti wifiMulti;

void btTASK( void * pvParameters ){
  
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
    .bck_io_num = 15,   //    BCK
    .ws_io_num = 13,    //    LRCK
    .data_out_num = 14,   //    DIN
    .data_in_num = I2S_PIN_NO_CHANGE
  };
  a2dp_sink.set_pin_config(my_pin_config);
  a2dp_sink.set_volume(75);
  a2dp_sink.start(btDeviceName);
  for(;;){
    delay(1000);    
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
  ArduinoOTA.setHostname(espHostName);
  ArduinoOTA.setPasswordHash(otaPasswordHash);
  ArduinoOTA
    .onStart([]() {
      vTaskDelete(btStream);
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

  ArduinoOTA.begin();
  ws2812fx.init();
  ws2812fx.setBrightness(8);
  ws2812fx.setSpeed(1000);
  ws2812fx.setColor(0x007BFF);
  ws2812fx.setMode(FX_MODE_STATIC);
  ws2812fx.start();
  for(;;){
    ws2812fx.service();
    now = millis();
    if(now - last_change > TIMER_MS) {
      ws2812fx.setMode((ws2812fx.getMode() + 1) % ws2812fx.getModeCount());
      last_change = now;
      }
    ArduinoOTA.handle();
    yield();
  } 
}
