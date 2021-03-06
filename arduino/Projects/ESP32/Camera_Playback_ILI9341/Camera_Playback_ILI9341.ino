// ArduCAM demo (C)2017 Lee
// Web: http://www.ArduCAM.com
// This program is a demo of how to use most of the functions
// of the library with a supported camera modules.
//This demo can only work on ARDUCAM_SHIELD_V2 platform.
//This demo is compatible with ESP8266
// This demo was made for Omnivision OV2640/OV5640/OV5642/ sensor.
// It will turn the ArduCAM into a real digital camera with capture and playback functions.
// 1. Preview the live video on LCD Screen.
// 2. Capture and buffer the image to FIFO when shutter pressed quickly.
// 3. Store the image to Micro SD/TF card with BMP format.
// 4. Playback the capture photos one by one when shutter buttom hold on for 3 seconds.
// This program requires the ArduCAM V4.0.0 (or later) library and ArduCAM shield V2
// and use Arduino IDE 1.6.8 compiler or above
#include "xbm.h"             // Sketch tab header for xbm images
#include <TFT_eSPI.h>        // Hardware-specific library
//#include <SD.h>
#include "SdFat.h"
#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include "memorysaver.h"
//This demo was made for Omnivision MT9D111A/MT9D111B/MT9M112/MT9V111_CAM/
//                                  MT9M001/MT9T112/MT9D112/OV7670/OV7675/
//                                  OV7725/OV2640/OV3640/OV5640/OV5642 sensor.

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

 const int SPI_CS = 32;

#define BMPIMAGEOFFSET 66
const int bmp_header[BMPIMAGEOFFSET] PROGMEM =
{
  0x42, 0x4D, 0x36, 0x58, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x00, 0x00, 0x00, 0x28, 0x00,
  0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x01, 0x00, 0x10, 0x00, 0x03, 0x00,
  0x00, 0x00, 0x00, 0x58, 0x02, 0x00, 0xC4, 0x0E, 0x00, 0x00, 0xC4, 0x0E, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0xE0, 0x07, 0x00, 0x00, 0x1F, 0x00,
  0x00, 0x00
};

ArduCAM myCAM(OV2640, SPI_CS);
TFT_eSPI tft = TFT_eSPI(SPI_CS);   // Invoke library
SdExFat sd;
ExFile file;
void setup()
{
uint8_t vid, pid;
uint8_t temp;
Wire.begin();
Serial.begin(115200);
Serial.println(F("ArduCAM Start!"));
// set the SPI_CS as an output:
pinMode(SPI_CS, OUTPUT);
//initialize SPI:
SPI.begin(14, 15, 25, 32);
while(1){
  //Check if the ArduCAM SPI bus is OK
  myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM.read_reg(ARDUCHIP_TEST1);
  if (temp != 0x55)
  {
    Serial.println(F("SPI interface Error!"));
    delay(1000); continue;    
  } else{
    Serial.println(F("SPI interface OK"));break;
  } 	  
}

while(1){
  //Check if the camera module type is OV2640
  myCAM.wrSensorReg8_8(0xff, 0x01);
  myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
  if ((vid != 0x26 ) && (( pid != 0x41 ) || ( pid != 0x42 ))){
    Serial.println(F("Can't find OV2640 module!"));
    delay(1000);continue;
  } else{
    Serial.println(F("OV2640 detected."));break;
  }
}

//Change MCU mode
myCAM.set_mode(MCU2LCD_MODE);
//Initialize the LCD Module
tft.begin();               // Initialise the display
tft.fillScreen(TFT_BLACK); // Black screen fill
myCAM.InitCAM();
//Initialize SD Card
while(!sd.begin(SD_CONFIG)){
  Serial.println(F("SD Card Error"));delay(1000);
}
 Serial.println(F("SD Card detected!"));
}
void loop()
{
char str[8];
unsigned long previous_time = 0;
static int k = 0;
myCAM.set_mode(CAM2LCD_MODE);		 	//Switch to CAM
while (1)
{
  if (!myCAM.get_bit(ARDUCHIP_TRIG, VSYNC_MASK))		//New Frame is coming
  {
    myCAM.set_mode(MCU2LCD_MODE);    	//Switch to MCU
    tft.drawXBitmap(x, y, logo, logoWidth, logoHeight, TFT_BLACK);
    myCAM.set_mode(CAM2LCD_MODE);    	//Switch to CAM
    while (!myCAM.get_bit(ARDUCHIP_TRIG, VSYNC_MASK)); 	//Wait for VSYNC is gone
  }
  else if (myCAM.get_bit(ARDUCHIP_TRIG, SHUTTER_MASK))
  {
    previous_time = millis();
    while (myCAM.get_bit(ARDUCHIP_TRIG, SHUTTER_MASK))
    {
      if ((millis() - previous_time) > 1500)
      {
        Playback();
      }
    }
    if ((millis() - previous_time) < 1500)
    {
      k = k + 1;
      itoa(k, str, 10);
      strcat(str, ".bmp");				//Generate file name
      myCAM.set_mode(MCU2LCD_MODE);    	//Switch to MCU, freeze the screen
      GrabImage(str);
    }
  }
}
}
void GrabImage(char* str)
{
file outFile;
char VH, VL;
byte buf[256];
static int k = 0;
int i, j = 0;
outFile = sd.open(str, O_WRITE | O_CREAT | O_TRUNC);
//outFile = SD.open(str, FILE_WRITE);
if (! outFile)
{
  Serial.println(F("File open error"));
  return;
}
//Flush the FIFO
myCAM.flush_fifo();
//Start capture
myCAM.start_capture();
Serial.println(F("Start Capture"));
//Polling the capture done flag
while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK));
Serial.println(F("Capture Done."));
k = 0;
//Write the BMP header
for ( i = 0; i < BMPIMAGEOFFSET; i++)
{
  char ch = pgm_read_byte(&bmp_header[i]);
  buf[k++] = ch;
}
outFile.write(buf, k);
k = 0;
//Read 320x240x2 byte from FIFO
//Save as RGB565 bmp format
for (i = 0; i < 240; i++)
for (j = 0; j < 320; j++)
{
  VH = myCAM.read_fifo();
  VL = myCAM.read_fifo();
  buf[k++] = VL;
  buf[k++] = VH;
  //Write image data to bufer if not full
  if (k >= 256)
  {
    //Write 256 bytes image data to file from buffer
    outFile.write(buf, 256);
    k = 0;
  }
}
//Close the file
outFile.close();
//Clear the capture done flag
myCAM.clear_fifo_flag();
//Switch to LCD Mode
myCAM.write_reg(ARDUCHIP_TIM, 0);
return;
}
void Playback()
{
  file inFile;
  char str[8];
  int k = 0;
  myCAM.set_mode(MCU2LCD_MODE);    		//Switch to MCU
  tft.drawXBitmap(x, y, logo, logoWidth, logoHeight, TFT_BLACK);
  while (1)
  {
    k = k + 1;
    itoa(k, str, 10);
    strcat(str, ".bmp");
    inFile = sd.open(str, FILE_READ);
    if (! inFile)
    return;
    tft.drawXBitmap(x, y, logo, logoWidth, logoHeight, TFT_BLACK, TFT_BLACK);
    //myGLCD.resetXY();
    dispBitmap(inFile);
    inFile.close();
    delay(2000);
  }
}
//Only support RGB565 bmp format
void dispBitmap(File inFile)
{ 
  char VH = 0, VL = 0;
  int i, j = 0;
  for (i = 0 ; i < BMPIMAGEOFFSET; i++)
  inFile.read();
  for (i = 0; i < 320; i++)
  for (j = 0; j < 240; j++)
  {
    VL = inFile.read();
    //Serial.write(VL);
    VH = inFile.read();
    //Serial.write(VH);
    #if defined(ESP8266)
      yield();
    #endif
    tft.drawXBitmap(VL, VH, logo, logoWidth, logoHeight, TFT_WHITE);
  }
  tft.drawXBitmap(x, y, logo, logoWidth, logoHeight, TFT_BLACK, TFT_BLACK);
}
