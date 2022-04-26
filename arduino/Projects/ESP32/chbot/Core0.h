/**********   My Custom Libraries   ***********/

//#include "MySdFatFunctions.h"
//#include "OTA.h"
//#include "BT.h"

/********************************************/

/*************************            Bluetooth audio and OTA on core 0       ************************************/

void btTASK( void * pvParameters ){
  initBTTask;
  for(;;){
  ArduinoOTA.handle();
  delay(100);
  }
}
