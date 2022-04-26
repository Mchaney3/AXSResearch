// regular Arduino prog with <SD.h>
//

#define USE_ADAFRUIT_GFX // Comment out to use TFT_eSPI

#define USE_LINE_BUFFER  // Enable for faster rendering

#if defined(USE_ADAFRUIT_GFX)
#define TFT_CS  27 //5
#define TFT_DC  25 //26
#define TFT_RST 26  //27
#define SD_CS   15
#include <Adafruit_ILI9341.h>
Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_RST);
#else
#include <TFT_eSPI.h>              // Hardware-specific library
TFT_eSPI tft = TFT_eSPI();         // Invoke custom library
#endif

#include <SD.h>
#include <math.h>

#include "pngle.h"

#define LINE_BUF_SIZE 64  // pixel = 524, 16 = 406, 32 = 386, 64 = 375, 128 = 368, 240 = 367, no draw = 324 (51ms v 200ms)
int16_t px = 0, sx = 0;
int16_t py = 0, sy = 0;
uint8_t pc = 0;
uint16_t lbuf[LINE_BUF_SIZE];

int16_t png_dx = 0, png_dy = 0;

// Define corner position
void setPngPosition(int16_t x, int16_t y)
{
    png_dx = x;
    png_dy = y;
}

// Draw pixel - called by pngle
void pngle_on_draw(pngle_t *pngle, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint8_t rgba[4])
{
    uint16_t color = (rgba[0] << 8 & 0xf800) | (rgba[1] << 3 & 0x07e0) | (rgba[2] >> 3 & 0x001f);

#if !defined(USE_ADAFRUIT_GFX) && defined(USE_LINE_BUFFER)
    color = (color << 8) | (color >> 8);
#endif

    if (rgba[3] > 127) { // Transparency threshold (no blending yet...)

#ifdef USE_LINE_BUFFER // This must handle skipped pixels in transparent PNGs
        if ( pc >= LINE_BUF_SIZE) {
#ifdef USE_ADAFRUIT_GFX
            tft.drawRGBBitmap(png_dx + sx, png_dy + sy, lbuf, LINE_BUF_SIZE, 1);
#else
            tft.pushImage(png_dx + sx, png_dy + sy, LINE_BUF_SIZE, 1, lbuf);
#endif
            px = x; sx = x; sy = y; pc = 0;
        }

        if ( (x == px) && (sy == y) && (pc < LINE_BUF_SIZE) ) {
            px++;
            lbuf[pc++] = color;
        }
        else {
#ifdef USE_ADAFRUIT_GFX
            tft.drawRGBBitmap(png_dx + sx, png_dy + sy, lbuf, pc, 1);
#else
            tft.pushImage(png_dx + sx, png_dy + sy, pc, 1, lbuf);
#endif
            px = x; sx = x; sy = y; pc = 0;
            px++; lbuf[pc++] = color;
        }
#else
        tft.drawPixel(x, y, color);
#endif
    }
}

void setup()
{
    Serial.begin(115200);
    tft.begin();

    if (!SD.begin(SD_CS)) {
        Serial.println("SD Mount Failed");
        return;
    }

}

void loop()
{
    load_file_SD(0, 0, "/EagleEye.jpg");
    load_file_SD(0, 0, "/Baboon40.jpg");
    load_file_SD(0, 0, "/lena20k.jpg");
    load_file_SD(0, 0, "/mouse480.jpg");
}

void load_file_SD(int x, int y, const char *path)
{
    File file = SD.open(path);
    if (!file) {
        Serial.print(path);
        Serial.println(": Failed to open file for reading");
        return ;
    }
    int32_t sz = file.size();
    
    tft.fillScreen(0);
    uint32_t t = millis();

    setPngPosition(x, y);
    pngle_t *pngle = pngle_new();
    pngle_set_draw_callback(pngle, pngle_on_draw);

    // Feed data to pngle
    uint8_t buf[1024];

    int remain = 0;
    int len;
#if !defined(USE_ADAFRUIT_GFX) && !defined(USE_LINE_BUFFER)
    tft.startWrite(); // Crashes Adafruit_GFX
#endif

    while ((len = file.read(buf + remain, sizeof(buf) - remain)) > 0) {
        int fed = pngle_feed(pngle, buf, remain + len);
        if (fed < 0) {
            Serial.print("ERROR: ");
            Serial.println(pngle_error(pngle));
            break;
        }

        remain = remain + len - fed;
        if (remain > 0) memmove(buf, buf + fed, remain);
    }
#ifdef USE_LINE_BUFFER
    // Draw any remaining pixels - had no warning that image has ended...
    if (pc) {
#ifdef USE_ADAFRUIT_GFX
        tft.drawRGBBitmap(png_dx + sx, png_dy + sy, lbuf, pc, 1);
#else
        tft.pushImage(png_dx + sx, png_dy + sy, pc, 1, lbuf);
#endif
        pc = 0;
    }
#endif
#if !defined(USE_ADAFRUIT_GFX) && !defined(USE_LINE_BUFFER)
    tft.endWrite();
#endif
    pngle_destroy(pngle);
    file.close();
    t = millis() - t;
    Serial.print(path);
    Serial.print(": (");
    Serial.print(sz); 
    Serial.print(" bytes) : ");
    Serial.print(t); 
    Serial.println(" ms");
    delay(2000);
}
