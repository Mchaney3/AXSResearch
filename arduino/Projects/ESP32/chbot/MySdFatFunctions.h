/**********   My Custom Libraries   ***********/



/********************************************/

#define SD_FAT_TYPE 2
//Initialize SD card on SdFat SoftSPI
// Pin numbers in templates must be constants.
// Chip select may be constant or RAM variable.
const uint8_t SD_CS_PIN = 33;
const uint8_t SOFT_SCK_PIN  = 32;
const uint8_t SOFT_MOSI_PIN = 27;
const uint8_t SOFT_MISO_PIN = 26;
// SdFat software SPI template
SoftSpiDriver<SOFT_MISO_PIN, SOFT_MOSI_PIN, SOFT_SCK_PIN> softSpi;
// Speed argument is ignored for software SPI.
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SD_SCK_MHZ(80), &softSpi)

SdExFat sd;
ExFile file;

void initSdFat() {
while (!Serial) {
    yield();
  }
  int cpuSpeed = getCpuFrequencyMhz();
  // Just to know which program is running on my Arduino
  Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));
  Serial.println("CPU running at " + String(cpuSpeed) + "MHz");
  Serial.println("");
  Serial.println("Type any character to start");
  while (!Serial.available()) {
    yield();
  }

  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt();
  }

  if (!file.open("SoftSPI.txt", O_RDWR | O_CREAT)) {
    sd.errorHalt(F("open failed"));
  }
  file.println(F("This line was printed using software SPI."));

  file.rewind();

  while (file.available()) {
    Serial.write(file.read());
  }

  file.close();

  Serial.println(F("Done."));
}
