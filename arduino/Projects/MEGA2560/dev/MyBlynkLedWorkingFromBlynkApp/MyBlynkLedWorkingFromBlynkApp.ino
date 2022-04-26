#include <SPIFFS.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <BlynkSimpleEsp32.h>
#include "images.h"
#include <heltec.h>


// Template ID, Device Name and Auth Token are provided by the Blynk.Cloud
// See the Device Info tab, or Template settings
#define BLYNK_TEMPLATE_ID "TMPLmYWEJ4As"
#define BLYNK_DEVICE_NAME "ESP32"
#define BLYNK_AUTH_TOKEN "GMW2HaPUl7iMW-uPZo1cl4QYQwx5DuuR"


#define LedPin 25


// Comment this out to disable prints and save space
#define BLYNK_PRINT Serial













// NTP Servers:
static const char ntpServerName[] = "192.168.1.200";

WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets
time_t getNtpTime();

char auth[] = BLYNK_AUTH_TOKEN;

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "chlabs_bot";
char pass[] = "chlabsrobotseverywhere";

BlynkTimer timer;

WidgetLCD lcd(V1);

// Setting an arbitrary string value to update later. This will become GPS coordinates feed
int incomingStream = 0;
String lastStream = "";

// This function is called every time the Virtual Pin 0 state changes
BLYNK_WRITE(V0)
{
  // Set incoming value from pin V0 to a variable
  int value = param.asInt();

  // Update LED
  digitalWrite(LedPin, value);
}

// This function is called every time the device is connected to the Blynk.Cloud
BLYNK_CONNECTED()
{
  // Change Web Link Button message to "Congratulations!"
  Blynk.setProperty(V3, "offImageUrl", "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations.png");
  Blynk.setProperty(V3, "onImageUrl",  "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations_pressed.png");
  Blynk.setProperty(V3, "url", "https://docs.blynk.io/en/getting-started/what-do-i-need-to-blynk/how-quickstart-device-was-made");
}

// This function runs every second and will update the home screen with any static items


// This function runs every second and should display a sting from the console



void setup()
{

   /* Testing FS.h vs SPIFFS.h
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
 
  if (!file) {
    Serial.println("There was an error opening the file for writing");
    return;
  }
  if (file.print("TEST")) {
    Serial.println("File was written");
  } else {
    Serial.println("File write failed");
  }
 
  file.close();
*/

  int count = 0;
  //Initialize Display
  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Disable*/, true /*Serial Enable*/);
  Heltec.display->flipScreenVertically();
  
  // Debug console
  Serial.begin(115200);

  // Pin 25 is my EPS32's builtin secondary LED
  pinMode(LedPin, OUTPUT);

  Blynk.begin(auth, ssid, pass);
  while(WiFi.status() != WL_CONNECTED && count < 10)
  {
    count ++;
    Heltec.display->clear();
    Heltec.display->drawXbm(34, 14, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
    Heltec.display->display();
    delay(500);

  }
  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);

  // Setup a function to be called every second
  //timer.setInterval(1000L, messageUpdate);
  timer.setInterval(1000L, homeScreen);


/* //SPIFFS TRESTING
  // Initialize SPIFFS
//File file = SPIFFS.open("/test.txt", FILE_WRITE);
SPIFFS.begin();
if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  File file = SPIFFS.open("/test.txt", FILE_WRITE);
 
  if (!file) {
    Serial.println("There was an error opening the file for writing");
    return;
  }
  if (file.print("TEST")) {
    Serial.println("File was written");
  } else {
    Serial.println("File write failed");
  }
 
  file.close();
  
  */

  
}

void loop()
{
  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->drawString(0, 0, ssid);
  Heltec.display->setTextAlignment(TEXT_ALIGN_RIGHT);
  Heltec.display->drawString(128, 0, WiFi.localIP().toString());
  Heltec.display->display();
  delay(10000);

  /* Re enable after testing. These next two lines are CORE!
  Blynk.run();
  timer.run();
  */
}

void homeScreen()
{
  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->drawString(0, 0, ssid);
  Heltec.display->setTextAlignment(TEXT_ALIGN_RIGHT);
  Heltec.display->drawString(128, 0, WiFi.localIP().toString());
  Heltec.display->display();
}

void messageUpdate()
{
  String interfaceData = String(Serial.readString());
  //if (interfaceData != lastStream && interfaceData != "") {
  if (Serial.available() > 0) {
    Heltec.display->clear();
    Blynk.virtualWrite(V2, interfaceData);
    Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
    Heltec.display->setFont(ArialMT_Plain_10);
    Heltec.display->drawString(64, 22, interfaceData);
    Heltec.display->display();
    lastStream = interfaceData; // An effort to compare the last string with the current
  }
}
