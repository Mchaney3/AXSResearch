#include <IBusBM.h>
#include <Servo.h>


/*
  Translate iBUS signal to servo
  
  Supports any Arduino board where serial0 is available. 
 
  For boards where serial0 is connected to the onboard USB port (such as MEGA, UNO and micro) you need
  to disconnect the RX line from the receiver during programming. 

  Alternatively you can change the code below to use another serial port.

  Please use 5V boards only.

  Serial port RX/TX connected as follows:
  - RX connected to the iBUS servo pin (disconnect during programming on MEGA, UNO and micro boards!)
  - TX left open or for Arduino boards without an onboard USB controler - connect to an 
    USB/Serial converter to display debug information on your PC (set baud rate to 115200).  

*/

IBusBM IBus; // IBus object
Servo servoClaw;  // create servo object to control a servo
Servo servoElev;  // create servo object to control a servo
Servo servoRotate;  // create servo object to control a servo
Servo servoExtend;  // create servo object to control a servo

void setup() {
  Serial.begin(115200);   // remove comment from this line if you change the Serial port in the next line

  IBus.begin(Serial1);    // iBUS connected to Serial0 - change to Serial1 or Serial2 port when required

  servoClaw.attach(A6);  // attaches the servo on pin 9 to the servo object
  servoExtend.attach(A5);

  Serial.println("Start IBus2PWM");
}

int saveClawVal, saveExtendVal=0;

void loop() {
  int clawVal, extendVal;
  clawVal = IBus.readChannel(9); // get latest value for servo channel 1
  extendVal = IBus.readChannel(0); // get latest value for servo channel 1

  if (saveClawVal != clawVal) {
    Serial.println(clawVal); // display new value in microseconds on PC
    saveClawVal = clawVal;    
    servoClaw.writeMicroseconds(clawVal);   // sets the servo position 
  }

  if (saveExtendVal != extendVal) {
    Serial.println(extendVal); // display new value in microseconds on PC
    saveExtendVal = extendVal;    
    servoExtend.writeMicroseconds(extendVal);   // sets the servo position 
  }
  
 // delay(1);
}
