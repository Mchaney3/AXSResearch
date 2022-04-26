/*
This top section might be working. Copying changes after this comment block from template file to test.
Why does this work after changing my pins though?
*/

// See SetupX_Template.h for all options available


#define TFT_PARALLEL_8_BIT
#define ESP32_PARALLEL
#define ILI9341_DRIVER 
//sd 32
//camera 33
//mosi 5
//miso 12
//sck 4
// The ESP32 and TFT the pins used for testing are:
#define TFT_CS   33  // Chip select control pin (library pulls permanently low
#define TFT_RST  32  // Reset pin, toggles on startup
#define TFT_DC   27  // (RS) Data Command control pin - must use a pin in the range 0-31
#define TFT_WR   26  // Write strobe control pin - must use a pin in the range 0-31
#define TFT_RD   -1  // Read strobe control pin
#define TFT_D0   5  // Must use pins in the range 0-31 for the data bus
#define TFT_D1   13  // so a single register write sets/clears all bits.
#define TFT_D2   22  // Pins can be randomly assigned, this does not affect
#define TFT_D3   21  // TFT screen update performance.
#define TFT_D4   19
#define TFT_D5   18
#define TFT_D6   17
#define TFT_D7   12

#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
#define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
#define LOAD_FONT6  // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
#define LOAD_FONT7  // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:.
#define LOAD_FONT8  // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.
#define LOAD_GFXFF  // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts
//#define SPI_FREQUENCY  80000000
//#define SPI_READ_FREQUENCY  40000000
//#define TFT_SDA_READ
// #define USE_HSPI_PORT

#define SMOOTH_FONT
