#include <SD.h>
#include <SPI.h>
#include <SdFat.h>
#define SD_FAT_TYPE 2
//Initialize SD card on SdFat SoftSPI
// Chip select may be constant or RAM variable.
const uint8_t SD_CS_PIN = 33;
// Pin numbers in templates must be constants.
const uint8_t SOFT_MISO_PIN = 25;
const uint8_t SOFT_MOSI_PIN = 15;
const uint8_t SOFT_SCK_PIN  = 18;
// SdFat software SPI template
SoftSpiDriver<SOFT_MISO_PIN, SOFT_MOSI_PIN, SOFT_SCK_PIN> softSpi;
// Speed argument is ignored for software SPI.
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SD_SCK_MHZ(80), &softSpi)
// #define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(40), &softSpi)

SdFs sd;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting");
//  SPI.begin(18, 15, 25, 33);
//  if (!SD.begin(33)){
//    Serial.println("SD Failed");
//  }
//  else
//  {
//    Serial.println("SD Worked");
//  }
  if (!sd.begin(SD_CONFIG)){
    Serial.println("sdfat Borked");
  }
  else
  {
    Serial.println("sdfat Worked");
  }

}

void loop() {
  // put your main code here, to run repeatedly:

}
