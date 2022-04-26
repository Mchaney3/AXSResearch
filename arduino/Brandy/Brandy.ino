
const int ledPin = LED_BUILTIN;
int ledstate = LOW ;
unsigned long previousmillis = 0;
const long interval = 1000;



void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
}

void loop() {

  unsigned long currentMillis = millis();
  if (currentMillis - previousmillis >= interval){
    previousmillis = currentMillis;
    if (ledstate == LOW) {
      ledstate=HIGH;
    }
    else {
      ledstate =LOW ;
    }
digitalWrite(ledPin, ledstate);
    Serial.print("ledState = ");
    Serial.println(ledstate);
  }
}
