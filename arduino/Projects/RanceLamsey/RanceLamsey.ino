int one = 49;
int two = 50;
int three = 51;
int ten = 10;
int numBytes = 0;
int i = 0;
int j = 0;
String letters[]= {"a", "b", "c", "d", "e", "f","g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0"};
String randString = "";
String whatabitch = "Pop Tubzy's blue pill, trip a little and then sleep it off wondering what happened";
String neo = "Pop Tubzy's Red Pill and trip on code forever";

void setup() {
  Serial.begin(115200);
  delay(3000);
  Serial.println("\r\n");
  Serial.println("Hello Rance");
  delay(3000);
  Serial.println("\r\nTubzy has determined you are the one");
  delay(5000);
  Serial.println("\nYou can enter 1 and go back to sleep. OR, you can enter 2 and see how deep the rabbit hole goes");
  Serial.print("\n\n"); Serial.print("1. "); Serial.println(whatabitch);
  Serial.print("\n"); Serial.print("2. "); Serial.println(neo);
}
  
void loop() {  
  if(Serial.available()){
        int(input) = Serial.read();
//        Serial.print("You typed: " );
//        Serial.println(input);
        if(input == one) {
          Serial.println("\nWhat a bitch");
        }
        if (input == two) {
          Serial.println("\nI CAN SHOW YOU THE WOOOOORLD! SHINING SHIMMERING SPLEEEENDIIIID");
          delay(3000);
          Serial.println("\nWelcome Lance! I'm SO FREAKING EXCITED FOR YOU DUDE!");
          Serial.println("I wrote this app in in about 10 minutes and will send you the source code as soon as you're setup and ready :)");
          Serial.println("I hope it brings you as much happiness as it has brought me!");
          Serial.println("This is an Espressif ESP32-da Development board, or ESP32 Devkit V4");
          Serial.println("Google either of those to get more info but it's fully support in Arduino, as you can tell if you're reading this");
          Serial.println("Love you brother");
          Serial.print("This device will self destruct in: "); Serial.println("10"); delay(1000); Serial.println("9"); delay(1000); Serial.println("8"); delay(1000); Serial.println("7"); delay(1000); Serial.println("6"); delay(1000); Serial.println("5"); delay(1000); Serial.println("4"); delay(1000); Serial.println("3"); delay(1000); Serial.println("2"); delay(1000); Serial.println("1"); delay(1000); Serial.println("0"); delay(1000); Serial.println("KABOOOOOOOOOOM!"); delay(5000); Serial.println("J/K :)"); delay(1000); Serial.println("Not Really"); delay(2000); Serial.print("3, "); delay(500); Serial.print("2, "); delay(500); Serial.print("1, "); delay(500);
          for (;;){
            for(i = 0; i<112; i++) {
             randString = randString + letters[random(0, 36)];
            }
            Serial.println(randString);
            randString = "";
            delay(10);
          }
        }
        if (input == three) {
          Serial.println("Ah the hidden choice. Flip Tubzy off, tell him drugs are for losers and be the one anyway! BAHAHAHAHA.\r Could have tripped AND been the one. Loser.");
        }
        if (input != one && input != two && input != ten) {
          Serial.println("\nPretty basic instruction to follow there bruh");
        }
    }
}
