//    32-BIT DAC A2DP_Sink with WS2182FX and bluetooth streaming/control

#include <WiFi.h>
#include <WiFiMulti.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
WiFiMulti wifiMulti;
#include "BluetoothA2DPSink.h"
BluetoothA2DPSink a2dp_sink;
// this header is needed for Bluetooth Serial -> works ONLY on ESP32
#include "BluetoothSerial.h"
#include "credentials.h"

#include <SPI.h>
#define HSPI_MISO   26
#define HSPI_MOSI   27
#define HSPI_SCLK   25
#define HSPI_SS     32
#define VSPI_MISO   MISO
#define VSPI_MOSI   MOSI
#define VSPI_SCLK   SCK
#define VSPI_SS     SS

#if CONFIG_IDF_TARGET_ESP32S2
  #define VSPI FSPI
#endif

static const int spiClk = 1000000; // 1 MHz

//uninitalised pointers to SPI objects
SPIClass * vspi = NULL;
SPIClass * hspi = NULL;

// init Class:
BluetoothSerial ESP_BT;
int incoming;

#include <WS2812FX.h>
#include "WipItsLED.h"

/*******GPS TO BE USED
 * #define GPS_TX 17
 * #define GPS_RX 16
 * 
 * OR USE (AND LIKELY WHAT I SHOULD USE)
 * 
 * const int GPS_TX = 17;
 * const int GPS_RX = 16;
 * Serial1.begin(115200, 8N2, GPS_TX, GPS_RX);    I THINK!!!!
 * 
 * Or use software serial possibly. Whatevs
 */


TaskHandle_t btStream;
TaskHandle_t wifi_Neo;

void setup() {
  xTaskCreatePinnedToCore(
                    btTASK,   /* Task function. */
                    "btStream",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &btStream,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
  delay(500); 

  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                    wifiTASK,   /* Task function. */
                    "wifi_Neo",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &wifi_Neo,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */
    delay(500);
}


  void btTASK( void * pvParameters ){
//  Serial.print("Task1 running on core ");
//  Serial.println(xPortGetCoreID());

  a2dp_sink.set_bits_per_sample(32);
  i2s_pin_config_t my_pin_config = {
    .bck_io_num = 12,   //    BCK
    .ws_io_num = 14,    //    LRCK
    .data_out_num = 4,   //    DIN
    .data_in_num = I2S_PIN_NO_CHANGE
  };
  a2dp_sink.set_pin_config(my_pin_config);
  //  ESP_BT.begin("bt_serial");    //    Initialize Bluetooth and name your Bluetooth interface -> will show up on your phone
  a2dp_sink.start(btDeviceName);
  for(;;){
    delay(1000);
  }
}

void initOTA(){
  WiFi.persistent(false);  // Disable storing WiFi settings in flash
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(espHostName);
  wifiMulti.addAP(ssid1, ssidPassphrase1);
  wifiMulti.addAP(ssid2, ssidPassphrase2);
  if(wifiMulti.run() == WL_CONNECTED) {
    // do a little sumpn sumpn
  }
  ArduinoOTA.setHostname(espHostName);
  ArduinoOTA.setPasswordHash(otaPasswordHash);
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
//      Serial.println("Start updating " + type);
  })
  .onEnd([]() {
//      Serial.println("\nEnd");
  })
  .onProgress([](unsigned int progress, unsigned int total) {
//      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  })
  .onError([](ota_error_t error) {
//  Serial.printf("Error[%u]: ", error);
//  if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
//  else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
//  else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
//  else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
//  else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  ArduinoOTA.begin();

//  Serial.println("Ready");
//  Serial.print("IP address: ");
//  Serial.println(WiFi.localIP());
}

  void wifiTASK( void * pvParameters ){
//  Serial.print("Task1 running on core ");
//  Serial.println(xPortGetCoreID());

  initOTA();
   //initialise two instances of the SPIClass attached to VSPI and HSPI respectively
  vspi = new SPIClass(VSPI);
  hspi = new SPIClass(HSPI);
  
  //clock miso mosi ss
  vspi->begin(VSPI_SCLK, VSPI_MISO, VSPI_MOSI, VSPI_SS); //SCLK, MISO, MOSI, SS
  //alternatively route through GPIO pins
  hspi->begin(HSPI_SCLK, HSPI_MISO, HSPI_MOSI, HSPI_SS); //SCLK, MISO, MOSI, SS

  //set up slave select pins as outputs as the Arduino API
  //doesn't handle automatically pulling SS low
  pinMode(vspi->pinSS(), OUTPUT); //VSPI SS
  pinMode(hspi->pinSS(), OUTPUT); //HSPI SS
  ws2812fx.init();
  ws2812fx.setBrightness(8);
  ws2812fx.setSpeed(1000);
  ws2812fx.setColor(0x007BFF);
  ws2812fx.setMode(FX_MODE_STATIC);
  ws2812fx.start();
  
  for(;;){
  if(wifiMulti.run() != WL_CONNECTED) {
      delay(5000);
    }
    ws2812fx.service();
    ArduinoOTA.handle();
//    btCommandIncoming();

//use the SPI buses
  spiCommand(vspi, 0b01010101); // junk data to illustrate usage
  spiCommand(hspi, 0b11001100);
  delay(100);
  }
}

void spiCommand(SPIClass *spi, byte data) {
  //use it as you would the regular arduino SPI API
  spi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  digitalWrite(spi->pinSS(), LOW); //pull SS slow to prep other end for transfer
  spi->transfer(data);
  digitalWrite(spi->pinSS(), HIGH); //pull ss high to signify end of data transfer
  spi->endTransaction();
}

void loop() {
    //    Move along. Nothing to see here.
  }
