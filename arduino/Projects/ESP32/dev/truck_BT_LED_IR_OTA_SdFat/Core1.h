void CoreOneTasks( void * pvParameters ){
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
    delay(1);
  }
}
