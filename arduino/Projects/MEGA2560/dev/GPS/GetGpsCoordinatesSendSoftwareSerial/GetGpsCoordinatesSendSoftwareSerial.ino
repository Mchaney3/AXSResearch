// All GPS parts were created from the SIM7XXX_LTE_DEMO
#include "Adafruit_FONA.h"
#define SIMCOM_7000
// Define Pins For botletics SIM7000/7070 shield
#define FONA_PWRKEY 6
#define FONA_RST 7 // No RST pin for SIM7070
#define FONA_TX 10 // To Microcontroller RX
#define FONA_RX 11 // From Microcontroller TX
//#define FONA_DTR 8 // Connect with solder jumper
//#define FONA_RI 9 // Need to enable via AT commands
//#define T_ALERT 12 // Connect with solder jumper
//Define SoftwareSerial pins
#define ESP32_TX 20
#define ESP32_RX 21
// We default to using software serial. If you want to use hardware serial
// (because softserial isnt supported) comment out the following three lines 
// and uncomment the HardwareSerial line *NOT PART OF THIS SCRIPT
#include <SoftwareSerial.h>
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;
Adafruit_FONA_LTE fona = Adafruit_FONA_LTE();
uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);
uint8_t type;
char imei[16] = {0}; // MUST use a 16 character buffer for IMEI!

// Create interface to ESP32
SoftwareSerial Esp32SS = SoftwareSerial(ESP32_TX, ESP32_RX); // This is a Software Serial between my Arduino and My ESP32 oLED
  
void setup() {
  pinMode(FONA_RST, OUTPUT);
  digitalWrite(FONA_RST, HIGH); // Default state
  
  // Turn on the module by pulsing PWRKEY low for a little bit
  // This amount of time depends on the specific module that's used
  fona.powerOn(FONA_PWRKEY);

  Serial.begin(9600);
  Esp32SS.begin(115200);
  Serial.println(F("CHLABS GPS"));
  Esp32SS.println("CHLABS GPS BOOTING");

  // SIM7000 takes about 3s to turn on and SIM7500 takes about 15s
  // Press Arduino reset button if the module is still turning on and the board doesn't find it.
  // When the module is on it should communicate right after pressing reset

  // Software serial for communication with the GPS AT
  fonaSS.begin(115200); // Default SIM7000 shield baud rate

  Serial.println(F("Configuring to 9600 baud"));
  fonaSS.println("AT+IPR=9600"); // Set baud rate
  delay(100); // Short pause to let the command run
  fonaSS.begin(9600);
  if (! fona.begin(fonaSS)) {
    Serial.println(F("Couldn't find FONA"));
    while (1); // Don't proceed if it couldn't find the device
  }

  //Check that GPS is alive!
  type = fona.type();
  Serial.println(F("FONA is OK"));
  Serial.print(F("Found "));
  Serial.println(type);

  // Print module IMEI number.
  uint8_t imeiLen = fona.getIMEI(imei);
  if (imeiLen > 0) {
    Serial.print("Module IMEI: "); Serial.println(imei);
  }
  
  // Set modem to full functionality
  fona.setFunctionality(1); // AT+CFUN=1
  
  // Configure a GPRS APN, username, and password.
  // You might need to do this to access your network's GPRS/data
  // network.  Contact your provider for the exact APN, username,
  // and password values.  Username and password are optional and
  // can be removed, but APN is required.
  //fona.setNetworkSettings(F("your APN"), F("your username"), F("your password"));
  //fona.setNetworkSettings(F("m2m.com.attz")); // For AT&T IoT SIM card
  //fona.setNetworkSettings(F("telstra.internet")); // For Telstra (Australia) SIM card - CAT-M1 (Band 28)
  fona.setNetworkSettings(F("hologram")); // For Hologram SIM card
}

void getOnboardTemp() {
 char tempBuff[16];
 float temperature = analogRead(A0); // Should give farenheit
 dtostrf(temperature, 1, 2, tempBuff); // float_val, min_width, digits_after_decimal, char_buffer
 Serial.print("Temp: ");
 Serial.println(tempBuff);
}

void getGPSBatteryLevel() {
uint16_t battLevel;
  if (! fona.getBattVoltage(&battLevel)) battLevel = 3800; // Use dummy voltage if can't read
}

void getCurrentCoordinates() {

float latitude, longitude, speed_kph, heading, altitude;
if (fona.getGPS(&latitude, &longitude, &speed_kph, &heading, &altitude)) { // Use this line instead if you don't want UTC time
  Serial.println(F("---------------------"));
  Serial.print(F("Latitude: ")); Serial.print(latitude, 6); Serial.print(F("\tLongitude: ")); Serial.println(longitude, 6);
  Serial.print(F("Speed: ")); Serial.print(speed_kph); Serial.print(F("\tHeading: ")); Serial.println(heading);
  Serial.print(F("Altitude: ")); Serial.println(altitude);
// Comment out the stuff below if you don't care about UTC time
  /*
  Serial.print(F("Year: ")); Serial.println(year(t));
  Serial.print(F("Month: ")); Serial.println(month(t));
  Serial.print(F("Day: ")); Serial.println(day(t));
  Serial.print(F("Hour: ")); Serial.println(hour(t));
  Serial.print(F("Minute: ")); Serial.println(minute(t));
  Serial.print(F("Second: ")); Serial.println(second(t));
  */
  Serial.println(F("---------------------"));
  }
}


//Still need to finalize formatting. Sending IMEI for now to complete testing
void sendCoordinatesToEsp32() {
if(Esp32SS.available()) {
  float latitude, longitude, speed_mph, heading, altitude;
    if (fona.getGPS(&latitude, &longitude, &speed_mph, &heading, &altitude)) { // Use this line instead if you don't want UTC time
    Esp32SS.print("Coordinates: "); Esp32SS.print(latitude); Esp32SS.print(","); Esp32SS.println(longitude);
    Serial.println("Message Sent To ESP But I need To Build An Ack Process Here");
    Serial.print(F("Cooridinates: ")); Serial.print(latitude, 6); Serial.print(F(",")); Serial.println(longitude, 6);
    Serial.println("There's no , 6 in the code after this line");
    Serial.print("Coordinates: "); Serial.print(latitude); Serial.print(","); Serial.println(longitude);
    Serial.print(F("Speed: ")); Serial.print(speed_mph); Serial.print(F("\tHeading: ")); Serial.println(heading);
    Serial.print(F("Altitude: ")); Serial.println(altitude);
    }
  }
}



void loop() {
 // getOnboardTemp();
  //getGPSBatteryLevel();
  Serial.println(imei);
  Esp32SS.println(imei);
  // sendCoordinatesToEsp32();
  //getCurrentCoordinates();
  delay(1000);
}
