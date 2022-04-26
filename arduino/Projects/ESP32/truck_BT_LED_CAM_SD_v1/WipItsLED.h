#include "BluetoothA2DPSink.h"
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WS2812FX.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Wire.h>
#include "SdFat.h"
#include "sdios.h"
#include <SPI.h>
#include <ArduCAM.h>
#include "memorysaver.h"

const int8_t DISABLE_CS_PIN = -1;
#define SD_FAT_TYPE 2
//Initialize SD card on SdFat SoftSPI
// Chip select may be constant or RAM variable.
const uint8_t SD_CS_PIN = 33;
// Pin numbers in templates must be constants.
const uint8_t SOFT_MISO_PIN = 26;
const uint8_t SOFT_MOSI_PIN = 25;
const uint8_t SOFT_SCK_PIN  = 27;
// SdFat software SPI template
SoftSpiDriver<SOFT_MISO_PIN, SOFT_MOSI_PIN, SOFT_SCK_PIN> softSpi;
// Speed argument is ignored for software SPI.
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SD_SCK_MHZ(50), &softSpi)
SdExFat sd;
ExFile file;

#define LED_COUNT 300
#define LED_PIN 2
#define TIMER_MS 5000
#define WiFiTimer_MS 10000

WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
WiFiMulti wifiMulti;
BluetoothA2DPSink a2dp_sink;
#define pic_num 200
#define rate 0x05
//set pin 17 as the slave select for SPI:
const int CS = 32;
static int k = 0; 
#define AVIOFFSET 240
unsigned long movi_size = 0;
unsigned long jpeg_size = 0;
char pname[20];
byte  zero_buf[4] = {0x00, 0x00, 0x00, 0x00};
byte oodc[4] = {0x30, 0x30, 0x64, 0x63};
byte  avi1[4] = {0x41, 0x56, 0x49, 0x31};
byte  avi_header[AVIOFFSET] PROGMEM ={
  0x52, 0x49, 0x46, 0x46, 0xD8, 0x01, 0x0E, 0x00, 0x41, 0x56, 0x49, 0x20, 0x4C, 0x49, 0x53, 0x54,
  0xD0, 0x00, 0x00, 0x00, 0x68, 0x64, 0x72, 0x6C, 0x61, 0x76, 0x69, 0x68, 0x38, 0x00, 0x00, 0x00,
  0xA0, 0x86, 0x01, 0x00, 0x80, 0x66, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
  0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x40, 0x01, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4C, 0x49, 0x53, 0x54, 0x84, 0x00, 0x00, 0x00,
  0x73, 0x74, 0x72, 0x6C, 0x73, 0x74, 0x72, 0x68, 0x30, 0x00, 0x00, 0x00, 0x76, 0x69, 0x64, 0x73,
  0x4D, 0x4A, 0x50, 0x47, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x01, 0x00, 0x00, 0x00, rate, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x73, 0x74, 0x72, 0x66,
  0x28, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00,
  0x01, 0x00, 0x18, 0x00, 0x4D, 0x4A, 0x50, 0x47, 0x00, 0x84, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4C, 0x49, 0x53, 0x54,
  0x10, 0x00, 0x00, 0x00, 0x6F, 0x64, 0x6D, 0x6C, 0x64, 0x6D, 0x6C, 0x68, 0x04, 0x00, 0x00, 0x00,
  0x64, 0x00, 0x00, 0x00, 0x4C, 0x49, 0x53, 0x54, 0x00, 0x01, 0x0E, 0x00, 0x6D, 0x6F, 0x76, 0x69,
};
ArduCAM myCAM(OV2640, CS);

void print_quartet(unsigned long i,ExFile fd){
  fd.write(i % 0x100);  i = i >> 8;   //i /= 0x100;
  fd.write(i % 0x100);  i = i >> 8;   //i /= 0x100;
  fd.write(i % 0x100);  i = i >> 8;   //i /= 0x100;
  fd.write(i % 0x100);
}

void Video2SD(const char * path){
  //File file ;
  byte buf[256];
  static int i = 0; 
  uint8_t temp = 0, temp_last = 0;
  unsigned long position = 0;
  uint16_t frame_cnt = 0;
  uint8_t remnant = 0;
  uint32_t length = 0;
  bool is_header = false;
  //Create a avi file

   file = sd.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
  for ( i = 0; i < AVIOFFSET; i++)
  {
    char ch = pgm_read_byte(&avi_header[i]);
    buf[i] = ch;
  }
  file.write(buf, AVIOFFSET);
  //Write video data
   Serial.println(F("Recording video, please wait..."));
  for ( frame_cnt = 0; frame_cnt < pic_num; frame_cnt ++)
  {
    #if defined (ESP32)
    yield();
    #endif
    temp_last = 0;temp = 0;
    //Capture a frame            
    //Flush the FIFO
    myCAM.flush_fifo();
    //Clear the capture done flag
    myCAM.clear_fifo_flag();
    //Start capture
    myCAM.start_capture();
    while (!myCAM.get_bit(ARDUCHIP_TRIG , CAP_DONE_MASK));
    length = myCAM.read_fifo_length();
    file.write(oodc,4);
    file.write(zero_buf, 4);
    i = 0;
    jpeg_size = 0;
    myCAM.CS_LOW();
    myCAM.set_fifo_burst();
  while ( length-- )
  {
    #if defined (ESP32)
      yield();
      #endif
    temp_last = temp;
    temp =  SPI.transfer(0x00);
    //Read JPEG data from FIFO
    if ( (temp == 0xD9) && (temp_last == 0xFF) ) //If find the end ,break while,
    {
        buf[i++] = temp;  //save the last  0XD9     
       //Write the remain bytes in the buffer
        myCAM.CS_HIGH();
        file.write(buf, i);    
        is_header = false;
        jpeg_size += i;
        i = 0;
    }  
    if (is_header == true)
    { 
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
           jpeg_size += 256;
        }        
    }
    else if ((temp == 0xD8) & (temp_last == 0xFF))
    {
      is_header = true;
      buf[i++] = temp_last;
      buf[i++] = temp;   
    } 
  }
    remnant = (4 - (jpeg_size & 0x00000003)) & 0x00000003;
    jpeg_size = jpeg_size + remnant;
    movi_size = movi_size + jpeg_size;
    if (remnant > 0)
    file.write(zero_buf, remnant);
    position = file.position();
    file.seek(position - 4 - jpeg_size);
    print_quartet(jpeg_size, file);
    position = file.position();
    file.seek(position + 6);
    file.write(avi1,4);
    position = file.position();
    file.seek(position + jpeg_size - 10);
  }

  //Modify the MJPEG header from the beginning of the file

  file.seek(4);
  print_quartet(movi_size +12*frame_cnt+4, file);//riff file size
  //overwrite hdrl
  unsigned long us_per_frame = 1000000 / rate; //(per_usec); //hdrl.avih.us_per_frame
  file.seek(0x20);
  print_quartet(us_per_frame, file);
  unsigned long max_bytes_per_sec = movi_size * rate/ frame_cnt; //hdrl.avih.max_bytes_per_sec
  file.seek(0x24);
  print_quartet(max_bytes_per_sec, file);
  unsigned long tot_frames = frame_cnt;    //hdrl.avih.tot_frames
  file.seek(0x30);
  print_quartet(tot_frames, file);
  unsigned long frames =frame_cnt;// (TOTALFRAMES); //hdrl.strl.list_odml.frames
  file.seek(0xe0);
  print_quartet(frames, file);
  file.seek(0xe8);
  print_quartet(movi_size, file);// size again
  myCAM.CS_HIGH();
  //Close the file
  file.close();
  Serial.println(F("Record video OK"));
}

void avrc_metadata_callback(uint8_t id, const uint8_t *text) {
  /*
   * Serial.printf("==> AVRC metadata rsp: attribute id 0x%x, %s\n", id, text);
   * 0x1 = Song Title
   * 0x2 = Artist
   * 0x4 = Album
   */
  if (id == 0x1) {
    Serial.printf("Title: %s\n", text);
  }
  if (id == 0x2){
    Serial.printf("Artist: %s\n", text);
  }
  if (id == 0x4) {
    Serial.printf("Album: %s\n", text);
  }
}

void btTASK( void * pvParameters ){
  
  static i2s_config_t i2s_config = {
      .mode = (i2s_mode_t) (I2S_MODE_MASTER | I2S_MODE_TX),
      .sample_rate = 44100, // updated automatically by A2DP
      .bits_per_sample = (i2s_bits_per_sample_t)32,
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
      .communication_format = (i2s_comm_format_t) (I2S_COMM_FORMAT_STAND_I2S),
      .intr_alloc_flags = 0, // default interrupt priority
      .dma_buf_count = 8,
      .dma_buf_len = 64,
      .use_apll = true,
      .tx_desc_auto_clear = true // avoiding noise in case of data unavailability
  };
  i2s_pin_config_t my_pin_config = {
    .bck_io_num = 15,   //    BCK
    .ws_io_num = 13,    //    LRCK
    .data_out_num = 14,   //    DIN
    .data_in_num = I2S_PIN_NO_CHANGE
  };
  a2dp_sink.set_pin_config(my_pin_config);
  a2dp_sink.set_avrc_metadata_callback(avrc_metadata_callback);
  a2dp_sink.set_volume(95);
  delay(200);
  a2dp_sink.start(btDeviceName);
  for(;;){
    delay(1000);    
  }
}

unsigned long last_change, now, last_wifi_check = 0;

void ledTASK( void * pvParameters ){
  uint8_t vid, pid;
  uint8_t temp;

  // set the SPI_CS as an output:
  pinMode(CS, OUTPUT);

  Wire.begin();
  Serial.begin(115200);
  Serial.println(F("ArduCAM Start!"));
  SPI.begin();
  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt();
  }
  else  {
    Serial.println("SD Card Successfully Initialized");
  }
 
  // initialize SPI:
 //Reset the CPLD
  myCAM.write_reg(0x07, 0x80);
  delay(100);
  myCAM.write_reg(0x07, 0x00);
  delay(100);
 //Change to JPEG capture mode and initialize the OV2640 module
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  myCAM.OV2640_set_JPEG_size(OV2640_320x240);
  delay(1000);
    
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(ssid1, ssidPassphrase1);
  wifiMulti.addAP(ssid2, ssidPassphrase1);
  if(wifiMulti.run() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    }
  ArduinoOTA.setHostname(espHostName);
  ArduinoOTA.setPasswordHash(otaPasswordHash);
  ArduinoOTA
    .onStart([]() {
      vTaskDelete(btStream);
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();
  ws2812fx.init();
  ws2812fx.setBrightness(8);
  ws2812fx.setSpeed(1000);
  ws2812fx.setColor(0x007BFF);
  ws2812fx.setMode(FX_MODE_STATIC);
  ws2812fx.start();
  for(;;){
    ws2812fx.service();
    now = millis();
    if(now - last_change > TIMER_MS) {
      sprintf((char*)pname,"/%05d.avi",k);
      Video2SD(pname);
      k++;  
    }
    ArduinoOTA.handle();
    yield();
  } 
}
