void setup() {
  Serial.begin(115200);
  Serial2.begin(115200);

}

void loop() {
  Serial2.println(millis());
  delay(1000);
}
