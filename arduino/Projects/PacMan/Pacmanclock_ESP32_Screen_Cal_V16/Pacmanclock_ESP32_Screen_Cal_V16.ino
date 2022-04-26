/*  Retro Pac-Man Clock
 Author: @TechKiwiGadgets Date 08/01/2022
 
 *  Licensed as Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0)
 *  You are free to:
    Share — copy and redistribute the material in any medium or format
    Adapt — remix, transform, and build upon the material
    Under the following terms:
    Attribution — You must give appropriate credit, provide a link to the license, and indicate if changes were made. You may do so in any reasonable manner, but not in any way that suggests the licensor endorses you or your use.
    NonCommercial — You may not use the material for commercial purposes.
    ShareAlike — If you remix, transform, or build upon the material, you must distribute your contributions under the same license as the original
  
 V1 - Screen Cal for ESP32 Clock Screen
 V16 - Incorporate new EEPROM save feature. Needs ESP32 Pacman Clock code V68 and above to work
 */

#include "TFT_eSPI.h"
#include <TouchScreen.h>
#include "Free_Fonts.h" // Include the header file attached to this sketch

#include <WiFi.h>
//#include <EEPROM.h>
#include <TimeLib.h>  
//#include <Wire.h>  
#include <pgmspace.h>
#include <EEPROM.h>



#define SCREENWIDTH 320
#define SCREENHEIGHT 240
#define TFT_GREY 0xC618




#define YP 14  // must be an analog pin, use "An" notation!
#define XM 27  // must be an analog pin, use "An" notation!
#define YM 4   // can be a digital pin
#define XP 16  // can be a digital pin

#define MINPRESSURE -1500
#define MAXPRESSURE 0

#define EEPROM_SIZE 48 // 48 EEPROM bytes required to store 24 Integer values

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

boolean debounce = false; 

byte clockhour; // Holds current hour value
byte clockminute; // Holds current minute value

// Alarm Variables
boolean alarmstatus = false; // flag where false is off and true is on
boolean soundalarm = false; // Flag to indicate the alarm needs to be initiated
int alarmhour = 0;  // hour of alarm setting
int alarmminute = 0; // Minute of alarm setting
byte ahour; //Byte variable for hour
byte amin; //Byte variable for minute
int actr = 3000; // When alarm sounds this is a counter used to reset sound card until screen touched
int act = 0;

boolean mspacman = false;  //  if this is is set to true then play the game as Ms Pac-man



//Dot Array - There are 72 Dots with 4 of them that will turn Ghost Blue!

byte dot[73]; // Where if dot is zero then has been gobbled by Pac-Man



// Touch screen coordinates
boolean screenPressed = false;
int xT,yT;
int userT = 4; // flag to indicate directional touch on screen
boolean setupscreen = false; // used to access the setup screen
boolean callibratenow = false; // Flag used to loop until screen touched for 2 seconds

// Fruit flags
boolean fruitgone = false;
boolean fruitdrawn = false;
boolean fruiteatenpacman = false;

//Pacman & Ghost kill flags
boolean pacmanlost = false;
boolean ghostlost = false;

//Alarm setup variables
boolean xsetup = false; // Flag to determine if existing setup mode

// Scorecard
int pacmanscore = 0;
int ghostscore = 0;

int userspeedsetting = 1; // user can set normal, fast, crazy speed for the pacman animation

int gamespeed = 22; // Delay setting in mS for game default is 18
int cstep = 2; // Provides the resolution of the movement for the Ghost and Pacman character. Origially set to 2

// Animation delay to slow movement down
int dly = gamespeed; // Orignally 30


// Time Refresh counter 
int rfcvalue = 900; // wait this long untiul check time for changes
int rfc = 1;

// Pacman coordinates of top LHS of 28x28 bitmap
int xP = 4;
int yP = 108;
int P = 0;  // Pacman Graphic Flag 0 = Closed, 1 = Medium Open, 2 = Wide Open, 3 = Medium Open
int D = 0;  // Pacman direction 0 = right, 1 = down, 2 = left, 3 = up
int prevD;  // Capture legacy direction to enable adequate blanking of trail
int direct;   //  Random direction variable

// Graphics Drawing Variables

int Gposition = 0; // pointer to the undsguned short arraysholding bitmap of 784 pixels 28 x 28

// Ghost coordinates of top LHS of 28x28 bitmap
int xG = 288;
int yG = 108;
int GD = 2;  // Ghost direction 0 = right, 1 = down, 2 = left, 3 = up
int prevGD;  // Capture legacy direction to enable adequate blanking of trail
int gdirect;   //  Random direction variable 

// Declare global variables for previous time, to enable refesh of only digits that have changed


TFT_eSPI myGLCD = TFT_eSPI();       // Invoke custom library

unsigned long runTime = 0;


// Smoothing Variables for LPF
const float alpha = 0.8; 
float smoothedvalueZ = 900; // This is the Z touch input value which seems to vay from 900 to 0 when pressed. 
int pressureX = 0;
int pressureY = 0;
int pressureZ = 0;
int pthreshold = 600; // This is the trigger for taking user input seriously
/* Using this smoothing Algorithm
  if (pressureZ <= pthreshold) {
    smoothedvalueZ = (alpha * smoothedvalueZ) + ( (1 - alpha) * pressureZ);
    }
*/


// Touchscreen Callibration Coordinates
int tvar = 150; // This number used for + and - boundaries based on callibration

// Create an Array for each touch coordinate with [0] = X, [1] = Y, [2] = Callibrate Flag where 0 = No and 1 = Yes

int At[3] = {650,2450,0};    // Alarm Hour increment Button
int Bt[3] = {375,3097,0};    // Alarm Minute increment Button
int Dt[3] = {1300,1998,0};   // Alarm Hour decrement Button
int Et[3] = {900,2800,0};     // Alarm Minute decrement Button

int It[3] = {500,2600,0};    // Pacman Up
int Jt[3] = {1460,1860,0};   // Pacman Left
int Kt[3] = {850,3150,0};    // Pacman Right
int Ht[3] = {1400,2120,0};   // Pacman Down

int Ct[3] = {1120,3000,0};   // Exit screen
int Ft[3] = {1900,1650,0};   // Alarm Set/Off button and speed button
int Lt[3] = {270,3330,0};    // Alarm Menu button
int Gt[3] = {989,2390,0};    // Setup Menu and Change Pacman character


boolean testburn = false;


void setup() {


// Randomseed will shuffle the random function
randomSeed(analogRead(0));

  // Initiate display
// Setup the LCD
  myGLCD.begin();
  myGLCD.setRotation(1); // Inverted to accomodate USB cable
  myGLCD.fillScreen(TFT_BLACK);

  EEPROM.begin(EEPROM_SIZE); // Initialize the EEPROM with the predefined size
  
  Serial.begin(115200);




}


void loop() {



if (callibratenow == false) {
  startcallibrate(); // Start the Callibrate Process  

//  drawresults();
  delay(2000);

  Acal(); // Callibrate the A postion
  Bcal(); // Callibrate the B postion  
  Ccal(); // Callibrate the C postion 
  Dcal(); // Callibrate the D postion 
  Ecal(); // Callibrate the E postion 
  Fcal(); // Callibrate the F postion   
  Gcal(); // Callibrate the G postion
  Hcal(); // Callibrate the H postion  
  Ical(); // Callibrate the I postion 
  Jcal(); // Callibrate the J postion 
  Kcal(); // Callibrate the K postion 
  Lcal(); // Callibrate the L postion 

  saveeepromdata(); // Routine to check and save to EEprom

}


/*
   // Alarm Set buttons
    myGLCD.setTextSize(3);
    myGLCD.drawString("A       B", 107, 20);  // A = Hour +         B = Minute +
    myGLCD.drawString("D       E", 107, 110); // D = Hour -         E = Minute -

      myGLCD.setTextSize(2);
      myGLCD.drawString(" C ", 252, 208);    // Exit screen
//      myGLCD.drawRoundRect(240 , 200 , 70  , 30  , 2 , TFT_YELLOW);


    // Draw Alarm Status
      myGLCD.setTextSize(2);

      myGLCD.drawString("F", 20, 208); // Alarm Set/Off button and speed button
 
//      myGLCD.drawRoundRect(10 , 200 , 130  , 30  , 2 , TFT_YELLOW);
      myGLCD.drawString("G", 155, 106); // Centre of screen

      myGLCD.drawString("H", 155, 208); // Pacman down
      myGLCD.drawString("I", 155, 20);  // Pacman up
      myGLCD.drawString("J", 10, 106); // Pacman left
      myGLCD.drawString("K", 300, 106); // Pacman right

      myGLCD.drawString("L", 280, 15); // Alarm Menu button

*/




  delay(1000);
  
  //drawresults(); // Draw results on screen
  
  delay(2000);
  
  if (testburn == false) {
    writetoeeprom();
  } 

}






void Acal() { // Callibration Routine for Button A position

  // Clear the screen
    myGLCD.fillScreen(TFT_BLACK);
    myGLCD.drawRoundRect(0, 0, 319, 239, 2, TFT_BLUE); // X,Y location then X,Y Size 
    myGLCD.drawRoundRect(2, 2, 315, 235, 2, TFT_BLUE); 

  // Firstly display the target in the screen as an "A"

    myGLCD.setTextColor(TFT_WHITE,TFT_BLACK); 
    myGLCD.setTextSize(3);
    myGLCD.drawString("+        ", 107, 20);  // A = Hour +         B = Minute +

    myGLCD.setTextSize(1); 
    myGLCD.drawString("Press and hold the '+' to callibrate", 40, 180);

    // Stay in a loop until user presses screen for 2 seconds
    
    int calpoint = 0;  // temporary variable counts the number of times touchscreen is reading

    while (calpoint < 3) {
    
          // Stay in loop for 3 consecutive good reads of touchscreen data
          TSPoint p = ts.getPoint();
          pinMode(XM, OUTPUT);
          pinMode(YP, OUTPUT);    
          pressureZ = p.z;
          pressureX = p.x+3000;
          pressureY = p.y+3000; 
       
            if (pressureZ < pthreshold) { // Break out of loop when user ready to callibrate    
              delay(500);
              calpoint++; // Increment counter                 
               } else {
                  calpoint = 0; // reset counter
               } 
        
          }
 /* 
          myGLCD.setTextColor(TFT_RED,TFT_BLACK);         
          myGLCD.setTextSize(1);
  
          myGLCD.drawString("X", 20, 10);
          myGLCD.drawString("Y", 20, 35);      
                  
          myGLCD.drawNumber(pressureX, 40, 10);    
          myGLCD.drawNumber(pressureY, 40, 35);    
  
         myGLCD.setTextColor(TFT_WHITE,TFT_BLACK); 
    
        delay(300); // Delay to display x and y values
 */       
        // Clear screen and draw border
        myGLCD.fillScreen(TFT_BLACK);
        myGLCD.drawRoundRect(0, 0, 319, 239, 2, TFT_BLUE); // X,Y location then X,Y Size 
        myGLCD.drawRoundRect(2, 2, 315, 235, 2, TFT_BLUE); 
        
        // Indicate complete by changing the letter to Green
        myGLCD.setTextColor(TFT_GREEN,TFT_BLACK);  
        myGLCD.setTextSize(3);
        myGLCD.drawString("+        ", 107, 20);  // A = Hour +         B = Minute +
    
        myGLCD.setTextSize(1); 
        myGLCD.drawString("Nice work, now remove touch from screen", 40, 200);
 
       // Push the values into the Array ready for saving to EEprom 
       // int At[3] = {650,2450,0};    // Alarm Hour increment Button        
       At[0] = pressureX;   // Save X coordinate
       At[1] = pressureY;   // Save Y coordinate       
       At[2] = 1;           // Set flag indicating this screen position callibrated
       delay(2000);
    
}



void drawresults() {

// ---------------- Outside wall

  myGLCD.fillScreen(TFT_BLACK);
  myGLCD.drawRoundRect(0, 0, 319, 239, 2, TFT_BLUE); // X,Y location then X,Y Size 
  myGLCD.drawRoundRect(2, 2, 315, 235, 2, TFT_BLUE); 

  myGLCD.setTextSize(1); myGLCD.setTextColor(TFT_WHITE,TFT_BLACK); 
  myGLCD.drawString("A", 5, 5);   myGLCD.drawNumber(At[0], 15, 5);   myGLCD.drawNumber(At[1], 50,   5); myGLCD.drawNumber(At[2], 80,   5);  myGLCD.drawNumber(readIntFromEEPROM(0),  120,  5);  myGLCD.drawNumber(readIntFromEEPROM(2),  180,   5);
  myGLCD.drawString("B", 5, 25);  myGLCD.drawNumber(Bt[0], 15, 25);  myGLCD.drawNumber(Bt[1], 50,  25); myGLCD.drawNumber(Bt[2], 80,  25);  myGLCD.drawNumber(readIntFromEEPROM(4),  120,  25); myGLCD.drawNumber(readIntFromEEPROM(6),  180,  25);
  myGLCD.drawString("C", 5, 45);  myGLCD.drawNumber(Ct[0], 15, 45);  myGLCD.drawNumber(Ct[1], 50,  45); myGLCD.drawNumber(Ct[2], 80,  45);  myGLCD.drawNumber(readIntFromEEPROM(8),  120,  45); myGLCD.drawNumber(readIntFromEEPROM(10), 180,  45);
  myGLCD.drawString("D", 5, 65);  myGLCD.drawNumber(Dt[0], 15, 65);  myGLCD.drawNumber(Dt[1], 50,  65); myGLCD.drawNumber(Dt[2], 80,  65);  myGLCD.drawNumber(readIntFromEEPROM(12), 120,  65); myGLCD.drawNumber(readIntFromEEPROM(14), 180,  65);
  myGLCD.drawString("E", 5, 85);  myGLCD.drawNumber(Et[0], 15, 85);  myGLCD.drawNumber(Et[1], 50,  85); myGLCD.drawNumber(Et[2], 80,  85);  myGLCD.drawNumber(readIntFromEEPROM(16), 120,  85); myGLCD.drawNumber(readIntFromEEPROM(18), 180,  85);
  myGLCD.drawString("F", 5, 105); myGLCD.drawNumber(Ft[0], 15, 105); myGLCD.drawNumber(Ft[1], 50, 105); myGLCD.drawNumber(Ft[2], 80, 105);  myGLCD.drawNumber(readIntFromEEPROM(20), 120, 105); myGLCD.drawNumber(readIntFromEEPROM(22), 180, 105);
  myGLCD.drawString("G", 5, 125); myGLCD.drawNumber(Gt[0], 15, 125); myGLCD.drawNumber(Gt[1], 50, 125); myGLCD.drawNumber(Gt[2], 80, 125);  myGLCD.drawNumber(readIntFromEEPROM(24), 120, 125); myGLCD.drawNumber(readIntFromEEPROM(26), 180, 125);
  myGLCD.drawString("H", 5, 145); myGLCD.drawNumber(Ht[0], 15, 145); myGLCD.drawNumber(Ht[1], 50, 145); myGLCD.drawNumber(Ht[2], 80, 145);  myGLCD.drawNumber(readIntFromEEPROM(28), 120, 145); myGLCD.drawNumber(readIntFromEEPROM(30), 180, 145);
  myGLCD.drawString("I", 5, 165); myGLCD.drawNumber(It[0], 15, 165); myGLCD.drawNumber(It[1], 50, 165); myGLCD.drawNumber(It[2], 80, 165);  myGLCD.drawNumber(readIntFromEEPROM(32), 120, 165); myGLCD.drawNumber(readIntFromEEPROM(34), 180, 165);
  myGLCD.drawString("J", 5, 185); myGLCD.drawNumber(Jt[0], 15, 185); myGLCD.drawNumber(Jt[1], 50, 185); myGLCD.drawNumber(Jt[2], 80, 185);  myGLCD.drawNumber(readIntFromEEPROM(36), 120, 185); myGLCD.drawNumber(readIntFromEEPROM(38), 180, 185);
  myGLCD.drawString("K", 5, 205); myGLCD.drawNumber(Kt[0], 15, 205); myGLCD.drawNumber(Kt[1], 50, 205); myGLCD.drawNumber(Kt[2], 80, 205);  myGLCD.drawNumber(readIntFromEEPROM(40), 120, 205); myGLCD.drawNumber(readIntFromEEPROM(42), 180, 205);
  myGLCD.drawString("L", 5, 225); myGLCD.drawNumber(Lt[0], 15, 225); myGLCD.drawNumber(Lt[1], 50, 225); myGLCD.drawNumber(Lt[2], 80, 225);  myGLCD.drawNumber(readIntFromEEPROM(44), 120, 225); myGLCD.drawNumber(readIntFromEEPROM(46), 180, 225);


// setup necessary button positions and give them a variable name
/*
   // Alarm Set buttons
    myGLCD.setTextSize(3);
    myGLCD.drawString("A       B", 107, 20);  // A = Hour +         B = Minute +
    myGLCD.drawString("D       E", 107, 110); // D = Hour -         E = Minute -

      myGLCD.setTextSize(2);
      myGLCD.drawString(" C ", 252, 208);    // Exit screen
//      myGLCD.drawRoundRect(240 , 200 , 70  , 30  , 2 , TFT_YELLOW);


    // Draw Alarm Status
      myGLCD.setTextSize(2);

      myGLCD.drawString("F", 20, 208); // Alarm Set/Off button and speed button
 
//      myGLCD.drawRoundRect(10 , 200 , 130  , 30  , 2 , TFT_YELLOW);
      myGLCD.drawString("G", 155, 106); // Centre of screen

      myGLCD.drawString("H", 155, 208); // Pacman down
      myGLCD.drawString("I", 155, 20);  // Pacman up
      myGLCD.drawString("J", 10, 106); // Pacman left
      myGLCD.drawString("K", 300, 106); // Pacman right

      myGLCD.drawString("L", 280, 15); // Alarm Menu button

*/
}

void writeIntIntoEEPROM(int address, int number) // Enables Integer to be written in EEPROM using two consecutive Bytes
{ 
  byte byte1 = number >> 8;
  byte byte2 = number & 0xFF;
  EEPROM.write(address, byte1); 
  EEPROM.write(address + 1, byte2);
}

int readIntFromEEPROM(int address) // // Enables Integer to be read from EEPROM using two consecutive Bytes
{
  byte byte1 = EEPROM.read(address);
  byte byte2 = EEPROM.read(address + 1);
  return (byte1 << 8) + byte2;
}


void splashscreen() {

// ---------------- Outside wall

    myGLCD.fillScreen(TFT_BLACK);
    myGLCD.drawRoundRect(0, 0, 319, 239, 2, TFT_BLUE); // X,Y location then X,Y Size 
    myGLCD.drawRoundRect(2, 2, 315, 235, 2, TFT_BLUE); 

// setup necessary button positions and give them a variable name
    myGLCD.setTextSize(2);
    myGLCD.drawString("ESP 32 Pacman Clock", 50, 50);  
    myGLCD.setTextSize(1);       
    myGLCD.drawString("Screen Callibration Tool", 85, 90);    myGLCD.setTextColor(TFT_RED,TFT_BLACK); 
    myGLCD.drawString("Touch Screen for 4 seconds to start", 55, 110);     myGLCD.setTextColor(TFT_WHITE,TFT_BLACK); 
    myGLCD.drawString("by TechKiwiGadgets 2022", 85, 210); 




}


void startcallibrate() {

  splashscreen();

// Stay in a loop until user presses screen for 2 seconds


    while (callibratenow == false) {
    
       // Read the Touch Screen Locations *******
       TSPoint p = ts.getPoint();
       pinMode(XM, OUTPUT);
       pinMode(YP, OUTPUT);    
       pressureZ = p.z;

/*
       myGLCD.setTextColor(TFT_RED,TFT_BLACK);         
       myGLCD.setTextSize(1);
       myGLCD.drawString("Z", 10, 125);                          
       myGLCD.drawNumber(pressureZ, 40, 125);
*/

       delay(500);
       myGLCD.setTextColor(TFT_WHITE,TFT_BLACK); 

        
         if (pressureZ < pthreshold) { // Break out of loop when user ready to callibrate    
            delay(2000);
        
            // Debounce by Reading again the Touch Screen Locations
            TSPoint p = ts.getPoint();
            
              // if sharing pins, you'll need to fix the directions of the touchscreen pins
             pinMode(XM, OUTPUT);
             pinMode(YP, OUTPUT);   
            
                if (pressureZ < pthreshold) { // Break out of loop when user ready to callibrate   
                  callibratenow = true;    
               }
        
          }
    
    }

}


void Bcal() { // Callibration Routine for Button B position

  // Clear the screen
    myGLCD.fillScreen(TFT_BLACK);
    myGLCD.drawRoundRect(0, 0, 319, 239, 2, TFT_BLUE); // X,Y location then X,Y Size 
    myGLCD.drawRoundRect(2, 2, 315, 235, 2, TFT_BLUE); 

  // Firstly display the target in the screen as an "A"

    myGLCD.setTextColor(TFT_WHITE,TFT_BLACK); 
    myGLCD.setTextSize(3);
//    myGLCD.drawString("        B", 107, 20);  // A = Hour +         B = Minute +
    myGLCD.drawString("        +", 107, 20);  // A = Hour +         B = Minute +

    
    myGLCD.setTextSize(1); 
    myGLCD.drawString("Press and hold the '+' to callibrate", 40, 180);

    // Stay in a loop until user presses screen for 2 seconds
    
    int calpoint = 0;  // temporary variable counts the number of times touchscreen is reading

    while (calpoint < 3) {
    
          // Stay in loop for 3 consecutive good reads of touchscreen data
          TSPoint p = ts.getPoint();
          pinMode(XM, OUTPUT);
          pinMode(YP, OUTPUT);    
          pressureZ = p.z;
          pressureX = p.x+3000;
          pressureY = p.y+3000; 
                    
            if (pressureZ < pthreshold) { // Break out of loop when user ready to callibrate    
              delay(500);
              calpoint++; // Increment counter                 
               } else {
                  calpoint = 0; // reset counter
               } 
        
          }
/*
              myGLCD.setTextColor(TFT_RED,TFT_BLACK);         
              myGLCD.setTextSize(1);
      
              myGLCD.drawString("X", 20, 10);
              myGLCD.drawString("Y", 20, 35);      
                      
              myGLCD.drawNumber(pressureX, 40, 10);    
              myGLCD.drawNumber(pressureY, 40, 35);    
             myGLCD.setTextColor(TFT_WHITE,TFT_BLACK); 

        delay(300); // Delay to display x and y values
*/
        // Clear screen and draw border
        myGLCD.fillScreen(TFT_BLACK);
        myGLCD.drawRoundRect(0, 0, 319, 239, 2, TFT_BLUE); // X,Y location then X,Y Size 
        myGLCD.drawRoundRect(2, 2, 315, 235, 2, TFT_BLUE); 
        
        // Indicate complete by changing the letter to Green
        myGLCD.setTextColor(TFT_GREEN,TFT_BLACK);  
        myGLCD.setTextSize(3);
        myGLCD.drawString("        +", 107, 20);  // A = Hour +         B = Minute +
    
        myGLCD.setTextSize(1); 
        myGLCD.drawString("Nice work, now remove touch from screen", 40, 200);
 
       // Push the values into the Array ready for saving to EEprom 
       // int At[3] = {650,2450,0};    // Alarm Hour increment Button        
       Bt[0] = pressureX;   // Save X coordinate
       Bt[1] = pressureY;   // Save Y coordinate       
       Bt[2] = 1;           // Set flag indicating this screen position callibrated
       delay(2000);
    
}

void Ccal() { // Callibration Routine for Button C position       myGLCD.drawString(" C ", 252, 208);    // Exit screen

  // Clear the screen
    myGLCD.fillScreen(TFT_BLACK);
    myGLCD.drawRoundRect(0, 0, 319, 239, 2, TFT_BLUE); // X,Y location then X,Y Size 
    myGLCD.drawRoundRect(2, 2, 315, 235, 2, TFT_BLUE); 

  // Firstly display the target in the screen as an "A"

    myGLCD.setTextColor(TFT_WHITE,TFT_BLACK); 
    myGLCD.setTextSize(2);
    myGLCD.drawString(" + ", 252, 208);

    myGLCD.setTextSize(1); 
    myGLCD.drawString("Press and hold the '+' to callibrate", 40, 180);

    // Stay in a loop until user presses screen for 2 seconds
    
    int calpoint = 0;  // temporary variable counts the number of times touchscreen is reading

    while (calpoint < 3) {
    
          // Stay in loop for 3 consecutive good reads of touchscreen data
          TSPoint p = ts.getPoint();
          pinMode(XM, OUTPUT);
          pinMode(YP, OUTPUT);    
          pressureZ = p.z;
          pressureX = p.x+3000;
          pressureY = p.y+3000; 
                    
            if (pressureZ < pthreshold) { // Break out of loop when user ready to callibrate    
              delay(500);
              calpoint++; // Increment counter                 
               } else {
                  calpoint = 0; // reset counter
               } 
        
          }
/*
              myGLCD.setTextColor(TFT_RED,TFT_BLACK);         
              myGLCD.setTextSize(1);
      
              myGLCD.drawString("X", 20, 10);
              myGLCD.drawString("Y", 20, 35);      
                      
              myGLCD.drawNumber(pressureX, 40, 10);    
              myGLCD.drawNumber(pressureY, 40, 35);    
              
             myGLCD.setTextColor(TFT_WHITE,TFT_BLACK); 

        delay(300); // Delay to display x and y values
*/    
        // Clear screen and draw border
        myGLCD.fillScreen(TFT_BLACK);
        myGLCD.drawRoundRect(0, 0, 319, 239, 2, TFT_BLUE); // X,Y location then X,Y Size 
        myGLCD.drawRoundRect(2, 2, 315, 235, 2, TFT_BLUE); 
        
        // Indicate complete by changing the letter to Green
        myGLCD.setTextColor(TFT_GREEN,TFT_BLACK);  
        myGLCD.setTextSize(2);
        myGLCD.drawString(" + ", 252, 208);
    
        myGLCD.setTextSize(1); 
        myGLCD.drawString("Nice work, now remove touch from screen", 40, 200);
 
       // Push the values into the Array ready for saving to EEprom 
       // int At[3] = {650,2450,0};    // Alarm Hour increment Button        
       Ct[0] = pressureX;   // Save X coordinate
       Ct[1] = pressureY;   // Save Y coordinate       
       Ct[2] = 1;           // Set flag indicating this screen position callibrated
       delay(2000);
    
}

void Dcal() { // Callibration Routine for Button D position          myGLCD.drawString("D       E", 107, 110); // D = Hour -         E = Minute -

  // Clear the screen
    myGLCD.fillScreen(TFT_BLACK);
    myGLCD.drawRoundRect(0, 0, 319, 239, 2, TFT_BLUE); // X,Y location then X,Y Size 
    myGLCD.drawRoundRect(2, 2, 315, 235, 2, TFT_BLUE); 

  // Firstly display the target in the screen as an "A"

    myGLCD.setTextColor(TFT_WHITE,TFT_BLACK); 
    myGLCD.setTextSize(3);
    myGLCD.drawString("+", 107, 110); // D = Hour -    

    myGLCD.setTextSize(1); 
    myGLCD.drawString("Press and hold the '+' to callibrate", 40, 180);

    // Stay in a loop until user presses screen for 2 seconds
    
    int calpoint = 0;  // temporary variable counts the number of times touchscreen is reading

    while (calpoint < 3) {
    
          // Stay in loop for 3 consecutive good reads of touchscreen data
          TSPoint p = ts.getPoint();
          pinMode(XM, OUTPUT);
          pinMode(YP, OUTPUT);    
          pressureZ = p.z;
          pressureX = p.x+3000;
          pressureY = p.y+3000; 
                    
            if (pressureZ < pthreshold) { // Break out of loop when user ready to callibrate    
              delay(500);
              calpoint++; // Increment counter                 
               } else {
                  calpoint = 0; // reset counter
               } 
        
          }
/*
              myGLCD.setTextColor(TFT_RED,TFT_BLACK);         
              myGLCD.setTextSize(1);
      
              myGLCD.drawString("X", 20, 10);
              myGLCD.drawString("Y", 20, 35);      
                      
              myGLCD.drawNumber(pressureX, 40, 10);    
              myGLCD.drawNumber(pressureY, 40, 35);    
             myGLCD.setTextColor(TFT_WHITE,TFT_BLACK); 

        delay(300); // Delay to dislay x and y values
*/           
        // Clear screen and draw border
        myGLCD.fillScreen(TFT_BLACK);
        myGLCD.drawRoundRect(0, 0, 319, 239, 2, TFT_BLUE); // X,Y location then X,Y Size 
        myGLCD.drawRoundRect(2, 2, 315, 235, 2, TFT_BLUE); 
        
        // Indicate complete by changing the letter to Green
        myGLCD.setTextColor(TFT_GREEN,TFT_BLACK);  
        myGLCD.setTextSize(3);
        myGLCD.drawString("+", 107, 110); // D = Hour -  
    
        myGLCD.setTextSize(1); 
        myGLCD.drawString("Nice work, now remove touch from screen", 40, 200);
 
       // Push the values into the Array ready for saving to EEprom 
       // int At[3] = {650,2450,0};    // Alarm Hour increment Button        
       Dt[0] = pressureX;   // Save X coordinate
       Dt[1] = pressureY;   // Save Y coordinate       
       Dt[2] = 1;           // Set flag indicating this screen position callibrated
       delay(2000);
    
}

void Ecal() { // Callibration Routine for Button E position              myGLCD.drawString("D       E", 107, 110); // D = Hour -         E = Minute -

  // Clear the screen
    myGLCD.fillScreen(TFT_BLACK);
    myGLCD.drawRoundRect(0, 0, 319, 239, 2, TFT_BLUE); // X,Y location then X,Y Size 
    myGLCD.drawRoundRect(2, 2, 315, 235, 2, TFT_BLUE); 

  // Firstly display the target in the screen as an "A"

    myGLCD.setTextColor(TFT_WHITE,TFT_BLACK); 
    myGLCD.setTextSize(3);
    myGLCD.drawString("        +", 107, 110); // E = Minute - 

    myGLCD.setTextSize(1); 
    myGLCD.drawString("Press and hold the '+' to callibrate", 40, 180);

    // Stay in a loop until user presses screen for 2 seconds
    
    int calpoint = 0;  // temporary variable counts the number of times touchscreen is reading

    while (calpoint < 3) {
    
          // Stay in loop for 3 consecutive good reads of touchscreen data
          TSPoint p = ts.getPoint();
          pinMode(XM, OUTPUT);
          pinMode(YP, OUTPUT);    
          pressureZ = p.z;
          pressureX = p.x+3000;
          pressureY = p.y+3000; 
                    
            if (pressureZ < pthreshold) { // Break out of loop when user ready to callibrate    
              delay(500);
              calpoint++; // Increment counter                 
               } else {
                  calpoint = 0; // reset counter
               } 
        
          }
/*
              myGLCD.setTextColor(TFT_RED,TFT_BLACK);         
              myGLCD.setTextSize(1);
      
              myGLCD.drawString("X", 20, 10);
              myGLCD.drawString("Y", 20, 35);      
                      
              myGLCD.drawNumber(pressureX, 40, 10);    
              myGLCD.drawNumber(pressureY, 40, 35);    
             myGLCD.setTextColor(TFT_WHITE,TFT_BLACK); 

        delay(300); // Delay to dislay x and y values
 */          
        // Clear screen and draw border
        myGLCD.fillScreen(TFT_BLACK);
        myGLCD.drawRoundRect(0, 0, 319, 239, 2, TFT_BLUE); // X,Y location then X,Y Size 
        myGLCD.drawRoundRect(2, 2, 315, 235, 2, TFT_BLUE); 
        
        // Indicate complete by changing the letter to Green
        myGLCD.setTextColor(TFT_GREEN,TFT_BLACK);  
        myGLCD.setTextSize(3);
        myGLCD.drawString("        +", 107, 110); // E = Minute - 
    
        myGLCD.setTextSize(1); 
        myGLCD.drawString("Nice work, now remove touch from screen", 40, 200);
 
       // Push the values into the Array ready for saving to EEprom 
       // int At[3] = {650,2450,0};    // Alarm Hour increment Button        
       Et[0] = pressureX;   // Save X coordinate
       Et[1] = pressureY;   // Save Y coordinate       
       Et[2] = 1;           // Set flag indicating this screen position callibrated
       delay(2000);
    
}

void Fcal() { // Callibration Routine for Button F position      myGLCD.drawString("F", 20, 208); // Alarm Set/Off button and speed button

  // Clear the screen
    myGLCD.fillScreen(TFT_BLACK);
    myGLCD.drawRoundRect(0, 0, 319, 239, 2, TFT_BLUE); // X,Y location then X,Y Size 
    myGLCD.drawRoundRect(2, 2, 315, 235, 2, TFT_BLUE); 

  // Firstly display the target in the screen as an "A"

    myGLCD.setTextColor(TFT_WHITE,TFT_BLACK); 
    myGLCD.setTextSize(2);
    myGLCD.drawString("+", 20, 208); // Alarm Set/Off button and speed button

    myGLCD.setTextSize(1); 
    myGLCD.drawString("Press and hold the '+' to callibrate", 40, 180);

    // Stay in a loop until user presses screen for 2 seconds
    
    int calpoint = 0;  // temporary variable counts the number of times touchscreen is reading

    while (calpoint < 3) {
    
          // Stay in loop for 3 consecutive good reads of touchscreen data
          TSPoint p = ts.getPoint();
          pinMode(XM, OUTPUT);
          pinMode(YP, OUTPUT);    
          pressureZ = p.z;
          pressureX = p.x+3000;
          pressureY = p.y+3000; 
                    
            if (pressureZ < pthreshold) { // Break out of loop when user ready to callibrate    
              delay(500);
              calpoint++; // Increment counter                 
               } else {
                  calpoint = 0; // reset counter
               } 
        
          }
/*
              myGLCD.setTextColor(TFT_RED,TFT_BLACK);         
              myGLCD.setTextSize(1);
      
              myGLCD.drawString("X", 20, 10);
              myGLCD.drawString("Y", 20, 35);      
                      
              myGLCD.drawNumber(pressureX, 40, 10);    
              myGLCD.drawNumber(pressureY, 40, 35);    
             myGLCD.setTextColor(TFT_WHITE,TFT_BLACK); 

        delay(300); // Delay to dislay x and y values
*/           
        // Clear screen and draw border
        myGLCD.fillScreen(TFT_BLACK);
        myGLCD.drawRoundRect(0, 0, 319, 239, 2, TFT_BLUE); // X,Y location then X,Y Size 
        myGLCD.drawRoundRect(2, 2, 315, 235, 2, TFT_BLUE); 
        
        // Indicate complete by changing the letter to Green
        myGLCD.setTextColor(TFT_GREEN,TFT_BLACK);  
        myGLCD.setTextSize(2);
        myGLCD.drawString("+", 20, 208); // Alarm Set/Off button and speed button
    
        myGLCD.setTextSize(1); 
        myGLCD.drawString("Nice work, now remove touch from screen", 40, 200);
 
       // Push the values into the Array ready for saving to EEprom 
       // int At[3] = {650,2450,0};    // Alarm Hour increment Button        
       Ft[0] = pressureX;   // Save X coordinate
       Ft[1] = pressureY;   // Save Y coordinate       
       Ft[2] = 1;           // Set flag indicating this screen position callibrated
       delay(2000);
    
}

void Gcal() { // Callibration Routine for Button G position      myGLCD.drawString("G", 155, 106); // Centre of screen

  // Clear the screen
    myGLCD.fillScreen(TFT_BLACK);
    myGLCD.drawRoundRect(0, 0, 319, 239, 2, TFT_BLUE); // X,Y location then X,Y Size 
    myGLCD.drawRoundRect(2, 2, 315, 235, 2, TFT_BLUE); 

  // Firstly display the target in the screen as an "A"

    myGLCD.setTextColor(TFT_WHITE,TFT_BLACK); 
    myGLCD.setTextSize(2);
    myGLCD.drawString("+", 155, 106); // Centre of screen

    myGLCD.setTextSize(1); 
    myGLCD.drawString("Press and hold the '+' to callibrate", 40, 180);

    // Stay in a loop until user presses screen for 2 seconds
    
    int calpoint = 0;  // temporary variable counts the number of times touchscreen is reading

    while (calpoint < 3) {
    
          // Stay in loop for 3 consecutive good reads of touchscreen data
          TSPoint p = ts.getPoint();
          pinMode(XM, OUTPUT);
          pinMode(YP, OUTPUT);    
          pressureZ = p.z;
          pressureX = p.x+3000;
          pressureY = p.y+3000; 
                    
            if (pressureZ < pthreshold) { // Break out of loop when user ready to callibrate    
              delay(500);
              calpoint++; // Increment counter                 
               } else {
                  calpoint = 0; // reset counter
               } 
        
          }

/*
              myGLCD.setTextColor(TFT_RED,TFT_BLACK);         
              myGLCD.setTextSize(1);
      
              myGLCD.drawString("X", 20, 10);
              myGLCD.drawString("Y", 20, 35);      
                      
              myGLCD.drawNumber(pressureX, 40, 10);    
              myGLCD.drawNumber(pressureY, 40, 35);    
             myGLCD.setTextColor(TFT_WHITE,TFT_BLACK); 

        delay(300); // Delay to dislay x and y values
*/           
        // Clear screen and draw border
        myGLCD.fillScreen(TFT_BLACK);
        myGLCD.drawRoundRect(0, 0, 319, 239, 2, TFT_BLUE); // X,Y location then X,Y Size 
        myGLCD.drawRoundRect(2, 2, 315, 235, 2, TFT_BLUE); 
        
        // Indicate complete by changing the letter to Green
        myGLCD.setTextColor(TFT_GREEN,TFT_BLACK);  
        myGLCD.setTextSize(2);
        myGLCD.drawString("+", 155, 106); // Centre of screen
    
        myGLCD.setTextSize(1); 
        myGLCD.drawString("Nice work, now remove touch from screen", 40, 200);
 
       // Push the values into the Array ready for saving to EEprom 
       // int At[3] = {650,2450,0};    // Alarm Hour increment Button        
       Gt[0] = pressureX;   // Save X coordinate
       Gt[1] = pressureY;   // Save Y coordinate       
       Gt[2] = 1;           // Set flag indicating this screen position callibrated
       delay(2000);
    
}

void Hcal() { // Callibration Routine for Button H position            myGLCD.drawString("H", 155, 208); // Pacman down

  // Clear the screen
    myGLCD.fillScreen(TFT_BLACK);
    myGLCD.drawRoundRect(0, 0, 319, 239, 2, TFT_BLUE); // X,Y location then X,Y Size 
    myGLCD.drawRoundRect(2, 2, 315, 235, 2, TFT_BLUE); 

  // Firstly display the target in the screen as an "A"

    myGLCD.setTextColor(TFT_WHITE,TFT_BLACK); 
    myGLCD.setTextSize(2);
    myGLCD.drawString("+", 155, 208); // Pacman down

    myGLCD.setTextSize(1); 
    myGLCD.drawString("Press and hold the '+' to callibrate", 40, 180);

    // Stay in a loop until user presses screen for 2 seconds
    
    int calpoint = 0;  // temporary variable counts the number of times touchscreen is reading

    while (calpoint < 3) {
    
          // Stay in loop for 3 consecutive good reads of touchscreen data
          TSPoint p = ts.getPoint();
          pinMode(XM, OUTPUT);
          pinMode(YP, OUTPUT);    
          pressureZ = p.z;
          pressureX = p.x+3000;
          pressureY = p.y+3000; 
                    
            if (pressureZ < pthreshold) { // Break out of loop when user ready to callibrate    
              delay(500);
              calpoint++; // Increment counter                 
               } else {
                  calpoint = 0; // reset counter
               } 
        
          }

/*
              myGLCD.setTextColor(TFT_RED,TFT_BLACK);         
              myGLCD.setTextSize(1);
      
              myGLCD.drawString("X", 20, 10);
              myGLCD.drawString("Y", 20, 35);      
                      
              myGLCD.drawNumber(pressureX, 40, 10);    
              myGLCD.drawNumber(pressureY, 40, 35);    
             myGLCD.setTextColor(TFT_WHITE,TFT_BLACK); 

        delay(300); // Delay to dislay x and y values
 */          
        // Clear screen and draw border
        myGLCD.fillScreen(TFT_BLACK);
        myGLCD.drawRoundRect(0, 0, 319, 239, 2, TFT_BLUE); // X,Y location then X,Y Size 
        myGLCD.drawRoundRect(2, 2, 315, 235, 2, TFT_BLUE); 
        
        // Indicate complete by changing the letter to Green
        myGLCD.setTextColor(TFT_GREEN,TFT_BLACK);  
        myGLCD.setTextSize(2);
        myGLCD.drawString("+", 155, 208); // Pacman down
    
        myGLCD.setTextSize(1); 
        myGLCD.drawString("Nice work, now remove touch from screen", 40, 200);
 
       // Push the values into the Array ready for saving to EEprom 
       // int At[3] = {650,2450,0};    // Alarm Hour increment Button        
       Ht[0] = pressureX;   // Save X coordinate
       Ht[1] = pressureY;   // Save Y coordinate       
       Ht[2] = 1;           // Set flag indicating this screen position callibrated
       delay(2000);
    
}

void Ical() { // Callibration Routine for Button I position          myGLCD.drawString("I", 155, 20);  // Pacman up

  // Clear the screen
    myGLCD.fillScreen(TFT_BLACK);
    myGLCD.drawRoundRect(0, 0, 319, 239, 2, TFT_BLUE); // X,Y location then X,Y Size 
    myGLCD.drawRoundRect(2, 2, 315, 235, 2, TFT_BLUE); 

  // Firstly display the target in the screen as an "A"

    myGLCD.setTextColor(TFT_WHITE,TFT_BLACK); 
    myGLCD.setTextSize(2);
    myGLCD.drawString("+", 155, 20);  // Pacman up

    myGLCD.setTextSize(1); 
    myGLCD.drawString("Press and hold the '+' to callibrate", 40, 180);

    // Stay in a loop until user presses screen for 2 seconds
    
    int calpoint = 0;  // temporary variable counts the number of times touchscreen is reading

    while (calpoint < 3) {
    
          // Stay in loop for 3 consecutive good reads of touchscreen data
          TSPoint p = ts.getPoint();
          pinMode(XM, OUTPUT);
          pinMode(YP, OUTPUT);    
          pressureZ = p.z;
          pressureX = p.x+3000;
          pressureY = p.y+3000; 
                    
            if (pressureZ < pthreshold) { // Break out of loop when user ready to callibrate    
              delay(500);
              calpoint++; // Increment counter                 
               } else {
                  calpoint = 0; // reset counter
               } 
        
          }

/*
              myGLCD.setTextColor(TFT_RED,TFT_BLACK);         
              myGLCD.setTextSize(1);
      
              myGLCD.drawString("X", 20, 10);
              myGLCD.drawString("Y", 20, 35);      
                      
              myGLCD.drawNumber(pressureX, 40, 10);    
              myGLCD.drawNumber(pressureY, 40, 35);    
             myGLCD.setTextColor(TFT_WHITE,TFT_BLACK); 

        delay(300); // Delay to dislay x and y values
 */          
        // Clear screen and draw border
        myGLCD.fillScreen(TFT_BLACK);
        myGLCD.drawRoundRect(0, 0, 319, 239, 2, TFT_BLUE); // X,Y location then X,Y Size 
        myGLCD.drawRoundRect(2, 2, 315, 235, 2, TFT_BLUE); 
        
        // Indicate complete by changing the letter to Green
        myGLCD.setTextColor(TFT_GREEN,TFT_BLACK);  
        myGLCD.setTextSize(2);
        myGLCD.drawString("+", 155, 20);  // Pacman up
    
        myGLCD.setTextSize(1); 
        myGLCD.drawString("Nice work, now remove touch from screen", 40, 200);
 
       // Push the values into the Array ready for saving to EEprom 
       // int At[3] = {650,2450,0};    // Alarm Hour increment Button        
       It[0] = pressureX;   // Save X coordinate
       It[1] = pressureY;   // Save Y coordinate       
       It[2] = 1;           // Set flag indicating this screen position callibrated
       delay(2000);
    
}

void Jcal() { // Callibration Routine for Button J position                myGLCD.drawString("J", 10, 106); // Pacman left

  // Clear the screen
    myGLCD.fillScreen(TFT_BLACK);
    myGLCD.drawRoundRect(0, 0, 319, 239, 2, TFT_BLUE); // X,Y location then X,Y Size 
    myGLCD.drawRoundRect(2, 2, 315, 235, 2, TFT_BLUE); 

  // Firstly display the target in the screen as an "A"

    myGLCD.setTextColor(TFT_WHITE,TFT_BLACK); 
    myGLCD.setTextSize(2);
    myGLCD.drawString("+", 10, 106); // Pacman left

    myGLCD.setTextSize(1); 
    myGLCD.drawString("Press and hold the '+' to callibrate", 40, 180);

    // Stay in a loop until user presses screen for 2 seconds
    
    int calpoint = 0;  // temporary variable counts the number of times touchscreen is reading

    while (calpoint < 3) {
    
          // Stay in loop for 3 consecutive good reads of touchscreen data
          TSPoint p = ts.getPoint();
          pinMode(XM, OUTPUT);
          pinMode(YP, OUTPUT);    
          pressureZ = p.z;
          pressureX = p.x+3000;
          pressureY = p.y+3000; 
                    
            if (pressureZ < pthreshold) { // Break out of loop when user ready to callibrate    
              delay(500);
              calpoint++; // Increment counter                 
               } else {
                  calpoint = 0; // reset counter
               } 
        
          }

/*
              myGLCD.setTextColor(TFT_RED,TFT_BLACK);         
              myGLCD.setTextSize(1);
      
              myGLCD.drawString("X", 20, 10);
              myGLCD.drawString("Y", 20, 35);      
                      
              myGLCD.drawNumber(pressureX, 40, 10);    
              myGLCD.drawNumber(pressureY, 40, 35);    
             myGLCD.setTextColor(TFT_WHITE,TFT_BLACK); 

        delay(300); // Delay to display x and y values
*/           
        // Clear screen and draw border
        myGLCD.fillScreen(TFT_BLACK);
        myGLCD.drawRoundRect(0, 0, 319, 239, 2, TFT_BLUE); // X,Y location then X,Y Size 
        myGLCD.drawRoundRect(2, 2, 315, 235, 2, TFT_BLUE); 
        
        // Indicate complete by changing the letter to Green
        myGLCD.setTextColor(TFT_GREEN,TFT_BLACK);  
        myGLCD.setTextSize(2);
        myGLCD.drawString("+", 10, 106); // Pacman left
    
        myGLCD.setTextSize(1); 
        myGLCD.drawString("Nice work, now remove touch from screen", 40, 200);
 
       // Push the values into the Array ready for saving to EEprom 
       // int At[3] = {650,2450,0};    // Alarm Hour increment Button        
       Jt[0] = pressureX;   // Save X coordinate
       Jt[1] = pressureY;   // Save Y coordinate       
       Jt[2] = 1;           // Set flag indicating this screen position callibrated
       delay(2000);
    
}

void Kcal() { // Callibration Routine for Button K position               myGLCD.drawString("K", 300, 106); // Pacman right


  // Clear the screen
    myGLCD.fillScreen(TFT_BLACK);
    myGLCD.drawRoundRect(0, 0, 319, 239, 2, TFT_BLUE); // X,Y location then X,Y Size 
    myGLCD.drawRoundRect(2, 2, 315, 235, 2, TFT_BLUE); 

  // Firstly display the target in the screen as an "A"

    myGLCD.setTextColor(TFT_WHITE,TFT_BLACK); 
    myGLCD.setTextSize(2);
    myGLCD.drawString("+", 300, 106); // Pacman right

    myGLCD.setTextSize(1); 
    myGLCD.drawString("Press and hold the '+' to callibrate", 40, 180);

    // Stay in a loop until user presses screen for 2 seconds
    
    int calpoint = 0;  // temporary variable counts the number of times touchscreen is reading

    while (calpoint < 3) {
    
          // Stay in loop for 3 consecutive good reads of touchscreen data
          TSPoint p = ts.getPoint();
          pinMode(XM, OUTPUT);
          pinMode(YP, OUTPUT);    
          pressureZ = p.z;
          pressureX = p.x+3000;
          pressureY = p.y+3000; 
                    
            if (pressureZ < pthreshold) { // Break out of loop when user ready to callibrate    
              delay(500);
              calpoint++; // Increment counter                 
               } else {
                  calpoint = 0; // reset counter
               } 
        
          }
/*
              myGLCD.setTextColor(TFT_RED,TFT_BLACK);         
              myGLCD.setTextSize(1);
      
              myGLCD.drawString("X", 20, 10);
              myGLCD.drawString("Y", 20, 35);      
                      
              myGLCD.drawNumber(pressureX, 40, 10);    
              myGLCD.drawNumber(pressureY, 40, 35);    
             myGLCD.setTextColor(TFT_WHITE,TFT_BLACK); 

        delay(300); // Delay to dislay x and y values
 */          
        // Clear screen and draw border
        myGLCD.fillScreen(TFT_BLACK);
        myGLCD.drawRoundRect(0, 0, 319, 239, 2, TFT_BLUE); // X,Y location then X,Y Size 
        myGLCD.drawRoundRect(2, 2, 315, 235, 2, TFT_BLUE); 
        
        // Indicate complete by changing the letter to Green
        myGLCD.setTextColor(TFT_GREEN,TFT_BLACK);  
        myGLCD.setTextSize(2);
        myGLCD.drawString("+", 300, 106); // Pacman right
    
        myGLCD.setTextSize(1); 
        myGLCD.drawString("Nice work, now remove touch from screen", 40, 200);
 
       // Push the values into the Array ready for saving to EEprom 
       // int At[3] = {650,2450,0};    // Alarm Hour increment Button        
       Kt[0] = pressureX;   // Save X coordinate
       Kt[1] = pressureY;   // Save Y coordinate       
       Kt[2] = 1;           // Set flag indicating this screen position callibrated
       delay(2000);
    
}

void Lcal() { // Callibration Routine for Button L position               myGLCD.drawString("L", 280, 15); // Alarm Menu button


  // Clear the screen
    myGLCD.fillScreen(TFT_BLACK);
    myGLCD.drawRoundRect(0, 0, 319, 239, 2, TFT_BLUE); // X,Y location then X,Y Size 
    myGLCD.drawRoundRect(2, 2, 315, 235, 2, TFT_BLUE); 

  // Firstly display the target in the screen as an "A"

    myGLCD.setTextColor(TFT_WHITE,TFT_BLACK); 
    myGLCD.setTextSize(2);
    myGLCD.drawString("+", 280, 15); // Alarm Menu button

    myGLCD.setTextSize(1); 
    myGLCD.drawString("Press and hold the '+' to callibrate", 40, 180);

    // Stay in a loop until user presses screen for 2 seconds
    
    int calpoint = 0;  // temporary variable counts the number of times touchscreen is reading

    while (calpoint < 3) {
    
          // Stay in loop for 3 consecutive good reads of touchscreen data
          TSPoint p = ts.getPoint();
          pinMode(XM, OUTPUT);
          pinMode(YP, OUTPUT);    
          pressureZ = p.z;
          pressureX = p.x+3000;
          pressureY = p.y+3000; 
                    
            if (pressureZ < pthreshold) { // Break out of loop when user ready to callibrate    
              delay(500);
              calpoint++; // Increment counter                 
               } else {
                  calpoint = 0; // reset counter
               } 
        
          }

/*
              myGLCD.setTextColor(TFT_RED,TFT_BLACK);         
              myGLCD.setTextSize(1);
      
              myGLCD.drawString("X", 20, 10);
              myGLCD.drawString("Y", 20, 35);      
                      
              myGLCD.drawNumber(pressureX, 40, 10);    
              myGLCD.drawNumber(pressureY, 40, 35);    
             myGLCD.setTextColor(TFT_WHITE,TFT_BLACK); 

        delay(300); // Delay to dislay x and y values
*/           
        // Clear screen and draw border
        myGLCD.fillScreen(TFT_BLACK);
        myGLCD.drawRoundRect(0, 0, 319, 239, 2, TFT_BLUE); // X,Y location then X,Y Size 
        myGLCD.drawRoundRect(2, 2, 315, 235, 2, TFT_BLUE); 
        
        // Indicate complete by changing the letter to Green
        myGLCD.setTextColor(TFT_GREEN,TFT_BLACK);  
        myGLCD.setTextSize(2);
        myGLCD.drawString("+", 280, 15); // Alarm Menu button
    
        myGLCD.setTextSize(1); 
        myGLCD.drawString("Nice work, now remove touch from screen", 40, 200);
 
       // Push the values into the Array ready for saving to EEprom 
       // int At[3] = {650,2450,0};    // Alarm Hour increment Button        
       Lt[0] = pressureX;   // Save X coordinate
       Lt[1] = pressureY;   // Save Y coordinate       
       Lt[2] = 1;           // Set flag indicating this screen position callibrated
       delay(2000);
    
}

void   savetoeeprom() { // Save touch screen variables to EEPROM in ESP32


  
}

boolean readtouchscreen(int xr1, int xr2, int yr1, int yr2 ) { // pass parameters to this loop to debounce and validate a screen touch to avoid spurious resutls

  boolean touchresult;  // Holds return value of operation
  int xnum = 0; // find average of x coordinate over samples
  int ynum = 0; // find average of y coordinate over samples
  int numsamples = 5; // Number of samples taken
  
  // Firstly quickly read the screen value 20 times then take the last value as the correct value

  TSPoint p = ts.getPoint();
        // if sharing pins, you'll need to fix the directions of the touchscreen pins
       pinMode(XM, OUTPUT);
       pinMode(YP, OUTPUT); 
       delay(2);
              
  for (int screenread = 0; screenread < numsamples; screenread++) {    
           
        // if sharing pins, you'll need to fix the directions of the touchscreen pins
       delay(2);
       p = ts.getPoint();
      
       xnum = xnum + p.x; 
       ynum = ynum + p.y;
             
       pinMode(XM, OUTPUT);
       pinMode(YP, OUTPUT);        
  }       

  xT = (xnum/numsamples) + 3000;
  yT = (ynum/numsamples) + 3000;
          
      
                Serial.print("X = "); Serial.print(xT);
                Serial.print("\tY = "); Serial.print(yT);
                Serial.println("____________________");
                 
          // Calculate result
            
          if ((xT>=xr1) && (xT<=xr2) && (yT>=yr1) && (yT<=yr2)) { //  if this condition met then retuen a TRUE value 
                touchresult = true;
           } else {
                touchresult = false;
           }
               
   return touchresult;            
} 


void saveeepromdata() { // Menu used to set the Alarm time

  // Firstly Clear Screen and confirm if want save
    myGLCD.fillScreen(TFT_BLACK);
    myGLCD.drawRoundRect(0, 0, 319, 239, 2, TFT_BLUE); // X,Y location then X,Y Size 
    myGLCD.drawRoundRect(2, 2, 315, 235, 2, TFT_BLUE); 

  // Ask if you want to save
    myGLCD.setTextColor(TFT_BLUE,TFT_BLACK);  
    myGLCD.setTextSize(2);  
    myGLCD.drawString("Save to ESP32?", 80, 20); // Pacman right
    myGLCD.setTextSize(3);      
    myGLCD.drawString("Yes     No", 107, 110);  // D       E

    delay(2000);

//Reset screenpressed flag
boolean exitmenu = false;

while (exitmenu == false){

// Read the Touch Screen Locations
    TSPoint p = ts.getPoint();
    
    // if sharing pins, you'll need to fix the directions of the touchscreen pins
   pinMode(XM, OUTPUT);
   pinMode(YP, OUTPUT);


 
  if (pressureZ < pthreshold) {

          
    // Capture input command from user 
     
    if (readtouchscreen(Dt[0]-tvar, Dt[0]+tvar, Dt[1]-tvar, Dt[1]+tvar) == true) { // Decrement alarm hour X = 1718 Y = 1921_
        if (readtouchscreen(Dt[0]-tvar, Dt[0]+tvar, Dt[1]-tvar, Dt[1]+tvar) == true) {
          // Save Data to Eeprom
          myGLCD.setTextColor(TFT_WHITE,TFT_BLACK);  
          myGLCD.setTextSize(2);  
          myGLCD.drawString("Saving Data to EEPROM", 50, 150); 
          exitmenu = true;
        }  
      }  else 
  
    if (readtouchscreen(Et[0]-tvar, Et[0]+tvar, Et[1]-tvar, Et[1]+tvar) == true) {  // Decrement alarm minute X = 1182  Y = 2810
        if (readtouchscreen(Et[0]-tvar, Et[0]+tvar, Et[1]-tvar, Et[1]+tvar) == true) { 
          // Exit the loop
          exitmenu = true;
          myGLCD.setTextColor(TFT_WHITE,TFT_BLACK);  
          myGLCD.setTextSize(2); 
          myGLCD.drawString("Exit do nothing", 50, 200);         
        }
      } 



      // Should mean changes should scroll if held down
        delay(1000);      
    } 

  }
}

void writetoeeprom() { // Write the varialbles to Eeeprom

  testburn = true; // Only write once to Eeprom

   // Cycle through the data and store in EEPROM
   // writeIntIntoEEPROM(int address, int number) // Enables Integer to be written in EEPROM using two consecutive Bytes
   
  writeIntIntoEEPROM(0,  At[0]);
  writeIntIntoEEPROM(2,  At[1]);
  writeIntIntoEEPROM(4,  Bt[0]);
  writeIntIntoEEPROM(6,  Bt[1]);
  writeIntIntoEEPROM(8,  Ct[0]);
  writeIntIntoEEPROM(10, Ct[1]);
  writeIntIntoEEPROM(12, Dt[0]);
  writeIntIntoEEPROM(14, Dt[1]);

  writeIntIntoEEPROM(16,  Et[0]);
  writeIntIntoEEPROM(18,  Et[1]);
  writeIntIntoEEPROM(20,  Ft[0]);
  writeIntIntoEEPROM(22,  Ft[1]);
  writeIntIntoEEPROM(24,  Gt[0]);
  writeIntIntoEEPROM(26,  Gt[1]);
  writeIntIntoEEPROM(28,  Ht[0]);
  writeIntIntoEEPROM(30,  Ht[1]);
  
  writeIntIntoEEPROM(32,  It[0]);
  writeIntIntoEEPROM(34,  It[1]);
  writeIntIntoEEPROM(36,  Jt[0]);
  writeIntIntoEEPROM(38,  Jt[1]);
  writeIntIntoEEPROM(40,  Kt[0]);
  writeIntIntoEEPROM(42,  Kt[1]);
  writeIntIntoEEPROM(44,  Lt[0]);
  writeIntIntoEEPROM(46,  Lt[1]);
   
  
  EEPROM.commit(); // Writes the final values to EEPROM

  // Clear the screen
    myGLCD.fillScreen(TFT_BLACK);
    myGLCD.drawRoundRect(0, 0, 319, 239, 2, TFT_BLUE); // X,Y location then X,Y Size 
    myGLCD.drawRoundRect(2, 2, 315, 235, 2, TFT_BLUE); 
  
  myGLCD.setTextSize(2);
  myGLCD.setTextColor(TFT_YELLOW,TFT_BLACK); 
  myGLCD.drawString("KABOOM!!", 120, 80); // Pacman down
  myGLCD.drawString("Screen Callibrated", 60, 120); // Pacman down
  myGLCD.drawString("Now load Pacman Code!!", 30, 140); // Pacman down

 delay(4000);
}


/*
    EEPROM.write(0, ledState);
    EEPROM.commit();
   
   // Alarm Set buttons
    myGLCD.setTextSize(3);
    myGLCD.drawString("A       B", 107, 20);  // A = Hour +         B = Minute +
    myGLCD.drawString("D       E", 107, 110); // D = Hour -         E = Minute -

      myGLCD.setTextSize(2);
      myGLCD.drawString(" C ", 252, 208);    // Exit screen
//      myGLCD.drawRoundRect(240 , 200 , 70  , 30  , 2 , TFT_YELLOW);


    // Draw Alarm Status
      myGLCD.setTextSize(2);

      myGLCD.drawString("F", 20, 208); // Alarm Set/Off button and speed button
 
//      myGLCD.drawRoundRect(10 , 200 , 130  , 30  , 2 , TFT_YELLOW);
      myGLCD.drawString("G", 155, 106); // Centre of screen

      myGLCD.drawString("H", 155, 208); // Pacman down
      myGLCD.drawString("I", 155, 20);  // Pacman up
      myGLCD.drawString("J", 10, 106); // Pacman left
      myGLCD.drawString("K", 300, 106); // Pacman right

      myGLCD.drawString("L", 280, 15); // Alarm Menu button

*/

/*
sint At[3] = {650,2450,0};    // Alarm Hour increment Button
int Bt[3] = {375,3097,0};    // Alarm Minute increment Button
int Dt[3] = {1300,1998,0};   // Alarm Hour decrement Button
int Et[3] = {900,2800,0};     // Alarm Minute decrement Button

int It[3] = {500,2600,0};    // Pacman Up
int Jt[3] = {1460,1860,0};   // Pacman Left
int Kt[3] = {850,3150,0};    // Pacman Right
int Ht[3] = {1400,2120,0};   // Pacman Down

int Ct[3] = {1120,3000,0};   // Exit screen
int Ft[3] = {1900,1650,0};   // Alarm Set/Off button and speed button
int Lt[3] = {270,3330,0};    // Alarm Menu button
int Gt[3] = {989,2390,0};    // Setup Menu and Change Pacman character
 * 
 */
