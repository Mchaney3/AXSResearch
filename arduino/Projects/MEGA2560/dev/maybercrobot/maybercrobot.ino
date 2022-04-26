int throttleTx;
int throttleTxPin = 5;
int motorSpeedPinA = 3;
int motorSpeedA;
int motorDirectionA = 12;
int motorBrakeA = 9;
int motorSpeedPinB = 11;
int motorSpeedB;
int motorDirectionB = 13;
int motorBrakeB = 8;

void setup() {
  pinMode(throttleTxPin, INPUT);
  pinMode(motorSpeedPinA, OUTPUT);
  pinMode(motorDirectionA, OUTPUT);
  pinMode(motorBrakeA, OUTPUT);
  pinMode(motorSpeedPinB, OUTPUT);
  pinMode(motorDirectionB, OUTPUT);
  pinMode(motorBrakeB, OUTPUT);
  digitalWrite(motorDirectionA, HIGH);
  digitalWrite(motorDirectionB, HIGH);
  digitalWrite(motorBrakeA, LOW);
  digitalWrite(motorBrakeB, LOW);

}

void loop() {
  motorSpeedA = 0;
  motorSpeedB = 0;
  throttleTx = pulseIn(throttleTxPin,HIGH,25000);
  if(throttleTx > 1100) {
    motorSpeedA = map(throttleTx, 1000, 2000, 0, 255);
    motorSpeedB = map(throttleTx, 1000, 2000, 0, 255);
    if(motorSpeedA >= 0) {
      analogWrite(motorSpeedPinA, motorSpeedA);
      analogWrite(motorSpeedPinB, motorSpeedB);
    }
  }
  

}
