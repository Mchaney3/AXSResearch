// Boot Music

#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978
#define REST      0


// change this to make the song slower or faster
int tempo = 180;

// change this to whichever pin you want to use
int buzzer = 49;

// notes of the moledy followed by the duration.
// a 4 means a quarter note, 8 an eighteenth , 16 sixteenth, so on
// !!negative numbers are used to represent dotted notes,
// so -4 means a dotted quarter note, that is, a quarter plus an eighteenth!!
int melody[] = {

  // Nokia Ringtone 
  // Score available at https://musescore.com/user/29944637/scores/5266155
  
  NOTE_E5, 8, NOTE_D5, 8, NOTE_FS4, 4, NOTE_GS4, 4, 
  NOTE_CS5, 8, NOTE_B4, 8, NOTE_D4, 4, NOTE_E4, 4, 
  NOTE_B4, 8, NOTE_A4, 8, NOTE_CS4, 4, NOTE_E4, 4,
  NOTE_A4, 2, 
};

// sizeof gives the number of bytes, each int value is composed of two bytes (16 bits)
// there are two values per note (pitch and duration), so for each note there are four bytes
int notes = sizeof(melody) / sizeof(melody[0]) / 2;

// this calculates the duration of a whole note in ms
int wholenote = (60000 * 4) / tempo;

int divider = 0, noteDuration = 0;

/*
 * 
Function          Channel A       Channel B
Direction         Digital 12      Digital 13        1. Set High for forward or low for backward
Brake             Digital 9       Digital 8         2. Set high to engage, low to disengage
Speed (PWM)       Analog 3        Analog 11        3. Write PWM pulse to control speed (I will map my input from iBus serial connection to FS-iA6B)
Current Sensing   Analog 0        Analog 1
 */


#include <IBusBM.h>
IBusBM IBus;
int motorDirectionA = 12;
int motorDirectionB = 13;
int motorBrakeA = 9;
int motorBrakeB = 8;
int motorSpeedA = 3;
int motorSpeedB = 11;

void setup() {
  // iterate over the notes of the melody.
  // Remember, the array is twice the number of notes (notes + durations)
  for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {

    // calculates the duration of each note
    divider = melody[thisNote + 1];
    if (divider > 0) {
      // regular note, just proceed
      noteDuration = (wholenote) / divider;
    } else if (divider < 0) {
      // dotted notes are represented with negative durations!!
      noteDuration = (wholenote) / abs(divider);
      noteDuration *= 1.5; // increases the duration in half for dotted notes
    }

    // we only play the note for 90% of the duration, leaving 10% as a pause
    tone(buzzer, melody[thisNote], noteDuration * 0.9);

    // Wait for the specief duration before playing the next note.
    delay(noteDuration);

    // stop the waveform generation before the next note.
    noTone(buzzer);
  }
  Serial.begin(115200);
  IBus.begin(Serial1);    // iBUS connected to Serial0 - change to Serial1 or Serial2 port when required
  
  //Setup Channel A
  pinMode(motorDirectionA, OUTPUT); // Initiates Direction Channel A pin
  pinMode(motorBrakeA, OUTPUT);     // Initiates Brake Channel A pin
  pinMode(motorSpeedA, OUTPUT);     // Initiates Motor Channel A pin

  //Setup Channel B
  pinMode(motorDirectionB, OUTPUT); // Initiates Direction Channel B pin
  pinMode(motorBrakeB, OUTPUT);     // Initiates Brake Channel B pin
  pinMode(motorSpeedB, OUTPUT);     // Initiates Motor Channel B pin
}

int savevalmotor = 0;
int valmotor = 0;
int motor_speed;
int valturn = 0;
int savevalturn = 0;
int rightturn;
int rightturnspin;
int leftturn;
int leftturnspin;

void loop(){

  valmotor = IBus.readChannel(2); // get latest value for servo channel 8
  Serial.print("valmotor: ");
  Serial.println(valmotor);
  motor_speed = map(valmotor, 1000, 2000, 0, 255);
  valturn = IBus.readChannel(0);
  rightturn = map(valturn, 1500, 2000, 0, 160);
  leftturn = map(valturn, 1500, 1000, 0, 160);
  rightturnspin = map(valturn, 1500, 2000, 0, 255);
  leftturnspin = map(valturn, 1500, 1000, 0, 255);
  int reverse = IBus.readChannel(4);
    
  if (savevalmotor != valmotor) {
    if(reverse > 1100) {
      Serial.println("Reverse");
      digitalWrite(motorDirectionA, LOW);
      digitalWrite(motorDirectionB, HIGH);
    }else{
      Serial.println("Forward");
      digitalWrite(motorDirectionA, HIGH);
      digitalWrite(motorDirectionB, LOW);
    }
    if (valmotor != 1000 && valturn > 1500) {
      analogWrite(motorSpeedA, motor_speed);
      analogWrite(motorSpeedB, (motor_speed - rightturn));
    }
    if (valmotor != 1000 && valturn < 1500) {
      analogWrite(motorSpeedA, (motor_speed - leftturn));
      analogWrite(motorSpeedB, motor_speed);
      
    }else {
      analogWrite(motorSpeedA, motor_speed);
      analogWrite(motorSpeedB, motor_speed);
      delay(5);
    }
  }  
  if (valmotor == 1000 && valturn > 1500 && savevalturn != valturn) {
    digitalWrite(motorDirectionA, HIGH);
    analogWrite(motorSpeedA, rightturnspin);
    digitalWrite(motorDirectionB, HIGH);
    analogWrite(motorSpeedB, rightturnspin);
  }
  if (valmotor == 1000 && valturn < 1500 && savevalturn != valturn) {
    digitalWrite(motorDirectionA, LOW);
    analogWrite(motorSpeedA, leftturnspin);
    digitalWrite(motorDirectionB, LOW);
    analogWrite(motorSpeedB, leftturnspin);
  }
    
  
  savevalmotor = valmotor;
  savevalturn = valturn;
  delay(5);
}
