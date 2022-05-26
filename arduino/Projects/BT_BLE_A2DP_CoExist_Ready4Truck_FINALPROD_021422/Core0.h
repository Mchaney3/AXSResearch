#include "BluetoothA2DPSink32.h"
#include "BluetoothSerial.h"

BluetoothA2DPSink32 a2dp_sink; // Subclass of BluetoothA2DPSink

void avrc_metadata_callback(uint8_t id, const uint8_t *text) {
  Serial.printf("==> AVRC metadata rsp: attribute id 0x%x, %s\n", id, text);
  char msg[60];
  int len = snprintf(msg, sizeof(msg), "0x%x, %s\n", id, text);
  delay(1000);
}

void audio_state_changed(esp_a2d_audio_state_t state, void *ptr){
  Serial.println(a2dp_sink.to_str(state));
}

void connection_state_changed(esp_a2d_connection_state_t state, void *ptr){
  Serial.println(a2dp_sink.to_str(state));
}

#define MAX_NUM_CHARS 16 // maximum number of characters read from the Serial Monitor

BluetoothSerial ESP_BT;
int incoming;

char scmd[MAX_NUM_CHARS];    // char[] to store incoming serial commands
bool scmd_complete = false;  // whether the command string is complete

void printModes() {
  Serial.println(F("Supporting the following modes: "));
  Serial.println();
  for(int i=0; i < ws2812fx.getModeCount(); i++) {
    Serial.print(i);
    Serial.print(F("\t"));
    Serial.println(ws2812fx.getModeName(i));
  }
  Serial.println();
}
/*
 * Checks received command and calls corresponding functions.
 */
void process_command() {
  if (strcmp(scmd,"b+") == 0) {
    ws2812fx.increaseBrightness(25);
    Serial.print(F("Increased brightness by 25 to: "));
    Serial.println(ws2812fx.getBrightness());
  }

  if (strcmp(scmd,"b-") == 0) {
    ws2812fx.decreaseBrightness(25); 
    Serial.print(F("Decreased brightness by 25 to: "));
    Serial.println(ws2812fx.getBrightness());
  }

  if (strncmp(scmd,"b ",2) == 0) {
    uint8_t b = (uint8_t)atoi(scmd + 2);
    ws2812fx.setBrightness(b);
    Serial.print(F("Set brightness to: "));
    Serial.println(ws2812fx.getBrightness());
  }

  if (strcmp(scmd,"s+") == 0) {
    ws2812fx.setSpeed(ws2812fx.getSpeed() * 1.2);
    Serial.print(F("Increased speed by 20% to: "));
    Serial.println(ws2812fx.getSpeed());
  }

  if (strcmp(scmd,"s-") == 0) {
    ws2812fx.setSpeed(ws2812fx.getSpeed() * 0.8);
    Serial.print(F("Decreased speed by 20% to: "));
    Serial.println(ws2812fx.getSpeed());
  }

  if (strncmp(scmd,"s ",2) == 0) {
    uint16_t s = (uint16_t)atoi(scmd + 2);
    ws2812fx.setSpeed(s); 
    Serial.print(F("Set speed to: "));
    Serial.println(ws2812fx.getSpeed());
  }

  if (strncmp(scmd,"m ",2) == 0) {
    uint8_t m = (uint8_t)atoi(scmd + 2);
    ws2812fx.setMode(m);
    Serial.print(F("Set mode to: "));
    Serial.print(ws2812fx.getMode());
    Serial.print(" - ");
    Serial.println(ws2812fx.getModeName(ws2812fx.getMode()));
  }

  if (strncmp(scmd,"c ",2) == 0) {
    uint32_t c = (uint32_t)strtoul(scmd + 2, NULL, 16);
    ws2812fx.setColor(c);
    Serial.print(F("Set color to: 0x"));
    Serial.println(ws2812fx.getColor(), HEX);
  }

  scmd[0] = '\0';         // reset the commandstring
  scmd_complete = false;  // reset command complete
}

void btCommandIncoming() {
// -------------------- Receive Bluetooth signal ----------------------
  if (ESP_BT.available()) {
    incoming = ESP_BT.read(); //Read what we receive 

    // separate button ID from button value -> button ID is 10, 20, 30, etc, value is 1 or 0
    int ledSetting = floor(incoming / 10);
    int ledSettingValue = incoming % 10;
    
    switch (ledSetting) {
   //   First int sent from app should be 1 to identify were changing mode. 2 if changing speed, 3 if changing brightness
   //   Next numbers should be mode, speed or brightness values
   //   I can adjust this later to get a 6 or 10 digit value to identify all settings at once  
      case 1:
        ws2812fx.setMode(ledSettingValue);
        Serial.print(F("Set mode to: "));
        Serial.print(ws2812fx.getMode());
        Serial.print(" - ");
        Serial.println(ws2812fx.getModeName(ws2812fx.getMode()));
        break;
      case 2:  
        Serial.print("Setting speed to: "); Serial.println(ledSettingValue);
        ws2812fx.setSpeed(ledSettingValue);
        Serial.print(F("Set Speed to: "));
        Serial.print(ws2812fx.getSpeed());
        Serial.print(" - ");
        Serial.println(ws2812fx.getModeName(ws2812fx.getMode()));
        break;
      case 3:  
        Serial.print("Setting brightness to: "); Serial.println(ledSettingValue);
        ws2812fx.setBrightness(ledSettingValue);
        Serial.print(F("Set Brightness to: "));
        Serial.print(ws2812fx.getBrightness());
        Serial.print(" - ");
        Serial.println(ws2812fx.getModeName(ws2812fx.getMode()));
        break;
    }
    
  scmd[0] = '\0';         // reset the commandstring
  scmd_complete = false;  // reset command complete
  }
}

/*
 * Reads new input from serial to scmd string. Command is completed on \n
 */
void recvChar(void) {
  static byte index = 0;
  while (Serial.available() > 0 && scmd_complete == false) {
    char rc = Serial.read();
    if (rc != '\n') {
      if(index < MAX_NUM_CHARS) scmd[index++] = rc;
    } else {
      scmd[index] = '\0'; // terminate the string
      index = 0;
      scmd_complete = true;
      Serial.print("received '"); Serial.print(scmd); Serial.println("'");
    }
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

a2dp_sink.set_on_connection_state_changed(connection_state_changed);
  a2dp_sink.set_on_audio_state_changed(audio_state_changed);
  a2dp_sink.set_avrc_metadata_callback(avrc_metadata_callback);
  a2dp_sink.set_volume(85);
  a2dp_sink.set_bits_per_sample(32);
  i2s_pin_config_t my_pin_config = {
    .bck_io_num = 15,   //    BCK
    .ws_io_num = 13,    //    LRCK
    .data_out_num = 14,   //    DIN
    .data_in_num = I2S_PIN_NO_CHANGE
  };
  a2dp_sink.set_pin_config(my_pin_config);
  a2dp_sink.start("");
  if(!ESP_BT.begin("Mike's Truck")) {
    Serial.println("Could not start LED BLuetooth Control");
  } 
  else {
  Serial.println("Bluetooth Started");
  printModes();
  }
  for(;;){
    btCommandIncoming();
    delay(1);
  }
}
