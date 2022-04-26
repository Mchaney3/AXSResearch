//    32-BIT DAC A2DP_Sink with WS2182FX and bluetooth streaming/control

TaskHandle_t btStream;
TaskHandle_t led_show;
#include "credentials.h"
#include "BluetoothA2DPSink.h"
BluetoothA2DPSink a2dp_sink;
#include <WS2812FX.h>
#include "WipItsLED.h"

void setup() {
  
  Serial.begin(115200);

  xTaskCreatePinnedToCore(
                    btTASK,   /* Task function. */
                    "btStream",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    2,           /* priority of the task */
                    &btStream,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 1 */                  
  delay(500);

  xTaskCreatePinnedToCore(
                    ledTASK,   /* Task function. */
                    "led_show",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &led_show,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 0 */
  delay(500);
  
}

void loop() {
}
