// Animates white pixels to simulate flying through a star field

#include <WifiEspNowBroadcast.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <SPI.h>
#include <TFT_eSPI.h>

// Use hardware SPI
TFT_eSPI tft = TFT_eSPI();

// Stock font and GFXFF reference handle
#define GFXFF 1
#define FF18 &FreeSans12pt7b

// Custom are fonts added to library "TFT_eSPI\Fonts\Custom" folder
// a #include must also be added to the "User_Custom_Fonts.h" file
// in the "TFT_eSPI\User_Setups" folder. See example entries.
#define CF_OL24 &Orbitron_Light_24
#define CF_OL32 &Orbitron_Light_32
#define CF_RT24 &Roboto_Thin_24
#define CF_S24  &Satisfy_24
#define CF_Y32  &Yellowtail_32

// With 1024 stars the update rate is ~65 frames per second
#define NSTARS 1024
uint8_t sx[NSTARS] = {};
uint8_t sy[NSTARS] = {};
uint8_t sz[NSTARS] = {};

uint8_t za, zb, zc, zx;

// Fast 0-255 random number generator from http://eternityforest.com/Projects/rng.php:
uint8_t __attribute__((always_inline)) rng()
{
  zx++;
  za = (za^zc^zx);
  zb = (zb+za);
  zc = ((zc+(zb>>1))^za);
  return zc;
}

unsigned long previousMillis = 0;        // will store last time LED was updated
const long interval = 3000;

static const int BUTTON_PIN = 13;



void setup() {
  WiFi.persistent(false);
  bool ok = WifiEspNowBroadcast.begin("ESPNOW", 3);
  if (!ok) {
    Serial.println("WifiEspNowBroadcast.begin() failed");
    ESP.restart();
  }
  // WifiEspNowBroadcast.begin() function sets WiFi to AP+STA mode.
  // The AP interface is also controlled by WifiEspNowBroadcast.
  // You may use the STA interface after calling WifiEspNowBroadcast.begin().
  // For best results, ensure all devices are using the same WiFi channel.

  WifiEspNowBroadcast.onReceive(processRx, nullptr);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.print("MAC address of this node is ");
  Serial.println(WiFi.softAPmacAddress());
  Serial.println("Press the button to send a message");



/******   STARFIELD   ***************/  
  za = random(256);
  zb = random(256);
  zc = random(256);
  zx = random(256);

  Serial.begin(115200);
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  // fastSetup() must be used immediately before fastPixel() to prepare screen
  // It must be called after any other graphics drawing function call if fastPixel()
  // is to be called again
  //tft.fastSetup(); // Prepare plot window range for fast pixel plotting
}

void processRx(const uint8_t mac[WIFIESPNOW_ALEN], const uint8_t* buf, size_t count, void* arg){
  Serial.printf("Message from %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3],
                mac[4], mac[5]);
  for (size_t i = 0; i < count; ++i) {
    char message = static_cast<char>(buf[i]);
    Serial.print(message);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    // Calcualte frames per second
    //tft.setTextDatum(MC_DATUM);
    
    tft.setTextPadding(80);
    tft.setFreeFont(CF_OL24);
    
    tft.drawString(String(message), 5, 180, GFXFF);// Print the test text in the custom font
    // Reset text padding to zero (default)
    tft.setTextPadding(0);
  }
  Serial.println();
}

void sendMessage(){
  char msg[60];
  int len = snprintf(msg, sizeof(msg), "hello ESP-NOW from %s at %lu",
                     WiFi.softAPmacAddress().c_str(), millis());
  WifiEspNowBroadcast.send(reinterpret_cast<const uint8_t*>(msg), len);

  Serial.println("Sending message");
  Serial.println(msg);
  Serial.print("Recipients:");
  const int MAX_PEERS = 20;
  WifiEspNowPeerInfo peers[MAX_PEERS];
  int nPeers = std::min(WifiEspNow.listPeers(peers, MAX_PEERS), MAX_PEERS);
  for (int i = 0; i < nPeers; ++i) {
    Serial.printf(" %02X:%02X:%02X:%02X:%02X:%02X", peers[i].mac[0], peers[i].mac[1],
                  peers[i].mac[2], peers[i].mac[3], peers[i].mac[4], peers[i].mac[5]);
  }
  Serial.println();
}

void loop(){
/************   ESP-NPW   *************/
  if (digitalRead(BUTTON_PIN) == LOW) { // button is pressed
    sendMessage();

    while (digitalRead(BUTTON_PIN) == LOW) // wait for button release
      ;
  }

  WifiEspNowBroadcast.loop();

/**************   STARFIELD   ************/
  unsigned long t0 = micros();
  uint8_t spawnDepthVariation = 255;

  for(int i = 0; i < NSTARS; ++i)
  {
    if (sz[i] <= 1)
    {
      sx[i] = 160 - 120 + rng();
      sy[i] = rng();
      sz[i] = spawnDepthVariation--;
    }
    else
    {
      int old_screen_x = ((int)sx[i] - 160) * 256 / sz[i] + 160;
      int old_screen_y = ((int)sy[i] - 120) * 256 / sz[i] + 120;

      // This is a faster pixel drawing function for occassions where many single pixels must be drawn
      tft.drawPixel(old_screen_x, old_screen_y,TFT_BLACK);

      sz[i] -= 2;
      if (sz[i] > 1)
      {
        int screen_x = ((int)sx[i] - 160) * 256 / sz[i] + 160;
        int screen_y = ((int)sy[i] - 120) * 256 / sz[i] + 120;
  
        if (screen_x >= 0 && screen_y >= 0 && screen_x < 320 && screen_y < 240)
        {
          uint8_t r, g, b;
          r = g = b = 255 - sz[i];
          tft.drawPixel(screen_x, screen_y, tft.color565(r,g,b));
        }
        else
          sz[i] = 0; // Out of screen, die.
      }
    }
  }

/*************    START UPDATING TFT    *****************/
  
  unsigned long t1 = micros();
  unsigned long currentMillis = millis();
  //static char timeMicros[8] = {};
  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    int fps = 1.0/((t1 - t0)/1000000.0);
    if (fps > 60) {
      tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    }
    if (fps > 120) {
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
    }
    if (fps < 60) {
      tft.setTextColor(TFT_RED, TFT_BLACK);
    }
    // Calcualte frames per second
    //tft.setTextDatum(MC_DATUM);
    
    tft.setTextPadding(80);
    tft.setFreeFont(CF_OL24);
    tft.drawString(String(fps), 5, 205, GFXFF);// Print the test text in the custom font
    // Reset text padding to zero (default)
    tft.setTextPadding(0);
    Serial.println(1.0/((t1 - t0)/1000000.0));
  }
}
