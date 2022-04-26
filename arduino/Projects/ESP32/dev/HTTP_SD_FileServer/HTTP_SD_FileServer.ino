#include "SdFat.h"
#include "sdios.h"
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPI.h>
#include <TFT_eSPI.h>

// Chip select may be constant or RAM variable.
const uint8_t SD_CS_PIN = 5;
// Pin numbers in templates must be constants.
const uint8_t SOFT_MISO_PIN = 19;
const uint8_t SOFT_MOSI_PIN = 23;
const uint8_t SOFT_SCK_PIN  = 18;

// Try to select the best SD card configuration.
#if HAS_SDIO_CLASS
#define SD_CONFIG SdioConfig(FIFO_SDIO)
#elif ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(16))
#else  // HAS_SDIO_CLASS
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SD_SCK_MHZ(16))
#endif  // HAS_SDIO_CLASS
/* 
 *  SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h, 
 *  1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
*/
#define SD_FAT_TYPE 2

SdExFat sd;
ExFile file;
ExFile root;

//------------------------------------------------------------------------------

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

void setup() {

  Serial.begin(115200);
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  za = random(256);
  zb = random(256);
  zc = random(256);
  zx = random(256);
  // Wait for USB Serial
  while (!Serial) {
    yield();
  }
  delay(1000);
  Serial.println(F("Type any character to start\n"));
  while (!Serial.available()) {
    yield();
  }
  // Initialize the SD card.
  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt(&Serial);
  }
  int rootFileCount = 0;
  if (!root.open("/")) {
    Serial.println("Failed to open root /");
    return;
  }
  while (file.openNext(&root, O_RDONLY)) {
    if (!file.isHidden()) {
      rootFileCount++;
    }
  file.close();
  if (!file.open("SoftSPI.txt", O_RDWR | O_CREAT)) {
    sd.errorHalt(F("SD ERROR: Open file failed"));
  }
  file.println(F("This line was printed using software SPI."));
  file.rewind();
  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();
  Serial.println(F("SD Initialized."));
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.print("Connected with IP of: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    //request->send(SdFat, "/index.html", "text/html");
    request->send("/index.html", "text/html");
  });
//  server.serveStatic("/", "/");
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
