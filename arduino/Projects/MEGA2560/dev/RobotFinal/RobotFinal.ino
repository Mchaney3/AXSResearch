#include <IBusBM.h>
#include <Servo.h>
#include <SPI.h>
//#include <SD.h>
#include "SdFat.h"
#include "sdios.h"
#include "BufferedPrint.h"
#include <Adafruit_GPS.h>
#include <LCDWIKI_GUI.h> //Core graphics library
#include <LCDWIKI_KBV.h> //Hardware-specific library
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>

#define GPSSerial Serial1
#define SPI_CLOCK SD_SCK_MHZ(50)
#define GPSECHO false
//define some colour values
#define  BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

#define SD_CONFIG SdioConfig(FIFO_SDIO)
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SPI_CLOCK)
/*
#elif  ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SPI_CLOCK)
#else  // HAS_SDIO_CLASS
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SPI_CLOCK)
*/

#define ESP32Serial Serial3

uint32_t timer = millis();
const uint8_t SD_CS_PIN = SS;
//const int chipSelect = 53;

IBusBM IBus; // IBus object
SdFat sd;
File gpslogFile;
Adafruit_GPS GPS(&GPSSerial);
LCDWIKI_KBV mylcd(ILI9486,40,38,39,44,41); //model,cs,cd,wr,rd,reset
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);
sensors_event_t event; 
Servo espeye;  // create servo object to control a servo

void displaySensorDetails(void)
{
  sensor_t sensor;
  mag.getSensor(&sensor);
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
}

void setup() {

  mylcd.Init_LCD();
  mylcd.Fill_Screen(BLACK);
  // make this baud rate fast enough to we aren't waiting on it
  Serial.begin(115200);
  // 9600 baud is the default rate for the Ultimate GPS
  GPS.begin(9600);
  ESP32Serial.begin(115200);
  Serial.println("HMC5883 Magnetometer Test"); Serial.println("");
  
  /* Initialise the sensor */
  if(!mag.begin())
  {
    /* There was a problem detecting the HMC5883 ... check your connections */
    Serial.println("Ooops, no HMC5883 detected ... Check your wiring!");
    while(1);
  } else {
      displaySensorDetails();
      delay(500);
  }
  
  /* Display some basic information on this sensor */
  Serial.println("Starting IBus2PWM");
  IBus.begin(Serial2);    // iBUS connected to Serial0 - change to Serial1 or Serial2 port when required
//  ESP32Serial.begin(115200);    //    To be used later
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // uncomment this line to turn on only the "minimum recommended" data
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  // For parsing data, we don't suggest using anything but either RMC only or RMC+GGA since
  // the parser doesn't care about other sentences at this time
  // Set the update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); // 1 Hz update rate
  // For the parsing code to work nicely and have time to sort thru the data, and
  // print it out we don't suggest using anything higher than 1 Hz

  // Request updates on antenna status, comment out to keep quiet
  GPS.sendCommand(PGCMD_ANTENNA);

  delay(1000);

  // Ask for firmware version
  GPSSerial.println(PMTK_Q_RELEASE);

//  myservo.attach(9);  // attaches the servo on pin 9 to the servo object

  Serial.println("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(53, OUTPUT);
  
  // see if the card is present and can be initialized:
  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt(&Serial);
  }
  delay(500);
  Serial.println("SD card initialized. Checking log directory.");  
  if (!sd.exists("LOG")) {
    if (!sd.mkdir("LOG")) {
      Serial.println("Create log directory failed");
    } else {
      Serial.println("Log directory found. Checking files.");
    }
  }
  delay(500);
  if (!sd.chdir("LOG")) {
    if (!gpslogFile.open("gpslogFile.txt", O_RDONLY)) {
      if (!gpslogFile.open("gpslogFile.txt", O_WRONLY | O_CREAT)) {
        Serial.println("GPS log file not present and could not be created.");
      } else {
        String gpslogHeader = "Log file generated on 12/12/00 at 10:15 p.m. (THIS IS TEST DATA, NEED TIMESTAMPS";
        gpslogFile.println(gpslogHeader);
        gpslogFile.close();
        Serial.println("Log file created successfully");
      }
    } else {
      gpslogFile.close();
      Serial.println("GPS log file found");
    }
  }
  mylcd.Fill_Screen(0x0000);
  mylcd.Set_Rotation(3);
  mylcd.Set_Text_Mode(1);
  mylcd.Set_Text_colour(GREEN);

}

void MaGnEtO() {
  /* Get a new sensor event */ 
  mag.getEvent(&event);
 
  /* Display the results (magnetic vector values are in micro-Tesla (uT)) */
  Serial.print("\r\nX: "); Serial.print(event.magnetic.x); Serial.print("  ");
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
  if(heading > 2*PI)
    heading -= 2*PI;
   
  // Convert radians to degrees for readability.
  float headingDegrees = heading * 180/M_PI; 
  
  Serial.print("Heading (degrees): "); Serial.println(headingDegrees);
  
  delay(500);
}

void loop() {
  // read data from the GPS in the 'main loop'
  char c = GPS.read();
  // if you want to debug, this is a good time to do it!
  if (GPSECHO)
    if (c) Serial.print(c);
  // if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived()) {
    // a tricky thing here is if we print the NMEA sentence, or data
    // we end up not listening and catching other sentences!
    // so be very wary if using OUTPUT_ALLDATA and trying to print out data
    Serial.print(GPS.lastNMEA()); // this also sets the newNMEAreceived() flag to false
    if (!GPS.parse(GPS.lastNMEA())) // this also sets the newNMEAreceived() flag to false
      return; // we can fail to parse a sentence in which case we should just wait for another
  }

  // approximately every 2 seconds or so, print out the current stats
  if (millis() - timer > 2000) {
    timer = millis(); // reset the timer
    mylcd.Fill_Screen(BLACK);
    MaGnEtO();
    Serial.print("\nTime: ");
    if (GPS.hour < 10) { Serial.print('0'); }
    Serial.print(GPS.hour, DEC); Serial.print(':');
    if (GPS.minute < 10) { Serial.print('0'); }
    Serial.print(GPS.minute, DEC); Serial.print(':');
    if (GPS.seconds < 10) { Serial.print('0'); }
    Serial.print(GPS.seconds, DEC); Serial.print('.');
    if (GPS.milliseconds < 10) {
      Serial.print("00");
    } else if (GPS.milliseconds > 9 && GPS.milliseconds < 100) {
      Serial.print("0");
    }
    Serial.println(GPS.milliseconds);
    Serial.print("Date: ");
    Serial.print(GPS.day, DEC); Serial.print('/');
    Serial.print(GPS.month, DEC); Serial.print("/20");
    Serial.println(GPS.year, DEC);
    Serial.print("Fix: "); Serial.print((int)GPS.fix);
    Serial.print(" quality: "); Serial.println((int)GPS.fixquality);
    if (GPS.fix) {
      Serial.print("Location: ");
      Serial.print(GPS.latitude, 4); Serial.print(GPS.lat);
      Serial.print(", ");
      Serial.print(GPS.longitude, 4); Serial.println(GPS.lon);
      Serial.print("Speed (MPH): "); Serial.println(GPS.speed * 1.15078);
      Serial.print("Angle: "); Serial.println(GPS.angle);
      Serial.print("Altitude: "); Serial.println(GPS.altitude);
      Serial.print("Satellites: "); Serial.println((int)GPS.satellites);
      if (gpslogFile.open("gpslogFile.txt", O_WRONLY)){ 
        gpslogFile.print("Location: ");
        gpslogFile.print(GPS.latitude, 4); gpslogFile.print(GPS.lat);
        gpslogFile.print(", ");
        gpslogFile.print(GPS.longitude, 4); gpslogFile.print(GPS.lon);
        gpslogFile.print("\r\n");
        gpslogFile.print("Speed (MPH): "); gpslogFile.print(GPS.speed * 1.15078);
        gpslogFile.print("\r\n");
        gpslogFile.print("Angle: "); gpslogFile.print(GPS.angle);
        gpslogFile.print("\r\n");
        gpslogFile.print("Altitude: "); gpslogFile.print(GPS.altitude);
        gpslogFile.print("\r\n");
        gpslogFile.print("Satellites: "); gpslogFile.print((int)GPS.satellites);
        gpslogFile.println("\r\n");
        gpslogFile.close();
        Serial.println("GPS log file updated\r\n");
        updategpsDisplay();
      } else {
        Serial.println("Couldn't update log file\r\n");
      }
    }
  }
}

void updategpsDisplay() {
  //    Start building my gps display
  mylcd.Set_Text_Size(4);
  int gpshour = GPS.hour;
  mylcd.Print_Number_Int(gpshour, 0, 0, 0, ' ',16);

      
      /*
mylcd.Print_Number_Dec(GPS.hour, DEC); mylcd.Print_String(':');
 mylcd.Print_Number_Dec(GPS.minute, DEC); Serial.print(':');
 mylcd.Print_Number_Dec(GPS.seconds, DEC); Serial.print('.');

    Serial.print("Date: ");
    Serial.print(GPS.day, DEC); Serial.print('/');
    Serial.print(GPS.month, DEC); Serial.print("/20");
    Serial.println(GPS.year, DEC);
    Serial.print("Fix: "); Serial.print((int)GPS.fix);
    Serial.print(" quality: "); Serial.println((int)GPS.fixquality);
  
  mylcd.Print_String("Location: ", 0, 0);
  mylcd.Print_String(GPS.latitude, 4); gpslogFile.print(GPS.lat);
  gpslogFile.print(", ");
  gpslogFile.print(GPS.longitude, 4); gpslogFile.print(GPS.lon);
  gpslogFile.print("\r\n");
  gpslogFile.print("Speed (MPH): "); gpslogFile.print(GPS.speed * 1.15078);
  gpslogFile.print("\r\n");
  gpslogFile.print("Angle: "); gpslogFile.print(GPS.angle);
  gpslogFile.print("\r\n");
  gpslogFile.print("Altitude: "); gpslogFile.print(GPS.altitude);
  gpslogFile.print("\r\n");
  gpslogFile.print("Satellites: "); gpslogFile.print((int)GPS.satellites);
  gpslogFile.println("\r\n");
  gpslogFile.close();
  Serial.println("GPS log file updated\r\n");
  
  //display 2 times string
        
        mylcd.Print_String("Hello World!",40, 0);
        mylcd.Print_Number_Float(01234.56789, 2, 0, 56, '.', 0, ' ');  
        mylcd.Print_Number_Int(0xDEADBEF, 0, 72, 0, ' ',16);
        //mylcd.Print_String("DEADBEEF", 0, 72);
        */
}
