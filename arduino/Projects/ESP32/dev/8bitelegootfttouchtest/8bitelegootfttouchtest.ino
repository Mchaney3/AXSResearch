/*
 * Hardware:
 * - TFT LCD (SD card + touch) using ILI9341 via 8bit parallel interface: http://www.lcdwiki.com/3.2inch_16BIT_Module_ILI9341_SKU:MRB3205
 * - ESP-WROOM-32 dev Board 
 * 
 * Wiring: just follow the pin definitios below
 * NOTE: In order to make everything work you HAVE to solder the SMD resistor (actually it's a jumper) in 8bit position.
 * */

#include <XPT2046_Touchscreen.h> //https://github.com/PaulStoffregen/XPT2046_Touchscreen
#include <SPI.h>  //https://github.com/espressif/arduino-esp32/tree/master/libraries/SPI
#include <TFT_eSPI.h> // https://github.com/Bodmer/TFT_eSPI

// HSPI under progress
/*#define CS_PIN 15
#define IRQ_PIN 35
#define MISO 12
#define MOSI 13
#define SCK 14*/

//VSPI
#define CS_PIN 5
#define IRQ_PIN 35
#define MISO 19   //    maybe swap
#define MOSI 23   //    maybe swap
#define SCK 18

/*
 * Go to Arduino's path and change the User_Setup.h file
 * In my case: "C:\Users\PUT_YOUR_USER_HERE\Documents\Arduino\libraries\TFT_eSPI\User_Setup.h"
 * 
 * 

#define ILI9341_DRIVER 
//Dont forget to change 
#define ESP32_PARALLEL
// The ESP32 and TFT the pins used for testing are:
#define TFT_CS   33  // Chip select control pin (library pulls permanently low
#define TFT_DC   22  // (RS) Data Command control pin - must use a pin in the range 0-31
#define TFT_RST  32  // Reset pin, toggles on startup
#define TFT_WR    21  // Write strobe control pin - must use a pin in the range 0-31
#define TFT_RD    15  // Read strobe control pin
#define TFT_D0   2  // Must use pins in the range 0-31 for the data bus
#define TFT_D1   13  // so a single register write sets/clears all bits.
#define TFT_D2   26  // Pins can be randomly assigned, this does not affect
#define TFT_D3   25  // TFT screen update performance.
#define TFT_D4   27
#define TFT_D5   12
#define TFT_D6   14
#define TFT_D7   4
*/

#define TFT_W 320
#define TFT_H 240

XPT2046_Touchscreen ts(CS_PIN, IRQ_PIN); // initializes
TFT_eSPI tft = TFT_eSPI();
bool ON_OFF = false; // flag that indicates the button state
bool canTouchThis = true; // flag created to avoid switching states while hold the button
float lcd_x = 0; //holds X touch position
float lcd_y = 0; //holds Y touch position

void setup(void) {
  Serial.begin(115200);
  tft.init();
  ts.begin();
  tft.setRotation(1);
  ts.setRotation(3);
  while (!Serial && (millis() <= 1000));
  drawScreen("Initiated...", ILI9341_WHITE, ILI9341_DARKGREY);
}

void loop() {
  if(ts.tirqTouched()){
    if (ts.touched()) {
      TS_Point p = ts.getPoint();
      //lcd_x = p.x/12.8;
      lcd_x = p.x/(4096/TFT_W); // calculates the point based on screen width
      //lcd_y = p.y/17.06;
      lcd_y = p.y/(4096/TFT_H); // calculates the point based on screen height
      if((lcd_x > 80 and lcd_x < 239) and (lcd_y > 70 and lcd_y < 169) and  canTouchThis){
        if(ON_OFF){
          drawScreen("[OFF]", ILI9341_WHITE, ILI9341_RED);
        }else{
          drawScreen("[ON]", ILI9341_BLACK,ILI9341_GREEN);}
        ON_OFF = !ON_OFF; //switch states
        canTouchThis = false;
      }
      Serial.print("(x,y) = (");
      Serial.print(lcd_x);
      Serial.print(",");
      Serial.print(lcd_y);
      Serial.print(")");
      Serial.println();
    }
  }else{canTouchThis = true;}
  delay(50); // delay used to make things slower in serial output
}

void drawScreen(String s, uint16_t color, uint16_t bg){
    tft.fillScreen(ILI9341_BLACK);
    
    tft.fillRoundRect(80, 70, 239-80,169-70, 3,bg);
    //tft.setTextColor(ILI9341_WHITE,TFT_DARK_GREY);
    tft.setTextColor(color);
    tft.drawCentreString(s, 160, 112,4);
    
    tft.setTextColor(ILI9341_GREEN,ILI9341_BLACK);
    tft.drawCentreString("Under development...", 160, 210,2);
  }

//Use native colors
//ILI9341_BLACK       0x0000
//ILI9341_NAVY        0x000F
//ILI9341_DARKGREEN   0x03E0
//ILI9341_DARKCYAN    0x03EF
//ILI9341_MAROON      0x7800
//ILI9341_PURPLE      0x780F
//ILI9341_OLIVE       0x7BE0
//ILI9341_LIGHTGREY   0xC618
//ILI9341_DARKGREY    0x7BEF
//ILI9341_BLUE        0x001F
//ILI9341_GREEN       0x07E0
//ILI9341_CYAN        0x07FF
//ILI9341_RED         0xF800
//ILI9341_MAGENTA     0xF81F
//ILI9341_YELLOW      0xFFE0
//ILI9341_WHITE       0xFFFF
//ILI9341_ORANGE      0xFD20
//ILI9341_GREENYELLOW 0xAFE5
//ILI9341_PINK        0xF81F
