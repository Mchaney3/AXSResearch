/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-cam-take-photo-display-web-server/
  
  IMPORTANT!!! 
   - Select Board "AI Thinker ESP32-CAM"
   - GPIO 0 must be connected to GND to upload a sketch
   - After connecting GPIO 0 to GND, press the ESP32-CAM on-board RESET button to put your board in flashing mode
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

#include "WiFi.h"
//#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
#include <ESPAsyncWebServer.h>
#include <StringArray.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
//#include "SdFat.h"
#include "sdios.h"
#include "FS.h"
#include <ArduCAM.h>
#include "memorysaver.h"
//#include "arducam_esp32s_camera.h"

File file;
// Create a Serial output stream.
ArduinoOutStream cout(Serial);
//const int CAM_POWER_ON = D10;
// set GPIO16 as the slave select :
const int CS = 10;
const int SD_CS = 34;
//Version 2,set GPIO0 as the slave select :
char pname[20];
//static int  index=0;
byte buf[256];
static int i = 0;
static int k = 0;
uint8_t temp = 0, temp_last = 0;
uint32_t length = 0;
bool is_header = false;
ArduCAM myCAM(OV2640, CS);

char path[30];  //array for file path

// Replace with your network credentials
const char* ssid = "chlabs_bot";
const char* password = "chlabsrobotseverywhere";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

boolean takeNewPhoto = false;

// Photo File Name to save in SPIFFS
#define FILE_PHOTO "/photo.jpg"



const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { text-align:center; }
    .vert { margin-bottom: 10%; }
    .hori{ margin-bottom: 0%; }
  </style>
</head>
<body>
  <div id="container">
    <h2>ESP32-CAM Last Photo</h2>
    <p>It might take more than 5 seconds to capture a photo.</p>
    <p>
      <button onclick="rotatePhoto();">ROTATE</button>
      <button onclick="capturePhoto()">CAPTURE PHOTO</button>
      <button onclick="location.reload();">REFRESH PAGE</button>
    </p>
  </div>
  <div><img src="saved-photo" id="photo" width="70%"></div>
</body>
<script>
  var deg = 0;
  function capturePhoto() {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', "/capture", true);
    xhr.send();
  }
  function rotatePhoto() {
    var img = document.getElementById("photo");
    deg += 90;
    if(isOdd(deg/90)){ document.getElementById("container").className = "vert"; }
    else{ document.getElementById("container").className = "hori"; }
    img.style.transform = "rotate(" + deg + "deg)";
  }
  function isOdd(n) { return Math.abs(n % 2) == 1; }
</script>
</html>)rawliteral";


void initArduCAM() {
  uint8_t vid, pid;
  uint8_t temp;
//  static int i = 0;
  //set the CS as an output:
  pinMode(CS,OUTPUT);
//  pinMode(CAM_POWER_ON , OUTPUT);
//  digitalWrite(CAM_POWER_ON, HIGH);
   Wire.begin();
  Serial.begin(115200);
  Serial.println(F("ArduCAM Starting!"));
  //initialize SPI:
  SPI.begin();
  if(!SD.begin(SD_CS)){
    //    WHY THE F NOT!
    Serial.println("SD Card Mount Failed");
    Serial.println();
    Serial.println(F("Code,Symbol - failed operation"));
    for (uint8_t code = 0; code <= SD_CARD_ERROR_UNKNOWN; code++) {
      Serial.print(code < 16 ? "0X0" : "0X");
      Serial.print(code, HEX);
      Serial.print(",");
      printSdErrorSymbol(&Serial, code);
      Serial.print(" - ");
      printSdErrorText(&Serial, code);
      Serial.println();
    }
    return;
   }else
    Serial.println(F("SD Card detected!"));
  while(1){
  //Check if the ArduCAM SPI bus is OK
  myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM.read_reg(ARDUCHIP_TEST1);
  if(temp != 0x55){
    Serial.println(F("SPI interface Error!"));
    delay(2);
    continue;
  }
  else
   break;
  } 
    uint8_t cardType = SD.cardType();
    if(cardType == CARD_NONE){
        Serial.println(F("No SD card attached"));
        return;
    }
    Serial.print(F("SD Card Type: "));
    if(cardType == CARD_MMC){
        Serial.println(F("MMC"));
    } else if(cardType == CARD_SD){
        Serial.println(F("SDSC"));
    } else if(cardType == CARD_SDHC){
        Serial.println(F("SDHC"));
    } else {
        Serial.println(F("UNKNOWN"));
    }
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);  
   
#if defined (OV2640_MINI_2MP_PLUS) || defined (OV2640_CAM)
  //Check if the camera module type is OV2640
  myCAM.wrSensorReg8_8(0xff, 0x01);
  myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
  if ((vid != 0x26 ) && (( pid != 0x41 ) || ( pid != 0x42 )))
   Serial.println(F("Can't find OV2640 module!"));
  else
   Serial.println(F("OV2640 detected."));
  #elif defined (OV5640_MINI_5MP_PLUS) || defined (OV5640_CAM)
   //Check if the camera module type is OV5640
  myCAM.wrSensorReg16_8(0xff, 0x01);
  myCAM.rdSensorReg16_8(OV5640_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg16_8(OV5640_CHIPID_LOW, &pid);
   if((vid != 0x56) || (pid != 0x40))
   Serial.println(F("Can't find OV5640 module!"));
   else
   Serial.println(F("OV5640 detected."));
#elif defined (OV5642_MINI_5MP_PLUS) || defined (OV5642_MINI_5MP) || defined (OV5642_MINI_5MP_BIT_ROTATION_FIXED) ||(defined (OV5642_CAM))
 //Check if the camera module type is OV5642
  myCAM.wrSensorReg16_8(0xff, 0x01);
  myCAM.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
   if((vid != 0x56) || (pid != 0x42)){
   Serial.println(F("Can't find OV5642 module!"));
   }
   else
   Serial.println(F("OV5642 detected."));
  #endif
    //Change to JPEG capture mode and initialize the OV2640 module
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  myCAM.OV2640_set_JPEG_size(OV2640_1600x1200);
    // Print ESP32 Local IP Address
  Serial.print("IP Address: http://");
  Serial.println(WiFi.localIP());
   delay(1000);
}

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }


  // Turn-off the 'brownout detector'
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  initArduCAM();
  

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", index_html);
  });

  server.on("/capture", HTTP_GET, [](AsyncWebServerRequest * request) {
    takeNewPhoto = true;
    request->send_P(200, "text/plain", "Taking Photo");
  });

  server.on("/saved-photo", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SD, FILE_PHOTO, "image/jpg", true);
  });

  // Start server
  server.begin();

}

// Check if photo capture was successful
bool checkPhoto( fs::FS &fs ) {
  File f_pic = SD.open(FILE_PHOTO, FILE_WRITE);
  unsigned int pic_sz = f_pic.size();
  return ( pic_sz > 100 );
}

// Capture Photo and Save it to SPIFFS
void capturePhotoSaveSD(void) {
  File file ; 
  //Flush the FIFO
  myCAM.flush_fifo();
  //Clear the capture done flag
  myCAM.clear_fifo_flag();
  //Start capture
  myCAM.start_capture();
  Serial.println(F("Star Capture"));
  while(!myCAM.get_bit(ARDUCHIP_TRIG , CAP_DONE_MASK));
  Serial.println(F("Capture Done."));  
  length = myCAM.read_fifo_length();
  Serial.print(F("The fifo length is :"));
  Serial.println(length, DEC);
  if (length >= MAX_FIFO_SIZE){
    Serial.println(F("Over size."));
  }
  if (length == 0 ){
    Serial.println(F("Size is 0."));
  }
  file = SD.open(FILE_PHOTO, FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file for writing");
    return;
  }
  i = 0;
  myCAM.CS_LOW();
  myCAM.set_fifo_burst(); 
  while ( length-- ){
    temp_last = temp;
    temp =  SPI.transfer(0x00);
    //Read JPEG data from FIFO
    if ( (temp == 0xD9) && (temp_last == 0xFF) ){
      buf[i++] = temp;  //save the last  0XD9     
     //Write the remain bytes in the buffer
      myCAM.CS_HIGH();
      file.write(buf, i);    
    //Close the file
      file.close();
      Serial.println(F("Image save OK."));
      is_header = false;
      i = 0;
    }  
    if (is_header == true){ 
      //Write image data to buffer if not full
      if (i < 256)
      buf[i++] = temp;
      else
      {
        //Write 256 bytes image data to file
        myCAM.CS_HIGH();
        file.write(buf, 256);
        i = 0;
        buf[i++] = temp;
        myCAM.CS_LOW();
        myCAM.set_fifo_burst();
        file.close();
      }        
    }
    else if ((temp == 0xD8) & (temp_last == 0xFF))
    {
      is_header = true;
      buf[i++] = temp_last;
      buf[i++] = temp;   
    } 
  } 
}

void loop() {
  if (takeNewPhoto) {
    capturePhotoSaveSD();
    takeNewPhoto = false;
  }
  delay(1);
}
