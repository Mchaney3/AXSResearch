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
Servo aileron;  // create servo object to control a servo
Servo elevator;  // create servo object to control a servo
Servo rudder;
Servo throttle;
//Serial2 ESP32Comms; // create serial connection object for ESP32 on Serial2 Pins D17 RX2 and D16 TX2

void setup() {
  Serial.begin(9600);   // remove comment from this line if you change the Serial port in the next line
  IBus.begin(Serial1);    // iBUS connected to Serial0 - change to Serial1 or Serial2 port when required
  Serial2.begin(115200);

  aileron.attach(7);  // attaches the servo on pin 9 to the servo object
  elevator.attach(6);  // attaches the servo on pin 9 to the servo object
  rudder.attach(5);  // attaches the servo on pin 9 to the servo object
  throttle.attach(4);  // attaches the servo on pin 9 to the servo object
  
  //Serial.println("Start IBus2PWM");
}

int savevalAileron=0;
int savevalElevator=0;
int savevalThrottle=0;
int savevalRudder=0;

void loop() {
  int valAileron;
  int valElevator;
  int valThrottle;
  int valRudder;
  valAileron = IBus.readChannel(0); // get latest value for servo channel 1
  valElevator = IBus.readChannel(1); // get latest value for servo channel 2
  valThrottle = IBus.readChannel(2); // get latest value for servo channel 1
  valRudder = IBus.readChannel(3); // get latest value for servo channel 2

  if (savevalAileron != valAileron) {
    //Serial.print("Aileron:   ");
    //Serial.println(valAileron); // display new value in microseconds on PC
    savevalAileron = valAileron;    
    aileron.writeMicroseconds(valAileron);   // sets the servo position 
  }

  if (savevalElevator != valElevator) {
    //Serial.print("Elevator:   ");
    //Serial.println(valElevator); // display new value in microseconds on PC
    savevalElevator = valElevator;    
    elevator.writeMicroseconds(valElevator);   // sets the servo position 
  }

  if (savevalThrottle != valThrottle) {
    //Serial.print("Elevator:   ");
    //Serial.println(valElevator); // display new value in microseconds on PC
    savevalThrottle = valThrottle;    
    throttle.writeMicroseconds(valThrottle);   // sets the servo position 
  }

  if (savevalRudder != valRudder) {
    //Serial.print("Elevator:   ");
    //Serial.println(valElevator); // display new value in microseconds on PC
    savevalRudder = valRudder;    
    rudder.writeMicroseconds(valRudder);   // sets the servo position 
  }
  
  delay(50);
  checkEsp32Com();
}

void checkEsp32Com() {
  /*
  // read from port 0, send to port 1:
  if (Serial.available()) {
    int inByte = Serial.read();
    Serial1.print(inByte, DEC);
  }
  */
  // read from port 1, send to port 0:
  while (Serial2.available()) {
    char esp32DataIn = Serial2.read();
    //  Serial.println(dataIn, char);
    Serial.print(esp32DataIn);
  }
  Serial.println();
}
