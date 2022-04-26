#include "BluetoothA2DPSink32.h"
#include <WifiEspNow.h>

BluetoothA2DPSink32 a2dp_sink; // Subclass of BluetoothA2DPSink
String nowPlaying;
void avrc_metadata_callback(uint8_t id, const uint8_t *text) {
  Serial.printf("==> AVRC metadata rsp: attribute id 0x%x, %s\n", id, text);
  char msg[60];
  int len = snprintf(msg, sizeof(msg), "0x%x, %s\n", id, text);
  if(id = 1){
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setCursor(0, 0); // Set cursor at top left of screen
    tft.printf("Now Playing: ");
    nowPlaying = tft.printf("%x", text);
    tft.println();
  }
  delay(1000);
}

void audio_state_changed(esp_a2d_audio_state_t state, void *ptr){
  Serial.println(a2dp_sink.to_str(state));
}

void connection_state_changed(esp_a2d_connection_state_t state, void *ptr){
  Serial.println(a2dp_sink.to_str(state));
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
    .data_out_num = 25,   //    DIN
    .data_in_num = I2S_PIN_NO_CHANGE
  };
  a2dp_sink.set_pin_config(my_pin_config);
  a2dp_sink.set_on_connection_state_changed(connection_state_changed);
  a2dp_sink.set_on_audio_state_changed(audio_state_changed);
  a2dp_sink.set_avrc_metadata_callback(avrc_metadata_callback);
  a2dp_sink.set_volume(85);
  a2dp_sink.set_bits_per_sample(32);
  a2dp_sink.start(btDeviceName);
  for(;;){
    tft.fillRectHGradient(0, 0,320, 5, TFT_CYAN, TFT_ORANGE);
    delay(1);
  }
}
