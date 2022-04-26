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

  *** PINOUT ***
  * Input from receiver on pin 36 of #SP32
  * Output tot receiver on pin 37 of ESP32
  * Aileron control on pin 38
  * Elevator control on pin 39
  * Rudder Control on pin 34
  * Speed control on pin 35

*/

#include <IBusBM.h>
//#include <ESP32Servo.h>
#include <Servo.h>
#include <HardwareSerial.h>
IBusBM IBus; // IBus object
Servo aileron;  // create servo object to control a servo
Servo elevator;  // create servo object to control a servo
Servo rudder;
Servo speedcontrol;

#include <SoftwareSerial.h>

#define RXD2 23 //16 is used for OLED_RST !
#define TXD2 17

//SoftwareSerial Serial2(2);


void setup() {

  Serial.begin(9600);   // Diagnostic Serial
  Serial2.begin(9600);
  IBus.begin(Serial2);    // iBUS connected to Serial0 - change to Serial1 or Serial2 port when required

  // Allow allocation of all timers
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  aileron.setPeriodHertz(50);
  elevator.setPeriodHertz(50);
  rudder.setPeriodHertz(50);
  speedcontrol.setPeriodHertz(50);
  aileron.attach(25);  // attaches the servo on pin 38 on the ESP32 23
  elevator.attach(26);  // attaches the servo on pin 39 on the ESP32 19
  rudder.attach(22); // attaches the servo on pin 34 on the ESP32 22
  speedcontrol.attach(21); // attaches the servo on pin 35 on the ESP32 21
  
}

int savevalAileron=0;
int savevalElevator=0;
int savevalRudder=0;
int savevalSpeedControl=0;

void loop() {
  int valAileron;
  int valElevator;
  int valRudder;
  int valSpeedControl;
  valAileron = IBus.readChannel(0); // get latest value for servo channel 1
  valElevator = IBus.readChannel(1); // get latest value for servo channel 2
  valRudder = IBus.readChannel(2); // get latest value for servo channel  3 ***********Check channels for speed control and rudder
  valSpeedControl = IBus.readChannel(3); // get latest value for servo channel  4

  if (savevalAileron != valAileron) {
    Serial.print("Aileron:   ");
    Serial.println(valAileron); // display new value in microseconds on PC
    savevalAileron = valAileron;    
    aileron.writeMicroseconds(valAileron);   // sets the servo position 
  }

  if (savevalElevator != valElevator) {
    Serial.print("Elevator:   ");
    Serial.println(valElevator); // display new value in microseconds on PC
    savevalElevator = valElevator;    
    elevator.writeMicroseconds(valElevator);   // sets the servo position 
  }

  if (savevalRudder != valRudder) {
    Serial.print("Rudder:   ");
    Serial.println(valRudder); // display new value in microseconds on PC
    savevalRudder = valRudder;    
    rudder.writeMicroseconds(valRudder);   // sets the servo position 
  }

  if (savevalSpeedControl != valSpeedControl) {
    Serial.print("Throttle:   ");
    Serial.println(valSpeedControl); // display new value in microseconds on PC
    savevalSpeedControl = valSpeedControl;    
    speedcontrol.writeMicroseconds(valSpeedControl);   // sets the servo position 
  }
  
  delay(50);
}
