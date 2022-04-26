/**********   My Custom Libraries   ***********/

//#include "MySdFatFunctions.h"
//#include "LED.h"
//#include "IR.h"
//#include "OTA.h"

/********************************************/

void CoreOneTasks( void * pvParameters ){
  Wire.begin();
  Serial.begin(115200);
  Serial2.begin(9600);
  delay(1000);
  initSdFat();
  initWiFiOTA();    // OTA Updates handled on core 0 to kill BT stream on update start. A Semaphore can fix this :)
  initIRReceiver();
  initLEDs;

  /*******      LOOP    **********/
  for(;;){
  ws2812fx.service();
  if (IrReceiver.decode()) {
    process_IR();
  }
}
