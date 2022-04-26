#include <HardwareSerial.h>
#include "Adafruit_FONA.h" // https://github.com/botletics/SIM7000-LTE-Shield/tree/master/Code
Adafruit_FONA_LTE fona = Adafruit_FONA_LTE();
uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);
uint8_t type;
char replybuffer[255]; // this is a large buffer for replies
char imei[16] = {0}; // MUST use a 16 character buffer for IMEI!

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial2.begin(115200);

  Serial.println(F("Configuring to 9600 baud"));
  Serial2.println("AT+IPR=9600"); // Set baud rate
  delay(100); // Short pause to let the command run
  Serial2.begin(9600);
  if (! fona.begin(Serial2)) {
    Serial.println(F("Couldn't find FONA"));
    while (1); // Don't proceed if it couldn't find the device
  }

  type = fona.type();
  Serial.println(F("FONA is OK"));

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

  if (!fona.enableGPS(true)){
          Serial.println(F("Failed to turn on GPS"));
  }

}

void getGPSCoordinates() {
if(Serial2.available()) {
  float latitude, longitude, speed_mph, heading, altitude;
    if (fona.getGPS(&latitude, &longitude, &speed_mph, &heading, &altitude)) { // Use this line instead if you don't want UTC time
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
  // put your main code here, to run repeatedly:

}
