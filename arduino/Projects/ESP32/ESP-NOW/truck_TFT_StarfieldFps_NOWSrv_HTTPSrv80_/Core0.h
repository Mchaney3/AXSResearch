#include <TFT_eSPI.h>
#include "soc/rtc_wdt.h"

//#define CONFIG_ESP_TASK_WDT 0
// Use hardware SPI
TFT_eSPI tft = TFT_eSPI();


// Stock font and GFXFF reference handle
#define GFXFF 1
#define FF18 &FreeSans12pt7b

// Custom are fonts added to library "TFT_eSPI\Fonts\Custom" folder
// a #include must also be added to the "User_Custom_Fonts.h" file
// in the "TFT_eSPI\User_Setups" folder. See example entries.
#define CF_OL24 &Orbitron_Light_24
#define CF_OL32 &Orbitron_Light_32
#define CF_RT24 &Roboto_Thin_24
#define CF_S24  &Satisfy_24
#define CF_Y32  &Yellowtail_32

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

unsigned long previousMillis = 0;        // will store last time LED was updated
const long interval = 3000;

void tftScreen( void * pvParameters ){
  
  za = random(256);
  zb = random(256);
  zc = random(256);
  zx = random(256);
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  for(;;){
    /*************    START UPDATING TFT    *****************/
    unsigned long t0 = micros();
  uint8_t spawnDepthVariation = 255;
  unsigned long t1 = micros();
  unsigned long currentMillis = millis();
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
  //static char timeMicros[8] = {};
  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    int fps = 1.0/((t1 - t0)/1000000.0);
    if (fps > 60) {
      tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    }
    if (fps > 120) {
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
    }
    if (fps < 60) {
      tft.setTextColor(TFT_RED, TFT_BLACK);
    }
    // Calcualte frames per second
    //tft.setTextDatum(MC_DATUM);
    
    tft.setTextPadding(80);
    tft.setFreeFont(CF_OL24);
    tft.drawString(String(fps), 5, 205, GFXFF);// Print the test text in the custom font
    // Reset text padding to zero (default)
    tft.setTextPadding(0);
    Serial.println(1.0/((t1 - t0)/1000000.0));
    }   
  delay(1);
  }
}
