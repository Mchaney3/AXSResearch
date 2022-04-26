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
#include <IBusBM.h>
#include <Servo.h>
IBusBM IBus; // IBus object
Servo aileron;  // create servo object to control a servo
Servo elevator;  // create servo object to control a servo
#include <SoftwareSerial.h>
SoftwareSerial ESP32(52, 53); // RX, TX

void setup() {
  
  Serial.begin(115200);   // remove comment from this line if you change the Serial port in the next line
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Native USB only
  }
  ESP32.begin(115200); // Open software serial communication with ESP32
  ESP32.println("Hello, world?");
  
  IBus.begin(Serial1);    // iBUS connected to Serial0 - change to Serial1 or Serial2 port when required

  aileron.attach(2);  // attaches the servo on pin 9 to the servo object
  elevator.attach(5);  // attaches the servo on pin 9 to the servo object
  
  Serial.println("Start IBus2PWM");
}

int savevalAileron=0;
int savevalElevator=0;

void loop() {
  int valAileron;
  int valElevator;
  valAileron = IBus.readChannel(0); // get latest value for servo channel 1
  valElevator = IBus.readChannel(1); // get latest value for servo channel 2

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
  
  delay(50);
}
