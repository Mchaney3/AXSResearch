#define DYNAMIC_JSON_DOCUMENT_SIZE  2048 /* used by AsyncJson. Default is 1024, which is a little too small */

#include <WS2812FX.h>
#include <ESPAsyncWebServer.h> /* https://github.com/me-no-dev/ESPAsyncWebServer */
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>
#include "Arduino.h"
#include "WiFiMulti.h"
#include "Audio.h"
#include "bundle.css.h"
#include "bundle.js.h"
#include "app.css.h"
#include "app.js.h"
#include "material_icons_subset.woff2.h"
#include "index.html.h"
#include "credentials.h"

#define LED_PIN     18 // digital pin used to drive the LED strip
#define LED_COUNT 256 // number of LEDs on the strip

#define WIFI_SSID1 ssid1     // WiFi network
#define WIFI_PASSWORD1 ssidPassphrase1 // WiFi network password

#define I2S_LRC       4
#define I2S_DOUT      5
#define I2S_BCLK      6

#define EEPROM_SIZE 4096
#define PRESETS_MAX_SIZE (EEPROM_SIZE - sizeof(int) - sizeof(preset) - 1) // maximum size of the presets string
#define DEFAULT_PRESETS   "[{" \
  "\"name\":\"Default\",\"pin\":%d,\"numPixels\":%d,\"brightness\":64," \
  "\"segments\":[{\"start\":0,\"stop\":%d,\"mode\":0,\"speed\":1000,\"options\":0," \
    "\"colors\":[\"#ff0000\",\"#00ff00\",\"#0000ff\"]" \
  "}]" \
"}]"

WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
Audio audio;

WiFiMulti wifiMulti;

int pirPin = 40;                 // PIR Out pin 
int pirStat = 0;                   // PIR status

AsyncWebServer server(80);

struct Preset {
  int  pin = LED_PIN;
  int  numPixels = LED_COUNT;
  int  brightness = 16;
  int  numSegments = 1;
  int  speed = 200;
  WS2812FX::Segment segments[MAX_NUM_SEGMENTS] = {
    {0, LED_COUNT - 1, 1000, FX_MODE_CHASE_BLACKOUT_RAINBOW, NO_OPTIONS, {RED, GREEN, BLUE}}
  };
};
Preset preset; // note: "preset" is a Preset data struct

char presets[PRESETS_MAX_SIZE] = "[]"; // note: "presets" is a JSON string

void setup() {
  Serial.begin(115200);
  pinMode(pirPin, INPUT);
  EEPROM.begin(EEPROM_SIZE); // for ESP8266 (comment out if using an Arduino)

  // build the default presets string using the default LED_PIN and LED_COUNT values
  sprintf_P(presets, PSTR(DEFAULT_PRESETS), LED_PIN, LED_COUNT, LED_COUNT - 1);
  Serial.println(presets); // debug

  // init WiFi
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID1, WIFI_PASSWORD1);
  wifiMulti.run();
  if(WiFi.status() != WL_CONNECTED){
    WiFi.disconnect(true);
    wifiMulti.run();
  }
  Serial.print("\r\nPoint your browser to ");
  Serial.println(WiFi.localIP());

  initWebServer();
  initWebAPI();

  //  Audio setup
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(12); // 0...21
  audio.connecttohost("http://mp3.ffh.de/radioffh/hqlivestream.mp3"); //  128k mp3
  
  // start the web server
  server.begin();

  // init LED strip with a default segment
  ws2812fx.init();
    ws2812fx.setMode(FX_MODE_COLOR_SWEEP_RANDOM);
  updateWs2812fx();

  // if segment data had been previously saved to eeprom, load that data
  restoreFromEEPROM();

  ws2812fx.start();
}

void loop() {
  ws2812fx.service();   //  Macro run in checkMotionOmaha
  ArduinoOTA.handle();
  audio.loop();
    if(Serial.available()){ // put streamURL in serial monitor
        audio.stopSong();
        String r=Serial.readString(); r.trim();
        if(r.length()>5) audio.connecttohost(r.c_str());
        log_i("free heap=%i", ESP.getFreeHeap());
    }
  //  checkMotionOmaha();
}

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

void checkMotionOmaha(){
  pirStat = digitalRead(pirPin); 
  if (pirStat == HIGH) {            // if motion detected
    saveToEEPROM();
    int omaha = millis();
    int hike = 5000;
    Serial.println("Motion in the ocean Cap'n!!!");
    ws2812fx.setBrightness(200);
    ws2812fx.setColor(RED);
    ws2812fx.setMode(FX_MODE_BLINK);
    while ((omaha - millis()) <= hike) {
      ws2812fx.service();
    }
    restoreFromEEPROM();
  }
  else {
/*
    ws2812fx.setBrightness(32);
    ws2812fx.setSpeed(200);
    ws2812fx.setMode(FX_MODE_COLOR_SWEEP_RANDOM);
    updateWs2812fx();
*/
  }
}

void initOTA() {
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

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

// optional
void audio_info(const char *info){
    Serial.print("info        "); Serial.println(info);
}
void audio_id3data(const char *info){  //id3 metadata
    Serial.print("id3data     ");Serial.println(info);
}
void audio_eof_mp3(const char *info){  //end of file
    Serial.print("eof_mp3     ");Serial.println(info);
}
void audio_showstation(const char *info){
    Serial.print("station     ");Serial.println(info);
}
void audio_showstreamtitle(const char *info){
    Serial.print("streamtitle ");Serial.println(info);
}
void audio_bitrate(const char *info){
    Serial.print("bitrate     ");Serial.println(info);
}
void audio_commercial(const char *info){  //duration in sec
    Serial.print("commercial  ");Serial.println(info);
}
void audio_icyurl(const char *info){  //homepage
    Serial.print("icyurl      ");Serial.println(info);
}
void audio_lasthost(const char *info){  //stream URL played
    Serial.print("lasthost    ");Serial.println(info);
}
void audio_eof_speech(const char *info){
    Serial.print("eof_speech  ");Serial.println(info);
}
