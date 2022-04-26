#include <IBusBM.h>
#include <AFmotor.h>
IBusBM IBus; // IBus object
#include <Servo.h>
Servo motorSpeedA;
Servo motorSpeedB;

void setup() {
  Serial.begin(9600);   // remove comment from this line if you change the Serial port in the next line
  IBus.begin(Serial1);    // iBUS connected to Serial0 - change to Serial1 or Serial2 port when required
  motorSpeedA.attach(3);  // attaches the servo on pin 9 to the servo object
  motorSpeedB.attach(11);
}

int savevalMotorSpeedA=0;
int savevalMotorSpeedB=0;

void loop() {
  int valMotorSpeedA=0;
  int valMotorSpeedB=0;
  valMotorSpeedA = IBus.readChannel(2);
  valMotorSpeedB = IBus.readChannel(2);

  if (savevalMotorSpeedA != valMotorSpeedA) {
    //Serial.print("Elevator:   ");
    //Serial.println(valElevator); // display new value in microseconds on PC
    savevalMotorSpeedA = valMotorSpeedA;    
    motorSpeedA.writeMicroseconds(valMotorSpeedA);   // sets the servo position 
    motorSpeedB.writeMicroseconds(valMotorSpeedA);
  }

}
