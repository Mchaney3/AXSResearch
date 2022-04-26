#include <IBusBM.h>
#include <Servo.h>
//#include <AF//motor.h> 

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
Servo aileron;  // create servo object to control a servo
Servo elevator;  // create servo object to control a servo
//Servo //motor1;
//AF_DC//motor //motor(2);
int ledOne = 27;
int ledTwo = 32;
int ledThree = 39;
int ledFour = 42;
int ledFive = 47;

void setup() {
  ////Serial.begin(115200);   // remove comment from this line if you change the Serial port in the next line
  //Serial.println("Running");
  IBus.begin(Serial1);    // iBUS connected to Serial0 - change to Serial1 or Serial2 port when required
  
  aileron.attach(9);  // attaches the servo on pin 9 to the servo object
  elevator.attach(10);  // attaches the servo on pin 9 to the servo object
  
  pinMode(ledOne, OUTPUT);
  pinMode(ledTwo, OUTPUT);
  pinMode(ledThree, OUTPUT);
  pinMode(ledFour, OUTPUT);
  pinMode(ledFive, OUTPUT);
  
  // //Serial.println("Start Mikes Stuff");
}

int savevalAileron=0;
int savevalElevator=0;
int saveval//motor=0;
int savevalSwA = IBus.readChannel(4);
int savevalSwB = IBus.readChannel(5);
int savevalSwC = IBus.readChannel(6);
int savevalSwD = IBus.readChannel(7);

int valAileron=0;
int valElevator=0;
int val//motor=0;

int //motor_speed;

void loop() {

int valSwA = IBus.readChannel(4);
int valSwB = IBus.readChannel(5);
int valSwC = IBus.readChannel(6);
int valSwD = IBus.readChannel(7);

valAileron = IBus.readChannel(0); // get latest value for servo channel 0
valElevator = IBus.readChannel(3); // get latest value for servo channel 1
val//motor = IBus.readChannel(2); // get latest value for servo channel 8

if (savevalAileron != valAileron) {
  ////Serial.print("Aileron:   ");
  ////Serial.println(valAileron); // display new value in microseconds on PC
  savevalAileron = valAileron;    
  aileron.writeMicroseconds(valAileron);   // sets the servo position 
}

if (savevalElevator != valElevator) {
  ////Serial.print("Elevator:   ");
  ////Serial.println(valElevator); // display new value in microseconds on PC
  savevalElevator = valElevator;    
  elevator.writeMicroseconds(valElevator);   // sets the servo position 
}


if (saveval//motor != val//motor) {
  //motor.run(FORWARD);
  for (//motor_speed = map(val//motor, 1000, 2000, 0, 128); //motor_speed > val//motor; //motor_speed++) {
    //motor.setSpeed(//motor_speed);
    //delay(5);
  }
//motor.setSpeed(//motor_speed);
//saveval//motor = val//motor;
}

if (valSwA > 1500){
  digitalWrite(ledOne, HIGH);
  //Serial.print("Whiiiiite Liiiiiight");
  //Serial.println();
}
else {
  digitalWrite(ledOne, LOW);
}

if (valSwB > 1500){
  digitalWrite(ledTwo, HIGH);
  //Serial.print("Blue Light");
  //Serial.println();
}
else {
  digitalWrite(ledTwo, LOW);
}

if (valSwC == 1000) {
    aileron.write(0);
    }

if (valSwC == 1500) {
    aileron.write(90);
      }

if (valSwC == 2000) {
    aileron.write(180);
  }
}
