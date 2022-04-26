#include <IBusBM.h>
#include <Servo.h>
IBusBM IBus; // IBus object
Servo aileron;  // create servo object to control a servo

void setup() {
  IBus.begin(Serial);
  aileron.attach(13);
}

int savevalAileron=0;
int valAileron=0;

void loop() {
  valAileron = IBus.readChannel(0); // get latest value for servo channel 0
  if (savevalAileron != valAileron) {
  ////Serial.print("Aileron:   ");
  ////Serial.println(valAileron); // display new value in microseconds on PC
  savevalAileron = valAileron;    
  aileron.writeMicroseconds(valAileron);   // sets the servo position 
}

}
