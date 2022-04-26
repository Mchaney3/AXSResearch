#include <stdio.h>
#include <string.h>
#include "credentials.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WS2812FX.h>
#include "soc/soc.h" //disable brownout problems
#include "soc/rtc_cntl_reg.h"  //disable brownout problems
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include "SdFat.h"
#include "sdios.h"

#define LED_COUNT 150
#define LED_PIN 4
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

/* Assign a unique ID to this sensor at the same time */
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// The TinyGPSPlus object
TinyGPSPlus gps;

// The serial connection to the GPS device

//Initialize SD card on SdFat SoftSPI
// Chip select may be constant or RAM variable.
const uint8_t SD_CS_PIN = 32;
// Pin numbers in templates must be constants.
const uint8_t SOFT_MISO_PIN = 25;
const uint8_t SOFT_MOSI_PIN = 26;
const uint8_t SOFT_SCK_PIN  = 27;
// SdFat software SPI template
SoftSpiDriver<SOFT_MISO_PIN, SOFT_MOSI_PIN, SOFT_SCK_PIN> softSpi;
// Speed argument is ignored for software SPI.
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SD_SCK_MHZ(50), &softSpi)
// #define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(40), &softSpi)
SdFat sd;
File file;

void displaySensorDetails(void)
{
  sensor_t sensor;
  mag.getSensor(&sensor);
  /*****************
   * 
   *

  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" uT");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" uT");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" uT");  
  Serial.println("------------------------------------");
  Serial.println("");
  delay(500);

  *
  *
  */
}

String songTitle, songArtist, hd;
#include "Core0.h"

TaskHandle_t bt_stream;

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600);
  
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds
    // Clear the buffer
  display.clearDisplay();

  // Draw a single pixel in white
  display.drawPixel(10, 10, SSD1306_WHITE);

  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display.display();
  ws2812fx.init();
  ws2812fx.setBrightness(8);
  ws2812fx.setSpeed(1000);
  ws2812fx.setColor(0x007BFF);
  ws2812fx.setMode(FX_MODE_STATIC);
  ws2812fx.start();
  Serial.setDebugOutput(true);
  Serial.print("ESP32 SDK: ");
  Serial.println(ESP.getSdkVersion());
  Serial.println(F("START " __FILE__ " from " __DATE__));
  int cpuSpeed = getCpuFrequencyMhz();
  Serial.println("CPU running at " + String(cpuSpeed) + "MHz");
  Serial.println("");
  
  Serial.println("HMC5883 Magnetometer Test"); Serial.println("");
  /* Initialise the sensor */
  if(!mag.begin())  {
    /* There was a problem detecting the HMC5883 ... check your connections */
    Serial.println("Ooops, no HMC5883 detected ... Check your wiring!");
    while(1);
  }

  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt();
  }

  //    Write a function to check log files
  
  if (!file.open("TripLog.txt", O_RDWR | O_CREAT)) {
    sd.errorHalt(F("Failed to open TripLog.txt"));
  }

  xTaskCreatePinnedToCore(
                    btTASK,   /* Task function. */
                    "bt_stream",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    0,           /* Always highest priority to assure audio quality */
                    &bt_stream,      /* Task handle to keep track of created task */
                    0);          /* pin Bluetooth to core 0 */                  
  delay(500);
}

void getSensor(){
  /* Get a new sensor event */ 
  sensors_event_t event; 
  mag.getEvent(&event);
 
  /* Display the results (magnetic vector values are in micro-Tesla (uT)) */
  Serial.print("X: "); Serial.print(event.magnetic.x); Serial.print("  ");
  Serial.print("Y: "); Serial.print(event.magnetic.y); Serial.print("  ");
  Serial.print("Z: "); Serial.print(event.magnetic.z); Serial.print("  ");Serial.println("uT");

  // Hold the module so that Z is pointing 'up' and you can measure the heading with x&y
  // Calculate heading when the magnetometer is level, then correct for signs of axis.
  float heading = atan2(event.magnetic.y, event.magnetic.x);
  
  // Once you have your heading, you must then add your 'Declination Angle', which is the 'Error' of the magnetic field in your location.
  // Find yours here: http://www.magnetic-declination.com/
  // Mine is: -13* 2' W, which is ~13 Degrees, or (which we need) 0.22 radians
  // If you cannot find your Declination, comment out these two lines, your compass will be slightly off.
  float declinationAngle = 0.22;
  heading += declinationAngle;
  
  // Correct for when signs are reversed.
  if(heading < 0)
    heading += 2*PI;
    
  // Check for wrap due to addition of declination.
  if(heading > 2*PI)  heading -= 2*PI;
   
  // Convert radians to degrees for readability.
  float headingDegrees = heading * 180/M_PI;
  hd = String(headingDegrees, 2);
  
  //Serial.print("Heading (degrees): "); Serial.println(headingDegrees);
  //displaySensorDetails();
  display.clearDisplay();
  display.setTextWrap(false);
  display.setTextSize(1);             // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Title: ");  display.print(songTitle);
  display.setCursor(0, 9);
  display.print("City: ");
  display.setTextSize(2);
  display.setCursor(102, 19);
  /* if headingDegrees greater than 345 or less then 15 say North
   *  if headingDegree greater then 15 and less than 45 say northeast........
   *  print time
   *  print gpt city location
   *  diplasy temperature
   */
  if (headingDegrees > 344 || (headingDegrees >= 0 && headingDegrees < 16))  {
    display.print("N ");
  }
  if (headingDegrees > 15 && headingDegrees < 75)  {
    display.print("NE");
  }
  if (headingDegrees > 74 && headingDegrees < 106)  {
    display.print("E ");
  }
  if (headingDegrees > 105 && headingDegrees < 165)  {
    display.print("SE");
  }
  if (headingDegrees > 164 && headingDegrees < 196)  {
    display.print("S ");
  }
  if (headingDegrees > 195 && headingDegrees < 256)  {
    display.print("SW");
  }
  if (headingDegrees > 255 && headingDegrees < 286)  {
    display.print("W ");
  }
  if (headingDegrees > 285 && headingDegrees < 346)  {
    display.print("NW");
  }
//  display.print(hd);  display.print(char(247));
  display.display();
  Serial.print("Heading (degrees): "); Serial.println(headingDegrees);

  /****       WRITE INFO TO SD CARD IN v1.2   ****/
}

void displayGPS() {
  if (gps.time.isValid()) {
    display.setTextSize(2);
    display.setCursor(0,19);
    if (gps.time.hour() < 10) Serial.print(F("0"));
    display.print(gps.time.hour());
    display.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    display.print(gps.time.minute());
    display.display();
  }
}

void logGPS() {
  Serial.print(F("Location: ")); 
  if (gps.location.isValid())
  {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F("  Date/Time: "));
  if (gps.date.isValid())
  {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F(" "));
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(F("."));
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
    Serial.print(gps.time.centisecond());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.println();
}

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;        // will store last time LED was updated
// constants won't change:
const long interval = 1000;           // interval at which to blink (milliseconds)

void loop() {
  unsigned long currentMillis = millis();
  ws2812fx.service();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    getSensor();
    displayGPS();
    
    // This sketch displays information every time a new sentence is correctly encoded.
    while (Serial2.available() > 0)  {
      if (gps.encode(Serial2.read()))  {
        logGPS();
      }
    }
  }
}
