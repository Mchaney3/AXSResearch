#include <Adafruit_GFX.h>
#include <DinoGame.h>

//#define USE_NOINTERNET

#ifdef USE_NOINTERNET
#include "no_internet.h"
#endif

#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();       // Invoke custom library
DinoGame game(tft);

void setup(void) {
  Serial.begin(115200);
  tft.begin();
  tft.setRotation(1);
  game.newGame();
  pinMode(13, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  #ifdef USE_NOINTERNET
  tft.drawBitmap(0, 131, 320, 109, no_internet);
  #endif
}

void loop() {
  char c = Serial.read();
  if(c >= '0' && c <= '9') game.setSpeedModifier(c - '0');
  if(!digitalRead(13)) game.jump();
  else{
    if(!digitalRead(12)) game.duck();
    else game.stand();  
  }
  while(game.isGameOver()){
    if(!digitalRead(13)){
      game.newGame();
      #ifdef USE_NOINTERNET
      tft.drawBitmap(0, 131, 320, 109, no_internet);
      #endif   
      break;
    }
  }
  game.loop();
}
