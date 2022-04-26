#define LED_COUNT 255
#define LED_PIN 2

WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void initLEDs() {
  Serial.println("Initializing Bluetooth");
  ws2812fx.init();
  ws2812fx.setBrightness(8);
  ws2812fx.setSpeed(1000);
  ws2812fx.setColor(0x007BFF);
  ws2812fx.setMode(FX_MODE_CHASE_RAINBOW);
  ws2812fx.start();
  Serial.println("LED's Initiated");
}
