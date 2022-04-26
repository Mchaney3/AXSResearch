/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-web-server-microsd-card/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "FS.h"
#include <SD.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <SoftSPI.h>

#define CS_PIN 2
#define MISO 5
#define MOSI 15
#define SCK 25

// Create a new SPI port with:
// Pin 5 = MOSI,
// Pin 2 = MISO,
// Pin 25 = SCK
SoftSPI mySPI(5, 2, 15);

// Chip select may be constant or RAM variable.

TFT_eSPI tft = TFT_eSPI();

// With 1024 stars the update rate is ~65 frames per second
#define NSTARS 1024
uint8_t sx[NSTARS] = {};
uint8_t sy[NSTARS] = {};
uint8_t sz[NSTARS] = {};

uint8_t za, zb, zc, zx;

// Fast 0-255 random number generator from http://eternityforest.com/Projects/rng.php:
uint8_t __attribute__((always_inline)) rng()
{
  zx++;
  za = (za^zc^zx);
  zb = (zb+za);
  zc = ((zc+(zb>>1))^za);
  return zc;
}

// Replace with your network credentials
const char* ssid = "chlabs_bot";
const char* password = "chlabsrobotseverywhere";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);


void initSDCard(){
  if(!SD.begin(5)){
    Serial.println("SD begin failed");
    return;
  }
  uint8_t cardType = SD.cardType();
  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
    Serial.println("MMC");
  } else if(cardType == CARD_SD){
    Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }
  File sspitxt = SD.open("/SoftSPI.txt", FILE_WRITE);
  if (!sspitxt) {
    Serial.print(F("SD test open failed"));
    return;
  }
  sspitxt.println(F("This line was printed using software SPI."));
//  file.rewind();
  while (sspitxt.available()) {
    Serial.write(sspitxt.read());
    
  }
  sspitxt.close();
  Serial.println("SD Card Initialized");
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
  Serial.print("");
  
}

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void initLCD() {
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  za = random(256);
  zb = random(256);
  zc = random(256);
  zx = random(256);
}

void setup() {
//  spi.begin(25 /* SCK */, 2 /* MISO */, 5 /* MOSI */, 15 /* SS */);
  mySPI.begin();
  Serial.begin(115200);
  while (!Serial) {
    yield();
  }
  /*
  Serial.println("Type any character to start");
  while (!Serial.available()) {
    yield();
  }
  */
  initLCD();
  initWiFi();
  initSDCard();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SD, "/index.html", "text/html");
  });

  server.serveStatic("/", SD, "/");

  server.begin();
}

void loop() {
  unsigned long t0 = micros();
  uint8_t spawnDepthVariation = 255;

  for(int i = 0; i < NSTARS; ++i)
  {
    if (sz[i] <= 1)
    {
      sx[i] = 160 - 120 + rng();
      sy[i] = rng();
      sz[i] = spawnDepthVariation--;
    }
    else
    {
      int old_screen_x = ((int)sx[i] - 160) * 256 / sz[i] + 160;
      int old_screen_y = ((int)sy[i] - 120) * 256 / sz[i] + 120;

      // This is a faster pixel drawing function for occassions where many single pixels must be drawn
      tft.drawPixel(old_screen_x, old_screen_y,TFT_BLACK);

      sz[i] -= 2;
      if (sz[i] > 1)
      {
        int screen_x = ((int)sx[i] - 160) * 256 / sz[i] + 160;
        int screen_y = ((int)sy[i] - 120) * 256 / sz[i] + 120;
  
        if (screen_x >= 0 && screen_y >= 0 && screen_x < 320 && screen_y < 240)
        {
          uint8_t r, g, b;
          r = g = b = 255 - sz[i];
          tft.drawPixel(screen_x, screen_y, tft.color565(r,g,b));
        }
        else
          sz[i] = 0; // Out of screen, die.
      }
    }
  }
  unsigned long t1 = micros();
  //static char timeMicros[8] = {};

 // Calcualte frames per second
 // Serial.println(1.0/((t1 - t0)/1000000.0));
}
