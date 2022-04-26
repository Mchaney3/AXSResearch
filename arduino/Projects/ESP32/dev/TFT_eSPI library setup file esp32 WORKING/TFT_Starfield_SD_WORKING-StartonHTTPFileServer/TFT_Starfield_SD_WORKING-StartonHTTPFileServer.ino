// Animates white pixels to simulate flying through a star field

#include <SPI.h>
#include <TFT_eSPI.h>
#include "SdFat.h"
#include "sdios.h"

#define SD_FAT_TYPE 2
//Initialize SD card on SdFat SoftSPI
// Chip select may be constant or RAM variable.
const uint8_t SD_CS_PIN = 33;
// Pin numbers in templates must be constants.
const uint8_t SOFT_MISO_PIN = 25;
const uint8_t SOFT_MOSI_PIN = 15;
const uint8_t SOFT_SCK_PIN  = 14;
// SdFat software SPI template
SoftSpiDriver<SOFT_MISO_PIN, SOFT_MOSI_PIN, SOFT_SCK_PIN> softSpi;
// Speed argument is ignored for software SPI.
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SD_SCK_MHZ(50), &softSpi)
// #define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(40), &softSpi)

// Use hardware SPI
TFT_eSPI tft = TFT_eSPI();
SdExFat sd;
ExFile file;
ExFile root;

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

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  za = random(256);
  zb = random(256);
  zc = random(256);
  zx = random(256);

  // fastSetup() must be used immediately before fastPixel() to prepare screen
  // It must be called after any other graphics drawing function call if fastPixel()
  // is to be called again
  //  tft.fastSetup(); // Prepare plot window range for fast pixel plotting

//*************************   SD CARD   *******************

  if (!sd.begin(SD_CONFIG)) {
    Serial.println("OH SHIT SON! SHIT'S FUCKED");
    sd.initErrorHalt(&Serial);
    return;
  } else {
    Serial.println("SD Works Bitches");
  }
}

//***********************   LOOP    **********************

void loop()
{
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
//  Serial.println(1.0/((t1 - t0)/1000000.0));
}
