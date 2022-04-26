#include <IBusBM.h>
#include <ESP32Servo.h>
#include "SdFat.h"
#include "sdios.h"

#define SD_FAT_TYPE 2
//Initialize SD card on SdFat SoftSPI
// Chip select may be constant or RAM variable.
const uint8_t SD_CS_PIN = 33;
// Pin numbers in templates must be constants.
const uint8_t SOFT_MISO_PIN = 15;
const uint8_t SOFT_MOSI_PIN = 14;
const uint8_t SOFT_SCK_PIN  = 13;
// SdFat software SPI template
SoftSpiDriver<SOFT_MISO_PIN, SOFT_MOSI_PIN, SOFT_SCK_PIN> softSpi;
// Speed argument is ignored for software SPI.
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SD_SCK_MHZ(40), &softSpi)


SdExFat sd;
ExFile file;
ExFile root;
IBusBM IBus;    // IBus object
Servo servoBase;  // create servo object to control a servo
Servo servoExtend;  // create servo object to control a servo
Servo servoElev;  // create servo object to control a servo
Servo servoClaw;  // create servo object to control a servo

// Possible PWM GPIO pins on the ESP32: 0(used by on-board button),2,4,5(used by on-board LED),12-19,21-23,25-27,32-33 
#define servoBasePin    2
#define servoExtendPin  4  
#define servoElevPin    5
#define servoClawPin    12

void setup() {
  Serial.begin(115200);     // debug info
  if (!sd.begin(SD_CONFIG)) {
    Serial.println("OH SHIT SON! SHIT'S FUCKED");
    sd.initErrorHalt(&Serial);
    return;
  } else {
    Serial.println("SD Works Bitches");
  }
  IBus.begin(Serial2,1);    // iBUS object connected to serial2 RX2 pin and use timer 1
  servoBase.attach(servoBasePin); // attaches the servo on pin 18 to the servo object (using timer 0)
  servoExtend.attach(servoExtendPin); // attaches the servo on pin 18 to the servo object (using timer 0)
  servoElev.attach(servoElevPin); // attaches the servo on pin 18 to the servo object (using timer 0)
  servoClaw.attach(servoClawPin); // attaches the servo on pin 18 to the servo object (using timer 0)
  Serial.println("Start IBus2PWM_ESP32");
}

int saveBaseVal, saveExtendVal, saveElevVal, saveClawVal, roboModeVal = 0;

void loop() {
  int servoBaseVal, servoExtendVal, servoElevVal, servoClawVal, roboModeVal, roboMode;
  servoBaseVal = IBus.readChannel(0); // get latest value for servo channel 1
  servoExtendVal = IBus.readChannel(1); // get latest value for servo channel 1
  servoElevVal = IBus.readChannel(2); // get latest value for servo channel 1
  servoClawVal = IBus.readChannel(8); // get latest value for servo channel 1
  roboModeVal = IBus.readChannel(7);  //  Flip switch D to switch between Robo and Arm mode

  if (roboModeVal >= 1500) {
    roboMode = 0;
    } 
  else {
    roboMode = 1;
    }
  
  if (saveExtendVal != servoExtendVal) {
    Serial.println(servoExtendVal); // display new value in microseconds on PC
    saveExtendVal = servoExtendVal;    
    servoExtend.writeMicroseconds(servoExtendVal);   // sets the servo position 
  }
  
  if (saveBaseVal != servoBaseVal) {
    Serial.println(servoBaseVal); // display new value in microseconds on PC
    saveBaseVal = servoBaseVal;    
    servoBase.writeMicroseconds(servoBaseVal);   // sets the servo position 
  }

  if (saveElevVal != servoElevVal) {
    Serial.println(servoElevVal); // display new value in microseconds on PC
    saveElevVal = servoElevVal;    
    servoElev.writeMicroseconds(servoElevVal);   // sets the servo position 
  }

  if (saveClawVal != servoClawVal) {
    Serial.println(servoClawVal); // display new value in microseconds on PC
    saveClawVal = servoClawVal;    
    servoClaw.writeMicroseconds(servoClawVal);   // sets the servo position 
  }
 
  delay(100);
}
