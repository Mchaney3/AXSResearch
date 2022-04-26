#include <WS2812FX.h>

#define LED_COUNT 32
#define LED_PIN 2
#define MAX_NUM_CHARS 16 // maximum number of characters read from the Serial Monitor

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

char scmd[MAX_NUM_CHARS];    // char[] to store incoming serial commands
bool scmd_complete = false;  // whether the command string is complete

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

/*
 * Prints a usage menu.
 */
const char usageText[] PROGMEM = R"=====(
Usage:
m <n> : select mode <n>

b+    : increase brightness
b-    : decrease brightness
b <n> : set brightness to <n>

s+    : increase speed
s-    : decrease speed
s <n> : set speed to <n>

c 0x007BFF : set color to 0x007BFF

Have a nice day.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
)=====";

void printUsage() {
  Serial.println((const __FlashStringHelper *)usageText);
}


/*
 * Prints all available WS2812FX blinken modes.
 */
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
