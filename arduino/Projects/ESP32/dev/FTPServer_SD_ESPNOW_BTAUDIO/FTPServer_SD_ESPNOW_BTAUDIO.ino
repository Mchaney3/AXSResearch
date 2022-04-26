/*
 * FtpServer esp8266 and esp32 with SD
 *
 * AUTHOR:  Renzo Mischianti
 *
 * https://www.mischianti.org/2020/02/08/ftp-server-on-esp8266-and-esp32
 *
 */

#include <WiFi.h>
#include "SdFat.h"
#include <SimpleFTPServer.h>
#include "credentials.h"
#include "Core0.h"

#define FTP_SERVER_NETWORK_TYPE DEFAULT_FTP_SERVER_NETWORK_TYPE_ESP32
#define DEFAULT_STORAGE_TYPE_ESP32 STORAGE_SDFAT2
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

SdExFat sd;
ExFatFile file;

FtpServer ftpSrv;   //set #define FTP_DEBUG in ESP8266FtpServer.h to see ftp verbose on serial

void _callback(FtpOperation ftpOperation, unsigned int freeSpace, unsigned int totalSpace){
	Serial.print(">>>>>>>>>>>>>>> _callback " );
	Serial.print(ftpOperation);
	/* FTP_CONNECT,
	 * FTP_DISCONNECT,
	 * FTP_FREE_SPACE_CHANGE
	 */
	Serial.print(" ");
	Serial.print(freeSpace);
	Serial.print(" ");
	Serial.println(totalSpace);

	// freeSpace : totalSpace = x : 360

	if (ftpOperation == FTP_CONNECT) Serial.println(F("CONNECTED"));
	if (ftpOperation == FTP_DISCONNECT) Serial.println(F("DISCONNECTED"));
};
void _transferCallback(FtpTransferOperation ftpOperation, const char* name, unsigned int transferredSize){
	Serial.print(">>>>>>>>>>>>>>> _transferCallback " );
	Serial.print(ftpOperation);
	/* FTP_UPLOAD_START = 0,
	 * FTP_UPLOAD = 1,
	 *
	 * FTP_DOWNLOAD_START = 2,
	 * FTP_DOWNLOAD = 3,
	 *
	 * FTP_TRANSFER_STOP = 4,
	 * FTP_DOWNLOAD_STOP = 4,
	 * FTP_UPLOAD_STOP = 4,
	 *
	 * FTP_TRANSFER_ERROR = 5,
	 * FTP_DOWNLOAD_ERROR = 5,
	 * FTP_UPLOAD_ERROR = 5
	 */
	Serial.print(" ");
	Serial.print(name);
	Serial.print(" ");
	Serial.println(transferredSize);
};

void initSD() {
  /////FTP Setup, ensure SPIFFS is started before ftp;  /////////

  /////FTP Setup, ensure SPIFFS is started before ftp;  /////////

  if (sd.begin(SD_CONFIG)) {
      Serial.println("SD opened!");

      ftpSrv.setCallback(_callback);
      ftpSrv.setTransferCallback(_transferCallback);

      ftpSrv.begin("mike","V3n@Sarah");    //username, password for ftp.   (default 21, 50009 for PASV)
  }
  else
  {
    Serial.println("SD Failed");
  }
  if (!sd.exists("LOG")) {
    if (!sd.mkdir("LOG")) {
      Serial.println("Could not create /LOG directory");
    }
  }
  if (!sd.exists("IMAGE")) {
    if (!sd.mkdir("IMAGE")) {
      Serial.println("Failed to create /IMAGE directory");
    }
  }
  if (!sd.exists("MUSIC")) {
    if (!sd.mkdir("MUSIC")) {
      Serial.println("Failed to creat /MUSIC directory");
    }
  }
  if (!file.open("LOG/EventLog.txt", O_WRONLY | O_CREAT)) {
    Serial.println("create LOG/log.txt failed");
  } 
  file.close();
  if (!file.open("LOG/SysLog.txt", O_WRONLY | O_CREAT)) {
    Serial.println("create LOG/log.txt failed");
  }
  file.close();
}

TaskHandle_t bt_stream;
SemaphoreHandle_t xSemaphore = NULL;

void setup(void){
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  initSD();
  
  xSemaphore = xSemaphoreCreateMutex();
  
  xTaskCreatePinnedToCore(
                    btTASK,          /* Task function. */
                    "bt_stream",     /* name of task. */
                    10000,           /* Stack size of task */
                    NULL,            /* parameter of the task */
                    2,               /* Always highest priority to assure audio quality */
                    &bt_stream,      /* Task handle to keep track of created task */
                    0);              /* pin Bluetooth to core o */                  
  delay(500);
}
void loop(void){
  ftpSrv.handleFTP();        //make sure in loop you call handleFTP()!!
 // server.handleClient();   //example if running a webserver you still need to call .handleClient();

}
