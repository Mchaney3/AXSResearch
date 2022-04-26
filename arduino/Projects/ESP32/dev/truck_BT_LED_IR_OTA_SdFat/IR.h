#define IR_RECEIVE_PIN  34  // analog pin
#define DECODE_NEC      // Includes Apple and Onkyo
//#define DEBUG         // to see if attachInterrupt is used
//#define TRACE         // to see the state of the ISR state machine
#define STR_HELPER(x)   #x // IR Receiver helper
#define STR(x)          STR_HELPER(x) // IR Receiver Helper
int OKAY_SHIFT = 0;

void initIRReceiver(){
  IrReceiver.begin(IR_RECEIVE_PIN, DISABLE_LED_FEEDBACK);
  Serial.print(F("\rNEC Protocol initiated. IR ready at hardware pin "));
  Serial.println(IR_RECEIVE_PIN);
}

void process_IR(){    //    Need to write case for this instead of if, if, if, if
  if (IrReceiver.decodedIRData.protocol = NEC) {
    IrReceiver.printIRResultShort(&Serial);
    Serial.println();
    IrReceiver.resume(); // Enable receiving of the next value
    //    0x18 = Up arrow - Increase brightness by 25
    if (IrReceiver.decodedIRData.command == 0x18) {
      if (OKAY_SHIFT == 0) {
        Serial.print("Setting brightness to ");
        Serial.println(ws2812fx.getBrightness() + 25);
        ws2812fx.setBrightness(ws2812fx.getBrightness() + 25);
      } else {
        ws2812fx.setColor(ws2812fx.getColor() + 1);
        Serial.print("Setting color to ");
        Serial.println(ws2812fx.getColor() + 1);
      }
    }
    //    0x52 = Down Arrow - Decrease brightness by 25
    else if (IrReceiver.decodedIRData.command == 0x52) {
      if (OKAY_SHIFT == 0) {
        Serial.print("Setting brightness to ");
        Serial.println(ws2812fx.getBrightness() - 25);
        ws2812fx.setBrightness(ws2812fx.getBrightness() - 25);
        } else {
          ws2812fx.setColor(ws2812fx.getColor() - 1);
          Serial.print("Setting color to ");
          Serial.println(ws2812fx.getColor() - 1);
        }
      }
    //    0x8 = Left arrow
    else if (IrReceiver.decodedIRData.command == 0x8) {
      if (OKAY_SHIFT == 0) {
        int lastMode = ws2812fx.getMode() - 1;
        Serial.print("Settting LED mode to ");
        Serial.println(lastMode);
        ws2812fx.setMode(lastMode);
      } else {
        int speedDecrease = ws2812fx.getSpeed() - 10;
        Serial.print("Settting LED speed mode to ");
        Serial.println(speedDecrease);
        ws2812fx.setMode(speedDecrease);
      }
    }
    //    0x5A = Right arrow
    else if (IrReceiver.decodedIRData.command == 0x5A) {
      if (OKAY_SHIFT == 0) {
        int nextMode = ws2812fx.getMode() + 1;
        Serial.print("Settting LED mode to ");
        Serial.println(nextMode);
        ws2812fx.setMode(nextMode);
      } else {
        int speedIncrease = ws2812fx.getSpeed() + 10;
        Serial.print("Settting LED speed mode to ");
        Serial.println(speedIncrease);
        ws2812fx.setMode(speedIncrease);
      }
    }
    //    0x1C = OK
    else if (IrReceiver.decodedIRData.command == 0x1C) {
      if (OKAY_SHIFT == 0) {
        OKAY_SHIFT++;
      } else if (OKAY_SHIFT == 1) {
        OKAY_SHIFT--;
      }
      Serial.println(OKAY_SHIFT);
    }
    //    0x45 = 1
    else if (IrReceiver.decodedIRData.command == 0x45) {
      ws2812fx.setMode(FX_MODE_CHASE_BLACKOUT_RAINBOW);
      Serial.print("Set LED mode to ");
      Serial.println("FX_MODE_CHASE_BLACKOUT_RAINBOW");
      }
    //    0x46 = 2
    else if (IrReceiver.decodedIRData.command == 0x46) {
      ws2812fx.setMode(FX_MODE_FIREWORKS_RANDOM);
      Serial.print("Set LED mode to ");
      Serial.println("FX_MODE_FIREWORKS_RANDOM");
    }
    //    0x47 = 3
    else if (IrReceiver.decodedIRData.command == 0x47) {
      ws2812fx.setMode(FX_MODE_HYPER_SPARKLE );
      Serial.print("Set LED mode to ");
      Serial.println("FX_MODE_HYPER_SPARKLE");
    }
    //    0x44 = 4
    else if (IrReceiver.decodedIRData.command == 0x44) {
      ws2812fx.setMode(FX_MODE_THEATER_CHASE);
      Serial.print("Set LED mode to ");
      Serial.println("FX_MODE_THEATER_CHASE");
    }
    //    0x40 = 5
    else if (IrReceiver.decodedIRData.command == 0x40) {
      ws2812fx.setMode(FX_MODE_COLOR_SWEEP_RANDOM);
      Serial.print("Set LED mode to ");
      Serial.println("FX_MODE_COLOR_SWEEP_RANDOM");
    }
    //    0x43 = 6
    else if (IrReceiver.decodedIRData.command == 0x43) {
      ws2812fx.setMode(FX_MODE_DUAL_SCAN);
      Serial.print("Set LED mode to ");
      Serial.println("FX_MODE_DUAL_SCAN");
    }
    //    0x7 = 7
    else if (IrReceiver.decodedIRData.command == 0x7) {
      ws2812fx.setMode(FX_MODE_MULTI_DYNAMIC);
      Serial.print("Set LED mode to ");
      Serial.println("FX_MODE_MULTI_DYNAMIC");
    }
    //    0x15 = 8
    else if (IrReceiver.decodedIRData.command == 0x15) {
      ws2812fx.setMode(FX_MODE_COLOR_WIPE_RANDOM);
      Serial.print("Set LED mode to ");
      Serial.println("FX_MODE_COLOR_WIPE_RANDOM");
    }
    //    0x9 = 9
    else if (IrReceiver.decodedIRData.command == 0x9) {
      ws2812fx.setMode(FX_MODE_FIRE_FLICKER);
      Serial.print("Set LED mode to ");
      Serial.println("FX_MODE_FIRE_FLICKER");
    }
    //    0x19 = 0
    else if (IrReceiver.decodedIRData.command == 0x19) {
      ws2812fx.setMode(FX_MODE_MERRY_CHRISTMAS );
      Serial.print("Set LED mode to ");
      Serial.println("FX_MODE_MERRY_CHRISTMAS");
    }
    //    0x16 = Asterisk
    else if (IrReceiver.decodedIRData.command == 0x16) {
      int randMode = random(1,65);
      ws2812fx.setMode(randMode);
      Serial.print("Set LED mode to a rAnDoM mode named ");
      Serial.println(ws2812fx.getModeName(randMode));
    }
    //    0xD = Pound sign. Use this to follow with 0xx for mode number, 1xx for brightness, 2xx for speed, followed by ok
    else if (IrReceiver.decodedIRData.command == 0x44) {
      /*   # Sign followed by 1xx can set the mode
      *    # Sign followed by 2xx can set the brighntess
      *    # Sign followed by 3xx will set speed    
      */
    }
  }
}
