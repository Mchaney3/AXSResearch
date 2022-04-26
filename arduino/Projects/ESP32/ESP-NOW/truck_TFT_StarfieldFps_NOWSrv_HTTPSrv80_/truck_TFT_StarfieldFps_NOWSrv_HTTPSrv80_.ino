// Animates white pixels to simulate flying through a star field

#include <WifiEspNowBroadcast.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <ESPmDNS.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "ESPAsyncWebServer.h"
#include <ArduinoJson.h>
#include "credentials.h"
#include "dashboard.h"
#include "Core0.h"

// Structure example to receive data
// Must match the sender structure
typedef struct struct_GPSmessage {
  float Lat;
  float Lng;
  float He;
  unsigned int readingId;
} struct_GPSmessage;
struct_GPSmessage incomingGPS;   //    Adjust struct to match a new struct in the sender

static const int BUTTON_PIN = 13;

Dashboard* dashboard;
AsyncWebServer server(80);


/******************************************     END WEB SERVER WEB PAGE   *********************************
*                                                   Begin ESP NOW Setup                                  */

void processRx(const uint8_t mac[WIFIESPNOW_ALEN], const uint8_t* buf, size_t count, void* arg){
  Serial.printf("Message from %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3],
                mac[4], mac[5]);
  for (size_t i = 0; i < count; ++i) {
    char message = static_cast<char>(buf[i]);
    Serial.print(message);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    // Calcualte frames per second
    //tft.setTextDatum(MC_DATUM);
    
    tft.setTextPadding(80);
    tft.setFreeFont(CF_OL24);
    
    tft.drawString(String(message), 5, 180, GFXFF);// Print the test text in the custom font
    // Reset text padding to zero (default)
    tft.setTextPadding(0);
  }
  Serial.println();
}

/**********************************************    ESP NOW Configiured. Start Setup   *************/
TaskHandle_t tftUpdate;
void setup() {

  xTaskCreatePinnedToCore(
                    tftScreen,   /* Task function. */
                    "tftUpdate",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    2,           /* priority of the task */
                    &tftUpdate,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 1 */                  
  delay(500);
  
  Serial.begin(115200);

/***************************************************************    Init WIFI         *************/
  WiFi.persistent(false);
  WiFi.setSleep(false);
  WiFi.mode(WIFI_AP_STA);
  // Set device as a Wi-Fi Station  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Setting as a Wi-Fi Station..");
  }
  Serial.print("Station IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Wi-Fi Channel: ");
  Serial.println(WiFi.channel());

/***************************************************************    Init ESP Now      *************/

  bool ok = WifiEspNowBroadcast.begin("ESPNOW", 3);
  if (!ok) {
    Serial.println("WifiEspNowBroadcast.begin() failed");
    ESP.restart();
  }
  WifiEspNowBroadcast.onReceive(processRx, nullptr);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.print("MAC address of this node is ");
  Serial.println(WiFi.softAPmacAddress());
  Serial.println("Press the button to send a message");
  
/***************************************************************    Init Web Server   *************/

if (MDNS.begin("dashboard")) {
    // Add service to MDNS-SD
    MDNS.addService("http", "tcp", 80);
    Serial.println("mDNS responder started");
  } else {
    Serial.println("Error setting up MDNS responder!");
  }

  Serial.println("Starting server...");
  dashboard = new Dashboard(&server);

  // Note: explicit type is only needed on ESP8266
  dashboard->Add<uint32_t>("Uptime", millis, 5000);
  server.begin();
  Serial.println("Started server.");
}

void loop(){
/**************************Web Server   ******************/
#ifdef ESP32
  // Note: this works around
  // https://github.com/espressif/arduino-esp32/issues/3886
  if (!MDNS.begin("dashboard")) {
    Serial.println("Error refreshing MDNS responder!");
  }
#else
  MDNS.update();
#endif
  
/************   ESP-NPW   *************/
  if (digitalRead(BUTTON_PIN) == LOW) { // button is pressed
    //sendMessage();
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawString("Button", 0, 0, GFXFF);
    while (digitalRead(BUTTON_PIN) == LOW) // wait for button release
      ;
  }
  WifiEspNowBroadcast.loop();
}
