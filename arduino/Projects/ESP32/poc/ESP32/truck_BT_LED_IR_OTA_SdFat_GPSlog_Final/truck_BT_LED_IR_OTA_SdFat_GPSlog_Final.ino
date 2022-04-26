//    32-BIT DAC A2DP_Sink with WS2182FX and bluetooth streaming/control

//    TODO: Figure out the symaphore to shut down BT on Core 0, from Core 1
//  Stabilize code with ILI9341 connected. Crashing with BT connected

// Pins 32 through 39 are only unused pins. 33-39 are input only

#include "credentials.h"
#include "BluetoothA2DPSink.h"
//BluetoothA2DPSink a2dp_sink;
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <WS2812FX.h>
#include "soc/soc.h" //disable brownout problems
#include "soc/rtc_cntl_reg.h"  //disable brownout problems
#include "Core0.h"
#include <Arduino.h>
#include <IRremote.hpp>
#include "SdFat.h"
#include <EEPROM.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <TimeLib.h>
#include "FS.h"

#define LED_COUNT       255
#define LED_PIN         2
#define DECODE_NEC      // Includes Apple and Onkyo
//#define DEBUG         // to see if attachInterrupt is used
//#define TRACE         // to see the state of the ISR state machine
#define IR_RECEIVE_PIN  39  // analog pin
#define STR_HELPER(x)   #x // IR Receiver helper
#define STR(x)          STR_HELPER(x) // IR Receiver Helper
int OKAY_SHIFT = 0;

#define SD_FAT_TYPE 2
//Initialize SD card on SdFat SoftSPI
// Pin numbers in templates must be constants.
const uint8_t SOFT_MISO_PIN = 19;
const uint8_t SOFT_MOSI_PIN = 23;
const uint8_t SOFT_SCK_PIN  = 25;
// Chip select may be constant or RAM variable.
const uint8_t SD_CS_PIN = 26;
// SdFat software SPI template
SoftSpiDriver<SOFT_MISO_PIN, SOFT_MOSI_PIN, SOFT_SCK_PIN> softSpi;
// Speed argument is ignored for software SPI.
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SD_SCK_MHZ(80), &softSpi)

#define GPSMODE 1
#define SERVERMODE 0
#define LED_BUILTIN -1
static const int RXPin = 16, TXPin = 17;
static const uint32_t GPSBaud = 9600;
const long GPSinterval = 15; //15sec Time_t

const int ADDR_EEPROM = 0;
const long LEDinterval = 1000; // 1sec

const char* ssid = "<YOUR_SSID>";
const char* password = "<YOUR_PASSWORD>";

bool isGPSMODE;
long prevMillis = 0;
int ledState = LOW;
const int offset = -5; // Pacific Standard Time
time_t prevDisplay = 0;
TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);

bool isGPSDataAvailable;
char timeBuffer[16];
char fileNameBuffer[16];
String latitude, longitude;

SdExFat sd;
ExFile file;
ExFile fileToAppend;
ExFile root;

WiFiMulti wifiMulti;
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
//                                    // the pointer is used by pushSprite() to push it onto the TFT

void process_IR(){    //    Need to write case for this instead of if, if, if, if
  if (IrReceiver.decodedIRData.protocol != NEC) {
    // We have an unknown protocol here, print more info
    //IrReceiver.printIRResultRawFormatted(&Serial, true);
    } else {
      // Print a short summary of received data
      IrReceiver.printIRResultShort(&Serial);
      Serial.println();
    }
  IrReceiver.resume(); // Enable receiving of the next value
  //    0x18 = Up arrow - Increase brightness by 25
  if (IrReceiver.decodedIRData.command == 0x18) {
    if (OKAY_SHIFT == 0) {
      Serial.print("Setting brightness to ");
      Serial.println(ws2812fx.getBrightness() + 25);
      ws2812fx.setBrightness(ws2812fx.getBrightness() + 25);
    } else {
      ws2812fx.setColor(ws2812fx.getColor() + 1);
      Serial.print("Setting color to ");
      Serial.println(ws2812fx.getColor() + 1);
    }
  }
  //    0x52 = Down Arrow - Decrease brightness by 25
  else if (IrReceiver.decodedIRData.command == 0x52) {
    if (OKAY_SHIFT == 0) {
      Serial.print("Setting brightness to ");
      Serial.println(ws2812fx.getBrightness() - 25);
      ws2812fx.setBrightness(ws2812fx.getBrightness() - 25);
      } else {
        ws2812fx.setColor(ws2812fx.getColor() - 1);
        Serial.print("Setting color to ");
        Serial.println(ws2812fx.getColor() - 1);
      }
    }
  //    0x8 = Left arrow
  else if (IrReceiver.decodedIRData.command == 0x8) {
    if (OKAY_SHIFT == 0) {
      int lastMode = ws2812fx.getMode() - 1;
      Serial.print("Settting LED mode to ");
      Serial.println(lastMode);
      ws2812fx.setMode(lastMode);
    } else {
      int speedDecrease = ws2812fx.getSpeed() - 10;
      Serial.print("Settting LED speed mode to ");
      Serial.println(speedDecrease);
      ws2812fx.setMode(speedDecrease);
    }
  }
  //    0x5A = Right arrow
  else if (IrReceiver.decodedIRData.command == 0x5A) {
    if (OKAY_SHIFT == 0) {
      int nextMode = ws2812fx.getMode() + 1;
      Serial.print("Settting LED mode to ");
      Serial.println(nextMode);
      ws2812fx.setMode(nextMode);
    } else {
      int speedIncrease = ws2812fx.getSpeed() + 10;
      Serial.print("Settting LED speed mode to ");
      Serial.println(speedIncrease);
      ws2812fx.setMode(speedIncrease);
    }
  }
  //    0x1C = OK
  else if (IrReceiver.decodedIRData.command == 0x1C) {
    if (OKAY_SHIFT == 0) {
      OKAY_SHIFT++;
    } else if (OKAY_SHIFT == 1) {
      OKAY_SHIFT--;
    }
    Serial.println(OKAY_SHIFT);
  }
  //    0x45 = 1
  else if (IrReceiver.decodedIRData.command == 0x45) {
    ws2812fx.setMode(FX_MODE_CHASE_BLACKOUT_RAINBOW);
    Serial.print("Set LED mode to ");
    Serial.println("FX_MODE_CHASE_BLACKOUT_RAINBOW");
    }
  //    0x46 = 2
  else if (IrReceiver.decodedIRData.command == 0x46) {
    ws2812fx.setMode(FX_MODE_FIREWORKS_RANDOM);
    Serial.print("Set LED mode to ");
    Serial.println("FX_MODE_FIREWORKS_RANDOM");
  }
  //    0x47 = 3
  else if (IrReceiver.decodedIRData.command == 0x47) {
    ws2812fx.setMode(FX_MODE_HYPER_SPARKLE );
    Serial.print("Set LED mode to ");
    Serial.println("FX_MODE_HYPER_SPARKLE");
  }
  //    0x44 = 4
  else if (IrReceiver.decodedIRData.command == 0x44) {
    ws2812fx.setMode(FX_MODE_THEATER_CHASE);
    Serial.print("Set LED mode to ");
    Serial.println("FX_MODE_THEATER_CHASE");
  }
  //    0x40 = 5
  else if (IrReceiver.decodedIRData.command == 0x40) {
    ws2812fx.setMode(FX_MODE_COLOR_SWEEP_RANDOM);
    Serial.print("Set LED mode to ");
    Serial.println("FX_MODE_COLOR_SWEEP_RANDOM");
  }
  //    0x43 = 6
  else if (IrReceiver.decodedIRData.command == 0x43) {
    ws2812fx.setMode(FX_MODE_DUAL_SCAN);
    Serial.print("Set LED mode to ");
    Serial.println("FX_MODE_DUAL_SCAN");
  }
  //    0x7 = 7
  else if (IrReceiver.decodedIRData.command == 0x7) {
    ws2812fx.setMode(FX_MODE_MULTI_DYNAMIC);
    Serial.print("Set LED mode to ");
    Serial.println("FX_MODE_MULTI_DYNAMIC");
  }
  //    0x15 = 8
  else if (IrReceiver.decodedIRData.command == 0x15) {
    ws2812fx.setMode(FX_MODE_COLOR_WIPE_RANDOM);
    Serial.print("Set LED mode to ");
    Serial.println("FX_MODE_COLOR_WIPE_RANDOM");
  }
  //    0x9 = 9
  else if (IrReceiver.decodedIRData.command == 0x9) {
    ws2812fx.setMode(FX_MODE_FIRE_FLICKER);
    Serial.print("Set LED mode to ");
    Serial.println("FX_MODE_FIRE_FLICKER");
  }
  //    0x19 = 0
  else if (IrReceiver.decodedIRData.command == 0x19) {
    ws2812fx.setMode(FX_MODE_MERRY_CHRISTMAS );
    Serial.print("Set LED mode to ");
    Serial.println("FX_MODE_MERRY_CHRISTMAS");
  }
  //    0x16 = Asterisk
  else if (IrReceiver.decodedIRData.command == 0x16) {
    int randMode = random(1,65);
    ws2812fx.setMode(randMode);
    Serial.print("Set LED mode to a rAnDoM mode named ");
    Serial.println(ws2812fx.getModeName(randMode));
  }
  //    0xD = Pound sign. Use this to follow with 0xx for mode number, 1xx for brightness, 2xx for speed, followed by ok
  else if (IrReceiver.decodedIRData.command == 0x44) {
    /*   # Sign followed by 1xx can set the mode
    *    # Sign followed by 2xx can set the brighntess
    *    # Sign followed by 3xx will set speed    
    */
  }
}

void initSdFat() {
  /*
while (!Serial) {
    yield();
  }
  while (!Serial.available()) {
    yield();
  }
  */
  int cpuSpeed = getCpuFrequencyMhz();
  // Just to know which program is running on my Arduino
  Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));
  Serial.println("CPU running at " + String(cpuSpeed) + "MHz");
  Serial.println("");
  //Serial.println("Type any character to start");
  
  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt();
  }

  if (!file.open("SoftSPI.txt", O_RDWR | O_CREAT)) {
    sd.errorHalt(F("open failed"));
  }
  file.println(F("SD Write Successful!"));

  file.rewind();

  while (file.available()) {
    Serial.write(file.read());
  }

  file.close();

  Serial.println(F("\nSD Card Initialized!"));
}

void dataLogging(){
  Serial.println("\n\n Data Logging");
  sprintf(fileNameBuffer, "/%02u-%02u-%04u.csv", month(), day(), year());
  bool isFileExist = sd.exists(fileNameBuffer);
  Serial.print("isFileExist: ");
  Serial.println(isFileExist);

  fileToAppend = sd.open(fileNameBuffer, O_CREAT | O_RDWR | O_APPEND);
  if(!fileToAppend){
    Serial.println("Error opening GPS Log");
    return;
  }

  if(!isFileExist){ //Header
    fileToAppend.print("Time");
    fileToAppend.print(",");
    fileToAppend.print("Latitude");
    fileToAppend.print(",");
    fileToAppend.println("Longitude");
  }
  
  sprintf(timeBuffer, "%02u:%02u:%02u", hour(), minute(), second());
  fileToAppend.print(timeBuffer);
  fileToAppend.print(",");
  fileToAppend.print(latitude);
  fileToAppend.print(",");
  fileToAppend.println(longitude);
  fileToAppend.close();
}

int readIntEEPROM(int address){
  int val;
  byte* p = (byte*) &val;
  *p = EEPROM.read(address);
  *(p+1) = EEPROM.read(address + 1);
  *(p+2) = EEPROM.read(address + 2);
  *(p+3) = EEPROM.read(address + 3);
  return val;
}

void writeIntEEPROM(int address, int value){
  byte *p = (byte*) &value;
  EEPROM.write(address, *p);
  EEPROM.write(address+1, *(p+1));
  EEPROM.write(address+2, *(p+2));
  EEPROM.write(address+3, *(p+3));
  EEPROM.commit();
}

void logGPS(){
  while (ss.available() > 0){
    if (gps.encode(ss.read())){
      isGPSDataAvailable = false;
      int Year = gps.date.year();
      byte Month = gps.date.month();
      byte Day = gps.date.day();
      byte Hour = gps.time.hour();
      byte Minute = gps.time.minute();
      byte Second = gps.time.second();
      setTime(Hour, Minute, Second, Day, Month, Year);
      adjustTime(offset * SECS_PER_HOUR);
      if(gps.location.isValid()){
        latitude = String(gps.location.lat(), 6);
        longitude = String(gps.location.lng(), 6);
        isGPSDataAvailable = true;
      }       
    }
    if(timeStatus() != timeNotSet){
      if(now() - prevDisplay >= GPSinterval){
        prevDisplay = now();
        if(isGPSDataAvailable){
          dataLogging();
        }
      }
    }
  }

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    //while(true);
  }
}

TaskHandle_t bt_stream;
SemaphoreHandle_t xSemaphore = NULL;

void setup() {
  
  Serial.begin(115200);
  ss.begin(GPSBaud);
  delay(1000);
  initSdFat();
  wifiMulti.addAP(ssid1, ssidPassphrase1);
  wifiMulti.addAP(ssid2, ssidPassphrase1);
  if(wifiMulti.run() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.print(WiFi.localIP());
    Serial.print("\r\nMac Address: ");
    Serial.println(WiFi.macAddress());
    }
  Serial.println("Initializing OTA");
  ArduinoOTA.setHostname(espHostName);
  ArduinoOTA.setPasswordHash(otaPasswordHash);
  ArduinoOTA
    .onStart([]() {
      noInterrupts();
      a2dp_sink.pause();
      a2dp_sink.set_stream_reader(read_data_stream, false);
      a2dp_sink.stop();
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
      ESP.restart();
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
  ArduinoOTA.begin();
  //IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK, USE_DEFAULT_FEEDBACK_LED_PIN);
  IrReceiver.begin(IR_RECEIVE_PIN, DISABLE_LED_FEEDBACK);
  Serial.print(F("\rNEC Protocol initiated. IR ready at hardware pin "));
  Serial.println(IR_RECEIVE_PIN);
  ws2812fx.init();
  ws2812fx.setBrightness(8);
  ws2812fx.setSpeed(1000);
  ws2812fx.setColor(0x007BFF);
  ws2812fx.setMode(FX_MODE_CHASE_RAINBOW);
  ws2812fx.start();
  
  xSemaphore = xSemaphoreCreateMutex();
  
  xTaskCreatePinnedToCore(
                    btTASK,   /* Task function. */
                    "bt_stream",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    2,           /* Always highest priority to assure audio quality */
                    &bt_stream,      /* Task handle to keep track of created task */
                    0);          /* pin Bluetooth to core o */                  
  delay(500);

}

void loop() {
  ws2812fx.service();
  if (IrReceiver.decode()) {
    process_IR();
  }
  logGPS();
}
