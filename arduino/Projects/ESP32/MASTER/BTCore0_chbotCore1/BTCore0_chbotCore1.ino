#define DYNAMIC_JSON_DOCUMENT_SIZE  2048 /* used by AsyncJson. Default is 1024, which is a little too small */
#include "credentials.h"
#include <WiFi.h>
#include "Core0.h"
#include <WS2812FX.h>
#include <ESPAsyncWebServer.h> /* https://github.com/me-no-dev/ESPAsyncWebServer */
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>
#include <SD.h>
#include "SdFat.h"
#include "sdios.h"
#include "bundle.css.h"
#include "bundle.js.h"
#include "app.css.h"
#include "app.js.h"
#include "material_icons_subset.woff2.h"
#include "index.html.h"

#define LED_PIN   25  // digital pin used to drive the LED strip
#define LED_COUNT 8  // number of LEDs on the strip
#define EEPROM_SIZE 4096
#define PRESETS_MAX_SIZE (EEPROM_SIZE - sizeof(int) - sizeof(preset) - 1) // maximum size of the presets string
#define DEFAULT_PRESETS   "[{" \
  "\"name\":\"Default\",\"pin\":%d,\"numPixels\":%d,\"brightness\":64," \
  "\"segments\":[{\"start\":0,\"stop\":%d,\"mode\":0,\"speed\":1000,\"options\":0," \
    "\"colors\":[\"#ff0000\",\"#00ff00\",\"#0000ff\"]" \
  "}]" \
"}]"
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
AsyncWebServer server(80);

struct Preset {
  int  pin = LED_PIN;
  int  numPixels = LED_COUNT;
  int  brightness = 64;
  int  numSegments = 1;
  WS2812FX::Segment segments[MAX_NUM_SEGMENTS] = {
    {0, LED_COUNT - 1, 1000, FX_MODE_STATIC, NO_OPTIONS, {RED, GREEN, BLUE}}
  };
};
Preset preset; // note: "preset" is a Preset data struct

char presets[PRESETS_MAX_SIZE] = "[]"; // note: "presets" is a JSON string

#define SD_FAT_TYPE 2
//Initialize SD card on SdFat SoftSPI
// Chip select may be constant or RAM variable.
const uint8_t SD_CS_PIN = 33;
// Pin numbers in templates must be constants.
const uint8_t SOFT_SCK_PIN  = 32;
const uint8_t SOFT_MOSI_PIN = 27;
const uint8_t SOFT_MISO_PIN = 26;
// SdFat software SPI template
SoftSpiDriver<SOFT_MISO_PIN, SOFT_MOSI_PIN, SOFT_SCK_PIN> softSpi;
// Speed argument is ignored for software SPI.
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SD_SCK_MHZ(50), &softSpi)
SdExFat sd;
ExFile file;
ExFile root;

// config web server to serve "files"
void initWebServer() {
  server.onNotFound([](AsyncWebServerRequest * request) {
    request->send(404, "text/plain", "Page not found");
  });

  // return the index.html file
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", index_html);
  });

  server.on("/bundle.css", HTTP_GET, [] (AsyncWebServerRequest * request) {
    Serial.print("Sending bundle.css: "); Serial.println(bundle_css_len); // debug
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/css", bundle_css, bundle_css_len);
    response->addHeader("Expires", "Fri, 1 Jan 2100 9:00:00 GMT");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  server.on("/app.css", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/css", app_css);
  });

  server.on("/bundle.js", HTTP_GET, [] (AsyncWebServerRequest * request) {
    Serial.print("Sending bundle.js: "); Serial.println(bundle_js_len); // debug
    AsyncWebServerResponse *response = request->beginResponse_P(200, "application/javascript", bundle_js, bundle_js_len);
    response->addHeader("Expires", "Fri, 1 Jan 2100 9:00:00 GMT");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  server.on("/app.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "application/javascript", app_js);
  });

  server.on("/Material-Icons-subset.woff2", HTTP_GET, [] (AsyncWebServerRequest * request) {
    Serial.print("Sending Material-Icons-subset.woff2: "); Serial.println(material_icons_subset_len); // debug
    AsyncWebServerResponse *response = request->beginResponse_P(200, "font/woff2", material_icons_subset, material_icons_subset_len);
    response->addHeader("Expires", "Fri, 1 Jan 2100 9:00:00 GMT");
    request->send(response);
  });
}

// config web server to process API calls
void initWebAPI() {
  // send the presets info
  server.on("/loadpresets", HTTP_GET, [] (AsyncWebServerRequest * request) {
    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", presets);
    request->send(response);
  });

  // receive the presets info as a String and save to EEPROM
  server.on("/savepresets", HTTP_POST, [](AsyncWebServerRequest * request) {
    if (request->hasParam("presets", true)) {
      String data = request->getParam("presets", true)->value();
      Serial.println(data); // debug
      if(data.length() < PRESETS_MAX_SIZE) {
        strcpy(presets, data.c_str());
        saveToEEPROM(); // save presets to EEPROM
      } else {
        request->send(413, "text/plain", "Error: Preset string is too big.");
        return;
      }
    }
    request->send(200, "text/plain", "OK");
  });

  // receive the preset info in JSON format and setup the WS2812 strip
  AsyncCallbackJsonWebHandler* handler = new AsyncCallbackJsonWebHandler("/savePreset", [](AsyncWebServerRequest * request, JsonVariant & json) {
    JsonObject jsonObj = json.as<JsonObject>();
    serializeJson(jsonObj, Serial); Serial.println(); // debug
    preset.pin = jsonObj["pin"];
    preset.numPixels = jsonObj["numPixels"];
    preset.brightness = jsonObj["brightness"];

    JsonArray segments = jsonObj["segments"];
    preset.numSegments = segments.size();
    for (size_t i = 0; i < segments.size(); i++) {
      JsonObject seg = segments[i];

      preset.segments[i].start = seg["start"];
      preset.segments[i].stop = seg["stop"];
      preset.segments[i].mode = seg["mode"];
      preset.segments[i].speed = seg["speed"];
      preset.segments[i].options = seg["options"];

      JsonArray colors = seg["colors"];
      preset.segments[i].colors[0] = colors[0];
      preset.segments[i].colors[1] = colors[1];
      preset.segments[i].colors[2] = colors[2];
    }

    updateWs2812fx(); // update the ws2812fx object using preset info
    request->send(200, "text/plain", "OK");
  });
  server.addHandler(handler);

  // control run / stop / pause / resume
  server.on("/runcontrol", HTTP_POST, [](AsyncWebServerRequest * request) {
    showReqParams(request); // debug
    if (request->hasParam("action", true)) {
      String action = request->getParam("action", true)->value();
      if (action == "pause") {
        ws2812fx.pause();
      } else if (action == "resume") {
        ws2812fx.resume();
      } else if (action == "run") {
        ws2812fx.start();
      }  else if (action == "stop") {
        ws2812fx.stop();
      }
    }
    request->send(200, "text/plain", "OK");
  });

  // send the WS2812FX mode info in JSON format
  server.on("/getmodes", HTTP_GET, [] (AsyncWebServerRequest * request) {
    char modes[1000] = "[";
    for (uint8_t i = 0; i < ws2812fx.getModeCount(); i++) {
      strcat(modes, "\"");
      strcat_P(modes, (PGM_P)ws2812fx.getModeName(i));
      strcat(modes, "\",");
    }
    modes[strlen(modes) - 1] = ']';

    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", modes);
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });
}

// debug function to print HTTP request parameters
void showReqParams(AsyncWebServerRequest * request) {
  Serial.println("HTTP request parameters:");
  int params = request->params();
  for (int i = 0; i < params; i++) {
    AsyncWebParameter* p = request->getParam(i);
    if (p->isFile()) { //p->isPost() is also true
      Serial.printf("FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
    } else if (p->isPost()) {
      Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
    } else {
      Serial.printf("GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
    }
  }
}

void updateWs2812fx() {
  ws2812fx.stop();
  ws2812fx.strip_off();
  ws2812fx.setLength(preset.numPixels);
  ws2812fx.setPin(preset.pin);
  ws2812fx.stop(); // reset strip again in case length was increased
  ws2812fx.setBrightness(preset.brightness);
  ws2812fx.resetSegments();
  for (int i = 0; i < preset.numSegments; i++) {
    WS2812FX::Segment seg = preset.segments[i];
    ws2812fx.setSegment(i, seg.start, seg.stop, seg.mode, seg.colors, seg.speed, seg.options);
  }
  ws2812fx.start();
}

#define EEPROM_MAGIC_NUMBER 0x010e0d06
void saveToEEPROM() {
  Serial.println("saving to EEPROM");
  EEPROM.put(sizeof(int) * 0, (int)EEPROM_MAGIC_NUMBER);
  EEPROM.put(sizeof(int) * 1, preset);
  EEPROM.put(sizeof(int) * 1 + sizeof(preset), presets);
  EEPROM.commit(); // for ESP8266 (comment out if using an Arduino)
}

void restoreFromEEPROM() {
  int magicNumber = 0;
  EEPROM.get(sizeof(int) * 0, magicNumber);
  if (magicNumber == EEPROM_MAGIC_NUMBER) {
    Serial.println("restoring from EEPROM");
    EEPROM.get(sizeof(int) * 1, preset);
    EEPROM.get(sizeof(int) * 1 + sizeof(preset), presets);
    updateWs2812fx();
  }
}

void initOTA() {
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
   ArduinoOTA.setHostname(espHostName);

  // No authentication by default
  ArduinoOTA.setPasswordHash(otaHash);

  ArduinoOTA.onStart([]() {
    Serial.println("OTA start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nOTA end");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}

TaskHandle_t bt_stream;

void setup() {

  Serial.begin(115200);
  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt();
  }
  Serial.println("\r\nSD Initialized");
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  uint8_t mac[6];
  WiFi.softAPmacAddress(mac);
  Serial.printf("\r\nstatic uint8_t PEER[]{0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X};\n", mac[0],
                mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.println();
  Serial.print("\nServer IP is ");
  Serial.println(WiFi.localIP());
  EEPROM.begin(EEPROM_SIZE); // for ESP8266 (comment out if using an Arduino)

  // build the default presets string using the default LED_PIN and LED_COUNT values
  sprintf_P(presets, PSTR(DEFAULT_PRESETS), LED_PIN, LED_COUNT, LED_COUNT - 1);
  Serial.println(presets); // debug

  Serial.print("\r\nPoint your browser to http://ch_bot.chlabs.com or http://");
  Serial.println(WiFi.localIP());

  initWebServer();
  initWebAPI();

  // start the web server
  server.begin();

  // init LED strip with a default segment
  ws2812fx.init();
  updateWs2812fx();

  // if segment data had been previously saved to eeprom, load that data
  restoreFromEEPROM();

  ws2812fx.start();
  
  xTaskCreatePinnedToCore(
                    btTASK,   /* Task function. */
                    "bt_stream",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    0,           /* Always highest priority to assure audio quality */
                    &bt_stream,      /* Task handle to keep track of created task */
                    0);          /* pin Bluetooth to core 0 */                  
  delay(500);
  Serial.println("Bluetooth Started");
}




void loop() {
  ws2812fx.service();
  ArduinoOTA.handle();
}
