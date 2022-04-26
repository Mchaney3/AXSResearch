 /*  Retro Pac-Man Clock
 Author: @TechKiwiGadgets Date 04/01/2022
 
 *  Licensed as Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0)
 *  You are free to:
    Share — copy and redistribute the material in any medium or format
    Adapt — remix, transform, and build upon the material
    Under the following terms:
    Attribution — You must give appropriate credit, provide a link to the license, and indicate if changes were made. You may do so in any reasonable manner, but not in any way that suggests the licensor endorses you or your use.
    NonCommercial — You may not use the material for commercial purposes.
    ShareAlike — If you remix, transform, or build upon the material, you must distribute your contributions under the same license as the original
  
 V68 - Incorporate new callibration data stored in EEPROM. Ready for release on Instructables. 
 
 */

#include "SoundData.h"
#include "XT_DAC_Audio.h"
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

#define YP 16  // must be an analog pin, use "An" notation!
#define XM 17  // must be an analog pin, use "An" notation!
#define YM 18  // can be a digital pin
#define XP 19  // can be a digital pin

#define MAXPRESSURE 899

#define EEPROM_SIZE 48 // 48 EEPROM bytes required to store 24 Integer values



// Enter Callibration Variables here for the touch screen
// Run callibration sketch first then write down the following coordicates and update sketch

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

boolean debounce = false; 


const char* TZ_INFO = "GMT+0BST-1,M3.5.0/01:00:00,M10.5.0/02:00:00";
const char* ntpServer = "uk.pool.ntp.org";

const char* ssid       = "*******";
const char* password   = "*******";

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

// Display Dimmer Variables
int dimscreen = 255; // This variable is used to drive the screen brightness where 255 is max brightness
int LDR = 100; // LDR variable measured directly from Analog 7

extern unsigned short c_pacman[0x310]; // Ghost Bitmap Straight ahead
extern unsigned short ms_c_pacman_u[0x310]; // Ghost Bitmap Straight ahead
extern unsigned short ms_c_pacman_d[0x310]; // Ghost Bitmap Straight ahead
extern unsigned short ms_c_pacman_l[0x310]; // Ghost Bitmap Straight ahead
extern unsigned short ms_c_pacman_r[0x310]; // Ghost Bitmap Straight ahead
extern unsigned short ms_d_m_pacman[0x310]; // Ghost Bitmap Straight ahead
extern unsigned short ms_d_o_pacman[0x310]; // Ghost Bitmap Straight ahead
extern unsigned short ms_l_m_pacman[0x310]; // Ghost Bitmap Straight ahead
extern unsigned short ms_l_o_pacman[0x310]; // Ghost Bitmap Straight ahead
extern unsigned short ms_r_m_pacman[0x310]; // Ghost Bitmap Straight ahead
extern unsigned short ms_r_o_pacman[0x310]; // Ghost Bitmap Straight ahead
extern unsigned short ms_u_m_pacman[0x310]; // Ghost Bitmap Straight ahead
extern unsigned short ms_u_o_pacman[0x310]; // Ghost Bitmap Straight ahead
extern unsigned short d_m_pacman[0x310]; // Ghost Bitmap Straight ahead
extern unsigned short d_o_pacman[0x310]; // Ghost Bitmap Straight ahead
extern unsigned short l_m_pacman[0x310]; // Ghost Bitmap Straight ahead
extern unsigned short l_o_pacman[0x310]; // Ghost Bitmap Straight ahead
extern unsigned short r_m_pacman[0x310]; // Ghost Bitmap Straight ahead
extern unsigned short r_o_pacman[0x310]; // Ghost Bitmap Straight ahead
extern unsigned short u_m_pacman[0x310]; // Ghost Bitmap Straight ahead
extern unsigned short u_o_pacman[0x310]; // Ghost Bitmap Straight ahead
extern unsigned short ru_ghost[0x310]; // Ghost Bitmap Straight ahead
extern unsigned short rd_ghost[0x310]; // Ghost Bitmap Straight ahead
extern unsigned short rl_ghost[0x310]; // Ghost Bitmap Straight ahead
extern unsigned short rr_ghost[0x310]; // Ghost Bitmap Straight ahead
extern unsigned short fruit[0x310]; // Ghost Bitmap Straight ahead
extern unsigned short bluepacman[0x310]; // Ghost Bitmap Straight ahead

// Touch screen coordinates
boolean screenPressed = false;
int xT,yT;
int userT = 4; // flag to indicate directional touch on screen
boolean setupscreen = false; // used to access the setup screen

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
// There are four digits that bneed to be drawn independently to ensure consisitent positioning of time
  int c1 = 20;  // Tens hour digit
  int c2 = 20;  // Ones hour digit
  int c3 = 20;  // Tens minute digit
  int c4 = 20;  // Ones minute digit

TFT_eSPI myGLCD = TFT_eSPI();       // Invoke custom library

unsigned long runTime = 0;

XT_Wav_Class Pacman(PM);     // create an object of type XT_Wav_Class that is used by 
                                      // the dac audio class (below), passing wav data as parameter.
XT_Wav_Class pacmangobble(gobble);     // create an object of type XT_Wav_Class that is used by 
                                                                          
XT_DAC_Audio_Class DacAudio(25,0);    // Create the main player class object. 
                                      // Use GPIO 25, one of the 2 DAC pins and timer 0


// Touchscreen Callibration Coordinates
int tvar = 150; // This number used for + and - boundaries based on callibration

int Ax = 650; int Ay = 2450; // Alarm Hour increment Button
int Bx = 375; int By = 3097; // Alarm Minute increment Button
int Dx = 1300; int Dy = 1998; // Alarm Hour decrement Button
int Ex = 900; int Ey = 2800; // Alarm Minute decrement Button

int Ix = 500; int Iy = 2600; // Pacman Up
int Jx = 1460; int Jy = 1860; // Pacman Left
int Kx = 850; int Ky = 3150; // Pacman Right
int Hx = 1400; int Hy = 2120; // Pacman Down

int Cx = 1120; int Cy = 3000; // Exit screen
int Fx = 1900; int Fy = 1650; // Alarm Set/Off button and speed button
int Lx = 270; int Ly = 3330; // Alarm Menu button
int Gx = 989; int Gy = 2390; // Setup Menu and Change Pacman character



void setup() {


// Randomseed will shuffle the random function
randomSeed(analogRead(0));

  // Initiate display
// Setup the LCD
  myGLCD.begin();
  myGLCD.setRotation(1); // Inverted to accomodate USB cable

  myGLCD.fillScreen(TFT_BLACK);

// Initialize Dot Array
  for (int dotarray = 1; dotarray < 73; dotarray++) {    
    dot[dotarray] = 1;    
    }
  
  EEPROM.begin(EEPROM_SIZE); // Initialize the EEPROM with the predefined size


//Initialize 
  Serial.begin(115200);

pinMode(32, OUTPUT); // This pin used to mute the audio when not in use via pin 5 of PAM8403

  // MUTE sound to speaker via pin 5 of PAM8403
digitalWrite(32,LOW);

  //connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println(" CONNECTED");
  
  //init and get the time
  configTime(0, 0, ntpServer);
  setenv("TZ", TZ_INFO, 1);
  tzset();
  printLocalTime();

  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

 drawscreen(); // Initiate the game
 UpdateDisp(); // update value to clock 

 playalarmsound1(); // Play Alarmsound once on powerup

 // Initialise Touch Screen Coordinates from EEPROM
 if (readIntFromEEPROM(0)<25000) { // If EEPROM callibration numbers present then use them

    Ax = readIntFromEEPROM(0);  Ay = readIntFromEEPROM(2); 
    Bx = readIntFromEEPROM(4);  By = readIntFromEEPROM(6);
    Cx = readIntFromEEPROM(8);  Cy = readIntFromEEPROM(10);
    Dx = readIntFromEEPROM(12); Dy = readIntFromEEPROM(14); 
    Ex = readIntFromEEPROM(16);  Ey = readIntFromEEPROM(18);
    Fx = readIntFromEEPROM(20);  Fy = readIntFromEEPROM(22);
    Gx = readIntFromEEPROM(24);  Gy = readIntFromEEPROM(26); 
    Hx = readIntFromEEPROM(28);  Hy = readIntFromEEPROM(30);
    Ix = readIntFromEEPROM(32);  Iy = readIntFromEEPROM(34);
    Jx = readIntFromEEPROM(36);  Jy = readIntFromEEPROM(38);
    Kx = readIntFromEEPROM(40);  Ky = readIntFromEEPROM(42); 
    Lx = readIntFromEEPROM(44);  Ly = readIntFromEEPROM(46);    
 }

}

void loop() {

setgamespeed(); // Set game animation speed  
printscoreboard(); //Print scoreboard
drawfruit();  // Draw fruit and allocate points
refreshgame(); // Read the current date and time from the RTC and reset board
triggeralarm(); //=== Check if Alarm needs to be sounded
mainuserinput(); // Check if user input to touch screen
displaypacman(); // Draw Pacman in position on screen
displayghost(); // Draw Ghost in position on screen
delay(dly); 

//    myGLCD.fillRect(50  , 70  , 45  , 95, TFT_RED);

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

void setgamespeed(){

 if (userspeedsetting == 1) {
  gamespeed = 22;
} else
if (userspeedsetting == 2) {
  gamespeed = 14;
} else
if (userspeedsetting == 3) {
  gamespeed = 0;
}

dly = gamespeed; // Reset the game speed  
}

void printscoreboard(){ //Print scoreboard

    if((ghostscore >= 95)||(pacmanscore >= 95)){ // Reset scoreboard if over 95
  ghostscore = 0;
  pacmanscore = 0;
  
    for (int dotarray = 1; dotarray < 73; dotarray++) {
      
      dot[dotarray] = 1;
      
      }
  
  // Blank the screen across the digits before redrawing them
  //  myGLCD.setColor(0, 0, 0);
  //  myGLCD.setBackColor(0, 0, 0);
  
      myGLCD.fillRect(299  , 87  , 15  , 10  , TFT_BLACK); // Blankout ghost score  
      myGLCD.fillRect(7  , 87  , 15  , 10  , TFT_BLACK);   // Blankout pacman score
  
  drawscreen(); // Redraw dots  
  }
  

   myGLCD.setTextColor(TFT_RED,TFT_BLACK);   myGLCD.setTextSize(1);
  
  // Account for position issue if over or under 10
  
  if (ghostscore >= 10){
  //  myGLCD.setColor(237, 28, 36);
  //  myGLCD.setBackColor(0, 0, 0);
    myGLCD.drawNumber(ghostscore,301,88); 
  } else {
  //  myGLCD.setColor(237, 28, 36);
  //  myGLCD.setBackColor(0, 0, 0);
    myGLCD.drawNumber(ghostscore,307,88);  // Account for being less than 10
  }
  
   myGLCD.setTextColor(TFT_YELLOW,TFT_BLACK);   myGLCD.setTextSize(1);
  
  if (pacmanscore >= 10){
  //  myGLCD.setColor(248, 236, 1);
  //  myGLCD.setBackColor(0, 0, 0);
    myGLCD.drawNumber(pacmanscore,9,88);  
  } else{
  //  myGLCD.setColor(248, 236, 1);
  //  myGLCD.setBackColor(0, 0, 0);
    myGLCD.drawNumber(pacmanscore,15,88);  // Account for being less than 10
  } 
}

void drawfruit(){  // Draw fruit and allocate points

     // Draw fruit
  if ((fruitdrawn == false)&&(fruitgone == false)){ // draw fruit and set flag that fruit present so its not drawn again
      drawicon(146, 168, fruit); //   draw fruit 
      fruitdrawn = true;
  }  
  
  // Redraw fruit if Ghost eats fruit only if Ghost passesover 172 or 120 on the row 186
  if ((fruitdrawn == true)&&(fruitgone == false)&&(xG >= 168)&&(xG <= 170)&&(yG >= 168)&&(yG <= 180)){
        drawicon(146, 168, fruit); //   draw fruit 
  }
  
  if ((fruitdrawn == true)&&(fruitgone == false)&&(xG == 120)&&(yG == 168)){
        drawicon(146, 168, fruit); //   draw fruit 
  }
  
  // Award Points if Pacman eats Big Dots
  if ((fruitdrawn == true)&&(fruitgone == false)&&(xP == 146)&&(yP == 168)){
    fruitgone = true; // If Pacman eats fruit then fruit disappears  
    pacmanscore = pacmanscore + 5; //Increment pacman score 
  }
 
}


void refreshgame(){ // Read the current date and time from the RTC and reset board

// Read the current date and time from the RTC and reset board
rfc++;
  if (rfc >= rfcvalue) { // count cycles and print time
    UpdateDisp(); // update value to clock then ...
     fruiteatenpacman =  false; // Turn Ghost red again  
     fruitdrawn = false; // If Pacman eats fruit then fruit disappears
     fruitgone = false;
     // Reset every minute both characters
     pacmanlost = false;
     ghostlost = false;
     dly = gamespeed; // reset delay
     rfc = 0;
     
  }
  
}


void triggeralarm(){ //=== Check if Alarm needs to be sounded

   if (alarmstatus == true){  

         if ( (alarmhour == clockhour) && (alarmminute == clockminute)) {  // Sound the alarm        
               soundalarm = true;
           }     
      
   }
}

void mainuserinput() { // Check if user input to touch screen

/*Temp Test of alarm metrics
Serial.print(alarmhour);
Serial.print(" : ");
Serial.print(alarmminute);
Serial.print("  ");
Serial.print(clockhour);
Serial.print(" : ");
Serial.print(clockminute);
Serial.print("  ");
Serial.print(" alarmstatus  ");
Serial.print(alarmstatus);
Serial.print(" Soundalarm   ");
Serial.println(soundalarm);
*/

// Check if user input to touch screen
// UserT sets direction 0 = right, 1 = down, 2 = left, 3 = up, 4 = no touch input
// Read the Touch Screen Locations
  TSPoint p = ts.getPoint();

  // if sharing pins, you'll need to fix the directions of the touchscreen pins
 pinMode(XM, OUTPUT);
 pinMode(YP, OUTPUT);

  if (( p.z < MAXPRESSURE)&&( p.z > 0)) {

     // **********************************
     // ******* Enter Setup Mode *********
     // **********************************
     
     // If centre of screen touched while alarm sounding then turn off the sound and reset the alarm to not set 
        
        if ((p.z < MAXPRESSURE) && (alarmstatus == true) && (soundalarm ==true)) {
          alarmstatus = false;
          soundalarm = false;
          delay(300);
        } 
  
        if (readtouchscreen(Jx-tvar, Jx+tvar, Jy-tvar, Jy+tvar) == true) {      
          if (readtouchscreen(Jx-tvar, Jx+tvar, Jy-tvar, Jy+tvar) == true) { 
            userT = 2; // Request to go left   X = 1695  Y = 1959_
          }
        } else
        if (readtouchscreen(Kx-tvar, Kx+tvar, Ky-tvar, Ky+tvar) == true) {    
          if (readtouchscreen(Kx-tvar, Kx+tvar, Ky-tvar, Ky+tvar) == true) { 
            userT = 0; // Request to go right   X = 1017  Y = 3317
          }
        } else 
        if (readtouchscreen(Ix-tvar, Ix+tvar, Iy-tvar, Iy+tvar) == true) {    
          if (readtouchscreen(Ix-tvar, Ix+tvar, Iy-tvar, Iy+tvar) == true) { 
            userT = 3; // Request to go Up   X = 579 Y = 2603
          }
        } else 
        if (readtouchscreen(Hx-tvar, Hx+tvar, Hy-tvar, Hy+tvar) == true) {    
          if (readtouchscreen(Hx-tvar, Hx+tvar, Hy-tvar, Hy+tvar) == true) { 
            userT = 1; // Request to go Down   X = 1699 Y = 2090
          }
        } else
        if ((readtouchscreen(Gx-tvar, Gx+tvar, Gy-tvar, Gy+tvar) == true) &&  (soundalarm !=true)) { // Call Setup Routine if alarm is not sounding 
          if ((readtouchscreen(Gx-tvar, Gx+tvar, Gy-tvar, Gy+tvar) == true) &&  (soundalarm !=true)) { 
            setupclockmenu(); // Call the setup routine X = 1387  Y = 2280  
          }
        } 
              
        if(pacmanlost == false){ // only apply requested changes if Pacman still alive
    
           if (userT == 2 && D == 0 ){ // Going Right request to turn Left OK
             D = 2;
             }
           if (userT == 0 && D == 2 ){ // Going Left request to turn Right OK
             D = 0;
             }
           if (userT == 1 && D == 3 ){ // Going Up request to turn Down OK
             D = 1;
             }
           if (userT == 3 && D == 1 ){ // Going Down request to turn Up OK
             D = 3;
             }
          }

    screenPressed = true;
    }
        // Doesn't allow holding the screen / you must tap it
        else {
          screenPressed = false;
    }


  //=== Start Alarm Sound - Sound pays for 10 seconds then will restart at 20 second mark
  
  
  
  if ((alarmstatus == true)&&(soundalarm==true)){ // Set off a counter and take action to restart sound if screen not touched
  
          playalarmsound1(); 
          UpdateDisp(); // update value to clock 
  }

}

void displaypacman(){ // Draw Pacman in position on screen
 // Pacman Captured
// If pacman captured then pacman dissapears until reset
if ((fruiteatenpacman == false)&&(abs(xG-xP)<=5)&&(abs(yG-yP)<=5)){ 
// firstly blank out Pacman
//    myGLCD.setColor(0,0,0);
    myGLCD.fillRect(xP, yP, 28, 28, TFT_BLACK); 

  if (pacmanlost == false){
    ghostscore = ghostscore + 15;  
  }
  pacmanlost = true;
 // Slow down speed of drawing now only one moving charater
  dly = gamespeed;
  }
 
if (pacmanlost == false) { // Only draw pacman if he is still alive


// Draw Pac-Man
drawPacman(xP,yP,P,D,prevD); // Draws Pacman at these coordinates
  

// If Pac-Man is on a dot then print the adjacent dots if they are valid

//  myGLCD.setColor(200, 200, 200);
  
// Check Rows

if (yP== 4) {  // if in Row 1 **********************************************************
  if (xP== 4) { // dot 1
     if (dot[2] == 1) {  // Check if dot 2 gobbled already
    myGLCD.fillCircle(42, 19, 2, TFT_GREY); // dot 2
     }    
      if (dot[13] == 1) {  // Check if dot 13 gobbled already
    myGLCD.fillCircle(19, 40, 7, TFT_GREY); // Big dot 13
     }    

  } else
  if (xP== 28) { // dot 2
     if (dot[1] == 1) {  // Check if dot 1 gobbled already
    myGLCD.fillCircle(19, 19, 2, TFT_GREY); // dot 1
     }    
      if (dot[3] == 1) {  // Check if dot 3 gobbled already
    myGLCD.fillCircle(65, 19, 2, TFT_GREY); // dot 3
     }    

  } else
  if (xP== 52) { // dot 3
     if (dot[2] == 1) {  // Check if dot 2 gobbled already
    myGLCD.fillCircle(42, 19, 2, TFT_GREY); // dot 2
     }    
      if (dot[4] == 1) {  // Check if dot 4 gobbled already
    myGLCD.fillCircle(88, 19, 2, TFT_GREY); // dot 4
     } 
      if (dot[14] == 1) {  // Check if dot 14 gobbled already
    myGLCD.fillCircle(77, 40, 2, TFT_GREY); // dot 14
     }   
  } else
  if (xP== 74) { // dot 4
     if (dot[3] == 1) {  // Check if dot 3 gobbled already
    myGLCD.fillCircle(65, 19, 2, TFT_GREY); // dot 3
     }    
      if (dot[5] == 1) {  // Check if dot 5 gobbled already
    myGLCD.fillCircle(112, 19, 2, TFT_GREY); // dot 5
     }   
      if (dot[14] == 1) {  // Check if dot 14 gobbled already
    myGLCD.fillCircle(77, 40, 2, TFT_GREY); // dot 14
     }    
  } else
  if (xP== 98) { // dot 5
     if (dot[4] == 1) {  // Check if dot 3 gobbled already
    myGLCD.fillCircle(88, 19, 2, TFT_GREY); // dot 4
     }    
      if (dot[6] == 1) {  // Check if dot 5 gobbled already
    myGLCD.fillCircle(136, 19, 2, TFT_GREY); // dot 6
     }     
  } else
  if (xP== 120) { // dot 6
     if (dot[5] == 1) {  // Check if dot 5 gobbled already
    myGLCD.fillCircle(112, 19, 2, TFT_GREY); // dot 5
     }    
      if (dot[15] == 1) {  // Check if dot 15 gobbled already
    myGLCD.fillCircle(136, 40, 2, TFT_GREY); // dot 15
     }     
  } else
 

 if (xP== 168) { // dot 7
      if (dot[16] == 1) {  // Check if dot 16 gobbled already
    myGLCD.fillCircle(183, 40, 2, TFT_GREY); // dot 16
     }    
      if (dot[8] == 1) {  // Check if dot 8 gobbled already
    myGLCD.fillCircle(206, 19, 2, TFT_GREY); // dot 8
     }     
  } else
  if (xP== 192) { // dot 8
      if (dot[7] == 1) {  // Check if dot 7 gobbled already
    myGLCD.fillCircle(183, 19, 2, TFT_GREY); // dot 7
     }    
      if (dot[9] == 1) {  // Check if dot 9 gobbled already
    myGLCD.fillCircle(229, 19, 2, TFT_GREY); // dot 9
     }    
  } else
  if (xP== 216) { // dot 9
      if (dot[10] == 1) {  // Check if dot 10 gobbled already
    myGLCD.fillCircle(252, 19, 2, TFT_GREY); // dot 10
     }    
      if (dot[8] == 1) {  // Check if dot 8 gobbled already
    myGLCD.fillCircle(206, 19, 2, TFT_GREY); // dot 8
     }      
      if (dot[17] == 1) {  // Check if dot 17 gobbled already
    myGLCD.fillCircle(241, 40, 2, TFT_GREY); // dot 17
     }   
 } else
  if (xP== 238) { // dot 10
      if (dot[11] == 1) {  // Check if dot 11 gobbled already
    myGLCD.fillCircle(275, 19, 2, TFT_GREY); // dot 11
     }    
      if (dot[9] == 1) {  // Check if dot 9 gobbled already
    myGLCD.fillCircle(229, 19, 2, TFT_GREY); // dot 9
     }      
      if (dot[17] == 1) {  // Check if dot 17 gobbled already
    myGLCD.fillCircle(241, 40, 2, TFT_GREY); // dot 17
     }   
  } else
  if (xP== 262) { // dot 11
      if (dot[10] == 1) {  // Check if dot 10 gobbled already
    myGLCD.fillCircle(252, 19, 2, TFT_GREY); // dot 10
     }    
      if (dot[12] == 1) {  // Check if dot 12 gobbled already
    myGLCD.fillCircle(298, 19, 2, TFT_GREY); // dot 12
     }    
      if (dot[18] == 1) {  // Check if Big dot 18 gobbled already
    myGLCD.fillCircle(298, 40, 7, TFT_GREY); // dot 18
     } 
  } else
  if (xP== 284) { // dot 12
      if (dot[11] == 1) {  // Check if dot 11 gobbled already
    myGLCD.fillCircle(275, 19, 2, TFT_GREY); // dot 11
     }    
      if (dot[18] == 1) {  // Check if Big dot 18 gobbled already
    myGLCD.fillCircle(298, 40, 7, TFT_GREY); // dot 18
     }  
  }
} else 
if (yP== 26) {  // if in Row 2  **********************************************************
  if (xP== 4) { // dot 13
     if (dot[1] == 1) {  // Check if dot 1 gobbled already
    myGLCD.fillCircle(19, 19, 2, TFT_GREY); // dot 1
     }    
      if (dot[19] == 1) {  // Check if dot 19 gobbled already
    myGLCD.fillCircle(19, 60, 2, TFT_GREY); //  dot 19
     }   
  } else
  
    if (xP== 62) { // dot 14
      if (dot[3] == 1) {  // Check if dot 3 gobbled already
    myGLCD.fillCircle(65, 19, 2, TFT_GREY); // dot 3
     }   
         if (dot[4] == 1) {  // Check if dot 4 gobbled already
    myGLCD.fillCircle(88, 19, 2, TFT_GREY); // dot 4
     } 
         if (dot[21] == 1) {  // Check if dot 21 gobbled already
    myGLCD.fillCircle(65, 60, 2, TFT_GREY); // dot 21
     }   
      if (dot[22] == 1) {  // Check if dot 22 gobbled already
    myGLCD.fillCircle(88, 60, 2, TFT_GREY); // dot 22
     }    
     
  } else
  
  if (xP== 120) { // dot 15
     if (dot[24] == 1) {  // Check if dot 24 gobbled already
    myGLCD.fillCircle(136, 60, 2, TFT_GREY); // dot 24
     }    
      if (dot[6] == 1) {  // Check if dot 6 gobbled already
    myGLCD.fillCircle(136, 19, 2, TFT_GREY); // dot 6
     }      
  } else
  if (xP== 168) { // dot 16
      if (dot[7] == 1) {  // Check if dot 7 gobbled already
    myGLCD.fillCircle(183, 19, 2, TFT_GREY); // dot 7
     }    
      if (dot[26] == 1) {  // Check if dot 26 gobbled already
    myGLCD.fillCircle(183, 60, 2, TFT_GREY); // dot 26
     }          
  } else
    if (xP== 228) { // dot 17
      if (dot[9] == 1) {  // Check if dot 9 gobbled already
    myGLCD.fillCircle(229, 19, 2, TFT_GREY); // dot 9
     }      
       if (dot[10] == 1) {  // Check if dot 10 gobbled already
    myGLCD.fillCircle(252, 19, 2, TFT_GREY); // dot 10
     }  
      if (dot[28] == 1) {  // Check if dot 28 gobbled already
    myGLCD.fillCircle(229, 60, 2, TFT_GREY); // dot 28
     }  
       if (dot[29] == 1) {  // Check if dot 29 gobbled already
    myGLCD.fillCircle(252, 60, 2, TFT_GREY); // dot 29
     }     
     
  } else
  if (xP== 284) { // dot 18
      if (dot[31] == 1) {  // Check if dot 31 gobbled already
    myGLCD.fillCircle(298, 60, 2, TFT_GREY); // dot 31
     }    
      if (dot[12] == 1) {  // Check if dot 12 gobbled already
    myGLCD.fillCircle(298, 19, 2, TFT_GREY); // dot 12
     }  
  }
} else
if (yP== 46) {  // if in Row 3  **********************************************************
  if (xP== 4) { // dot 19
     if (dot[20] == 1) {  // Check if dot 20 gobbled already
    myGLCD.fillCircle(42, 60, 2, TFT_GREY); // dot 20
     }    
      if (dot[13] == 1) {  // Check if dot 13 gobbled already
    myGLCD.fillCircle(19, 40, 7, TFT_GREY); // Big dot 13
     }  
  } else
  if (xP== 28) { // dot 20
     if (dot[19] == 1) {  // Check if dot 19 gobbled already
    myGLCD.fillCircle(19, 60, 2, TFT_GREY); // dot 19
     }    
      if (dot[21] == 1) {  // Check if dot 21 gobbled already
    myGLCD.fillCircle(65, 60, 2, TFT_GREY); // dot 21
     }   
      if (dot[32] == 1) {  // Check if dot 32 gobbled already
    myGLCD.fillCircle(42, 80, 2, TFT_GREY); // dot 32
     }    
      if (dot[13] == 1) {  // Check if dot 13 gobbled already
    myGLCD.fillCircle(19, 40, 7, TFT_GREY); // Big dot 13
     } 
  } else
  if (xP== 52) { // dot 21
     if (dot[20] == 1) {  // Check if dot 20 gobbled already
    myGLCD.fillCircle(42, 60, 2, TFT_GREY); // dot 20
     }    
      if (dot[22] == 1) {  // Check if dot 22 gobbled already
    myGLCD.fillCircle(88, 60, 2, TFT_GREY); // dot 22
     } 
      if (dot[14] == 1) {  // Check if dot 14 gobbled already
    myGLCD.fillCircle(77, 40, 2, TFT_GREY); // dot 14
     }         
  } else
  if (xP== 74) { // dot 22
      if (dot[21] == 1) {  // Check if dot 21 gobbled already
    myGLCD.fillCircle(65, 60, 2, TFT_GREY); // dot 21
     }    
      if (dot[23] == 1) {  // Check if dot 23 gobbled already
    myGLCD.fillCircle(112, 60, 2, TFT_GREY); // dot 23
     } 
      if (dot[14] == 1) {  // Check if dot 14 gobbled already
    myGLCD.fillCircle(77, 40, 2, TFT_GREY); // dot 14
     }    
  } else
  if (xP== 98) { // dot 23
     if (dot[24] == 1) {  // Check if dot 24 gobbled already
    myGLCD.fillCircle(136, 60, 2, TFT_GREY); // dot 24
     }    
      if (dot[22] == 1) {  // Check if dot 22 gobbled already
    myGLCD.fillCircle(88, 60, 2, TFT_GREY); // dot 22
     }  
    
  } else
  if (xP== 120) { // dot 24
      if (dot[25] == 1) {  // Check if dot 25 gobbled already
    myGLCD.fillCircle(160, 60, 2, TFT_GREY); // dot 25
     }    
      if (dot[23] == 1) {  // Check if dot 23 gobbled already
    myGLCD.fillCircle(112, 60, 2, TFT_GREY); // dot 23
     }
      if (dot[15] == 1) {  // Check if dot 15 gobbled already
    myGLCD.fillCircle(136, 40, 2, TFT_GREY); // dot 15
     }        
  } else
  if (xP== 146) { // dot 25
     if (dot[24] == 1) {  // Check if dot 24 gobbled already
    myGLCD.fillCircle(136, 60, 2, TFT_GREY); // dot 24
     }    
      if (dot[26] == 1) {  // Check if dot 26 gobbled already
    myGLCD.fillCircle(183, 60, 2, TFT_GREY); // dot 26
     }    
  } else
  if (xP== 168) { // dot 26
      if (dot[25] == 1) {  // Check if dot 25 gobbled already
    myGLCD.fillCircle(160, 60, 2, TFT_GREY); // dot 25
     }    
      if (dot[27] == 1) {  // Check if dot 27 gobbled already
    myGLCD.fillCircle(206, 60, 2, TFT_GREY); // dot 27
     }
      if (dot[16] == 1) {  // Check if dot 16 gobbled already
    myGLCD.fillCircle(183, 40, 2, TFT_GREY); // dot 16
     }    
  } else
  if (xP== 192) { // dot 27
     if (dot[28] == 1) {  // Check if dot 28 gobbled already
    myGLCD.fillCircle(229, 60, 2, TFT_GREY); // dot 28
     }    
      if (dot[26] == 1) {  // Check if dot 26 gobbled already
    myGLCD.fillCircle(183, 60, 2, TFT_GREY); // dot 26
     }      
  } else
  if (xP== 216) { // dot 28
      if (dot[29] == 1) {  // Check if dot 29 gobbled already
    myGLCD.fillCircle(252, 60, 2, TFT_GREY); // dot 29
     }    
      if (dot[27] == 1) {  // Check if dot 27 gobbled already
    myGLCD.fillCircle(206, 60, 2, TFT_GREY); // dot 27
     }      
      if (dot[17] == 1) {  // Check if dot 17 gobbled already
    myGLCD.fillCircle(241, 40, 2, TFT_GREY); // dot 17
     }   
  } else
  if (xP== 238) { // dot 29
     if (dot[28] == 1) {  // Check if dot 28 gobbled already
    myGLCD.fillCircle(229, 60, 2, TFT_GREY); // dot 28
     }    
      if (dot[30] == 1) {  // Check if dot 30 gobbled already
    myGLCD.fillCircle(275, 60, 2, TFT_GREY); // dot 30
     }      
      if (dot[17] == 1) {  // Check if dot 17 gobbled already
    myGLCD.fillCircle(241, 40, 2, TFT_GREY); // dot 17
     }   
   
  } else
  if (xP== 262) { // dot 30
      if (dot[29] == 1) {  // Check if dot 29 gobbled already
    myGLCD.fillCircle(252, 60, 2, TFT_GREY); // dot 29
     }    
      if (dot[33] == 1) {  // Check if dot 33 gobbled already
    myGLCD.fillCircle(275, 80, 2, TFT_GREY); // dot 33
     }      
      if (dot[31] == 1) {  // Check if dot 31 gobbled already
    myGLCD.fillCircle(298, 60, 2, TFT_GREY); // dot 31
     }  
  
  } else
  if (xP== 284) { // dot 31
   if (dot[18] == 1) {  // Check if Big dot 18 gobbled already
    myGLCD.fillCircle(298, 40, 7, TFT_GREY); // dot 18
   }     
   if (dot[30] == 1) {  // Check if dot 30 gobbled already
    myGLCD.fillCircle(275, 60, 2, TFT_GREY); // dot 30
   } 
  }
} else

if (yP== 168) {  // if in Row 4  **********************************************************
  if (xP== 4) { // dot 42
     if (dot[43] == 1) {  // Check if dot 43 gobbled already
    myGLCD.fillCircle(42, 181, 2, TFT_GREY); // dot 43
     }     
     if (dot[55] == 1) {  // Check if dot 55 gobbled already
    myGLCD.fillCircle(19, 201, 7, TFT_GREY); // dot 55
     }     
  } else
  if (xP== 28) { // dot 43
     if (dot[42] == 1) {  // Check if dot 42 gobbled already
    myGLCD.fillCircle(19, 181, 2, TFT_GREY); // dot 42
     }     
     if (dot[44] == 1) {  // Check if dot 44 gobbled already
    myGLCD.fillCircle(65, 181, 2, TFT_GREY); // dot 44
     }   
      if (dot[40] == 1) {  // Check if dot 40 gobbled already
    myGLCD.fillCircle(42, 160, 2, TFT_GREY); // dot 40
     }       
  } else
  if (xP== 52) { // dot 44
     if (dot[43] == 1) {  // Check if dot 43 gobbled already
    myGLCD.fillCircle(42, 181, 2, TFT_GREY); // dot 43
     }     
     if (dot[45] == 1) {  // Check if dot 45 gobbled already
    myGLCD.fillCircle(88, 181, 2, TFT_GREY); // dot 45
     } 
     if (dot[56] == 1) {  // Check if dot 56 gobbled already
    myGLCD.fillCircle(77, 201, 2, TFT_GREY); // dot 56 
     }    
  } else
  if (xP== 74) { // dot 45
     if (dot[46] == 1) {  // Check if dot 46 gobbled already
    myGLCD.fillCircle(112, 181, 2, TFT_GREY); // dot 46
     }     
     if (dot[44] == 1) {  // Check if dot 44 gobbled already
    myGLCD.fillCircle(65, 181, 2, TFT_GREY); // dot 44
     } 
     if (dot[56] == 1) {  // Check if dot 56 gobbled already
    myGLCD.fillCircle(77, 201, 2, TFT_GREY); // dot 56 
     }    
     
  } else
  if (xP== 98) { // dot 46
     if (dot[47] == 1) {  // Check if dot 47 gobbled already
    myGLCD.fillCircle(136, 181, 2, TFT_GREY); // dot 47
     }     
     if (dot[45] == 1) {  // Check if dot 45 gobbled already
    myGLCD.fillCircle(88, 181, 2, TFT_GREY); // dot 45
     }  
  } else
  if (xP== 120) { // dot 47
     if (dot[48] == 1) {  // Check if dot 48 gobbled already
    myGLCD.fillCircle(160, 181, 2, TFT_GREY); // dot 48
     }     
    // Draw fruit
    if ((fruitdrawn == true)&&(fruitgone == false)){ // draw fruit again
        drawicon(146, 168, fruit); //   draw fruit 
    }
     if (dot[46] == 1) {  // Check if dot 46 gobbled already
    myGLCD.fillCircle(112, 181, 2, TFT_GREY); // dot 46
     } 
     if (dot[57] == 1) {  // Check if dot 57 gobbled already
    myGLCD.fillCircle(136, 201, 2, TFT_GREY); // dot 57 
     }      
    // Draw fruit
    if ((fruitdrawn == true)&&(fruitgone == false)){ // draw fruit again
        drawicon(146, 168, fruit); //   draw fruit 
    }
  } else
  if (xP== 146) { // dot 48
     if (dot[47] == 1) {  // Check if dot 47 gobbled already
    myGLCD.fillCircle(136, 181, 2, TFT_GREY); // dot 47
     }     
     if (dot[49] == 1) {  // Check if dot 49 gobbled already
    myGLCD.fillCircle(183, 181, 2, TFT_GREY); // dot 49
     }  
  } else

  if (xP== 168) { // dot 49
     if (dot[48] == 1) {  // Check if dot 48 gobbled already
    myGLCD.fillCircle(160, 181, 2, TFT_GREY); // dot 48
     }     
    // Draw fruit
    if ((fruitdrawn == true)&&(fruitgone == false)){ // draw fruit again
        drawicon(146, 168, fruit); //   draw fruit 
    }
     if (dot[50] == 1) {  // Check if dot 50 gobbled already
    myGLCD.fillCircle(206, 181, 2, TFT_GREY); // dot 50
     } 
     if (dot[58] == 1) {  // Check if dot 58 gobbled already
    myGLCD.fillCircle(183, 201, 2, TFT_GREY); // dot 58
     }        
    // Draw fruit
    if ((fruitdrawn == true)&&(fruitgone == false)){ // draw fruit again
        drawicon(146, 168, fruit); //   draw fruit 
    }
  } else
  if (xP== 192) { // dot 50
     if (dot[51] == 1) {  // Check if dot 51 gobbled already
    myGLCD.fillCircle(229, 181, 2, TFT_GREY); // dot 51
     }     
     if (dot[49] == 1) {  // Check if dot 49 gobbled already
    myGLCD.fillCircle(183, 181, 2, TFT_GREY); // dot 49
     }      
  } else
  if (xP== 216) { // dot 51
     if (dot[50] == 1) {  // Check if dot 50 gobbled already
    myGLCD.fillCircle(206, 181, 2, TFT_GREY); // dot 50
     }    
     if (dot[52] == 1) {  // Check if dot 52 gobbled already
    myGLCD.fillCircle(252, 181, 2, TFT_GREY); // dot 52
     } 
     if (dot[59] == 1) {  // Check if dot 59 gobbled already
    myGLCD.fillCircle(241, 201, 2, TFT_GREY); // dot 59
     }     
  } else
  if (xP== 238) { // dot 52
     if (dot[53] == 1) {  // Check if dot 53 gobbled already
    myGLCD.fillCircle(275, 181, 2, TFT_GREY); // dot 53
     }    
     if (dot[51] == 1) {  // Check if dot 51 gobbled already
    myGLCD.fillCircle(229, 181, 2, TFT_GREY); // dot 51
     }  
     if (dot[59] == 1) {  // Check if dot 59 gobbled already
    myGLCD.fillCircle(241, 201, 2, TFT_GREY); // dot 59
     }     
  } else
 
 if (xP== 262) { // dot 53
     if (dot[41] == 1) {  // Check if dot 41 gobbled already
    myGLCD.fillCircle(275, 160, 2, TFT_GREY); // dot 41
     }    
     if (dot[52] == 1) {  // Check if dot 52 gobbled already
    myGLCD.fillCircle(252, 181, 2, TFT_GREY); // dot 52
     } 
     if (dot[54] == 1) {  // Check if dot 54 gobbled already
    myGLCD.fillCircle(298, 181, 2, TFT_GREY); // dot 54
     }  
     if (dot[60] == 1) {  // Check if dot 60 gobbled already
    myGLCD.fillCircle(298, 201, 7, TFT_GREY); // Big dot 60
     }        
  } else
  if (xP== 284) { // dot 54
     if (dot[53] == 1) {  // Check if dot 53 gobbled already
    myGLCD.fillCircle(275, 181, 2, TFT_GREY); // dot 53
     }    
     if (dot[60] == 1) {  // Check if dot 60 gobbled already
    myGLCD.fillCircle(298, 201, 7, TFT_GREY); // Big dot 60
     }      
  } 

} else
if (yP== 188) {  // if in Row 5  **********************************************************
  if (xP== 4) { // dot 55
     if (dot[42] == 1) {  // Check if dot 42 gobbled already
    myGLCD.fillCircle(19, 181, 2, TFT_GREY); // dot 42
     } 
     if (dot[61] == 1) {  // Check if dot 61 gobbled already
    myGLCD.fillCircle(19, 221, 2, TFT_GREY); // dot 61
     }    
  } else
   if (xP== 62) { // dot 56
     if (dot[44] == 1) {  // Check if dot 44 gobbled already
    myGLCD.fillCircle(65, 181, 2, TFT_GREY); // dot 44
     } 
     if (dot[45] == 1) {  // Check if dot 45 gobbled already
    myGLCD.fillCircle(88, 181, 2, TFT_GREY); // dot 45
     } 
     if (dot[63] == 1) {  // Check if dot 63 gobbled already
    myGLCD.fillCircle(65, 221, 2, TFT_GREY); // dot 63
     }
     if (dot[64] == 1) {  // Check if dot 64 gobbled already
    myGLCD.fillCircle(88, 221, 2, TFT_GREY); // dot 64
     }      
     
  } else
  
  if (xP== 120) { // dot 57
     if (dot[47] == 1) {  // Check if dot 47 gobbled already
    myGLCD.fillCircle(136, 181, 2, TFT_GREY); // dot 47
     }     
     if (dot[66] == 1) {  // Check if dot 66 gobbled already
    myGLCD.fillCircle(136, 221, 2, TFT_GREY); // dot 66
     }    
    // Draw fruit
    if ((fruitdrawn == true)&&(fruitgone == false)){ // draw fruit again
        drawicon(146, 168, fruit); //   draw fruit 
    }
  } else
  if (xP== 168) { // dot 58
     if (dot[67] == 1) {  // Check if dot 67 gobbled already
    myGLCD.fillCircle(183, 221, 2, TFT_GREY); // dot 67
     }     
     if (dot[49] == 1) {  // Check if dot 49 gobbled already
    myGLCD.fillCircle(183, 181, 2, TFT_GREY); // dot 49
     }       
    // Draw fruit
    if ((fruitdrawn == true)&&(fruitgone == false)){ // draw fruit again
        drawicon(146, 168, fruit); //   draw fruit 
    }
  } else
  
  if (xP== 228) { // dot 59
     if (dot[51] == 1) {  // Check if dot 51 gobbled already
    myGLCD.fillCircle(229, 181, 2, TFT_GREY); // dot 51
     }
     if (dot[52] == 1) {  // Check if dot 52 gobbled already
    myGLCD.fillCircle(252, 181, 2, TFT_GREY); // dot 52
     } 
     if (dot[69] == 1) {  // Check if dot 69 gobbled already
    myGLCD.fillCircle(229, 221, 2, TFT_GREY); // dot 69
     } 
     if (dot[70] == 1) {  // Check if dot 70 gobbled already
    myGLCD.fillCircle(252, 221, 2, TFT_GREY); // dot 70
     }      
     
  } else
  
  if (xP== 284) { // dot 60
     if (dot[72] == 1) {  // Check if dot 72 gobbled already
    myGLCD.fillCircle(298, 221, 7, TFT_GREY); // Big dot 72
     } 
     if (dot[54] == 1) {  // Check if dot 54 gobbled already
    myGLCD.fillCircle(298, 181, 2, TFT_GREY); // dot 54
     }    
  } 

} else


if (yP== 208) {  // if in Row 6  **********************************************************
  if (xP== 4) { // dot 61
     if (dot[55] == 1) {  // Check if dot 55 gobbled already
    myGLCD.fillCircle(19, 201, 7, TFT_GREY); // dot 55
     } 
     if (dot[62] == 1) {  // Check if dot 62 gobbled already
    myGLCD.fillCircle(42, 221, 2, TFT_GREY); // dot 62
     }   
  } else
  if (xP== 28) { // dot 62
     if (dot[61] == 1) {  // Check if dot 61 gobbled already
    myGLCD.fillCircle(19, 221, 2, TFT_GREY); // dot 61
     }  
     if (dot[63] == 1) {  // Check if dot 63 gobbled already
    myGLCD.fillCircle(65, 221, 2, TFT_GREY); // dot 63
     }      
  } else
  if (xP== 52) { // dot 63
     if (dot[64] == 1) {  // Check if dot 64 gobbled already
    myGLCD.fillCircle(88, 221, 2, TFT_GREY); // dot 64
     } 
     if (dot[62] == 1) {  // Check if dot 62 gobbled already
    myGLCD.fillCircle(42, 221, 2, TFT_GREY); // dot 62
     }  
     if (dot[56] == 1) {  // Check if dot 56 gobbled already
    myGLCD.fillCircle(77, 201, 2, TFT_GREY); // dot 56 
     }      
  } else
  if (xP== 74) { // dot 64
     if (dot[65] == 1) {  // Check if dot 65 gobbled already
    myGLCD.fillCircle(112, 221, 2, TFT_GREY); // dot 65
     } 
     if (dot[63] == 1) {  // Check if dot 63 gobbled already
    myGLCD.fillCircle(65, 221, 2, TFT_GREY); // dot 63
     }  
     if (dot[56] == 1) {  // Check if dot 56 gobbled already
    myGLCD.fillCircle(77, 201, 2, TFT_GREY); // dot 56 
     }     
  } else
  if (xP== 98) { // dot 65
     if (dot[64] == 1) {  // Check if dot 64 gobbled already
    myGLCD.fillCircle(88, 221, 2, TFT_GREY); // dot 64
     } 
     if (dot[66] == 1) {  // Check if dot 66 gobbled already
    myGLCD.fillCircle(136, 221, 2, TFT_GREY); // dot 66
     }    
  } else
  if (xP== 120) { // dot 66
     if (dot[65] == 1) {  // Check if dot 65 gobbled already
    myGLCD.fillCircle(112, 221, 2, TFT_GREY); // dot 65
     } 
     if (dot[57] == 1) {  // Check if dot 57 gobbled already
    myGLCD.fillCircle(136, 201, 2, TFT_GREY); // dot 57 
     }    
  } else
  if (xP== 168) { // dot 67
     if (dot[68] == 1) {  // Check if dot 68 gobbled already
    myGLCD.fillCircle(206, 221, 2, TFT_GREY); // dot 68
     } 
     if (dot[58] == 1) {  // Check if dot 58 gobbled already
    myGLCD.fillCircle(183, 201, 2, TFT_GREY); // dot 58
     }     
  } else
  if (xP== 192) { // dot 68
     if (dot[67] == 1) {  // Check if dot 67 gobbled already
    myGLCD.fillCircle(183, 221, 2, TFT_GREY); // dot 67
     } 
     if (dot[69] == 1) {  // Check if dot 69 gobbled already
    myGLCD.fillCircle(229, 221, 2, TFT_GREY); // dot 69
     }    
  } else
  if (xP== 216) { // dot 69
     if (dot[68] == 1) {  // Check if dot 68 gobbled already
    myGLCD.fillCircle(206, 221, 2, TFT_GREY); // dot 68
     } 
     if (dot[59] == 1) {  // Check if dot 59 gobbled already
    myGLCD.fillCircle(241, 201, 2, TFT_GREY); // dot 59
     }
     if (dot[70] == 1) {  // Check if dot 70 gobbled already
    myGLCD.fillCircle(252, 221, 2, TFT_GREY); // dot 70
     }    
  } else
  if (xP== 238) { // dot 70
     if (dot[69] == 1) {  // Check if dot 69 gobbled already
    myGLCD.fillCircle(229, 221, 2, TFT_GREY); // dot 69
     } 
     if (dot[59] == 1) {  // Check if dot 59 gobbled already
    myGLCD.fillCircle(241, 201, 2, TFT_GREY); // dot 59
     }
     if (dot[71] == 1) {  // Check if dot 71 gobbled already
    myGLCD.fillCircle(275, 221, 2, TFT_GREY); // dot 71
     }       
  } else
  if (xP== 262) { // dot 71
     if (dot[70] == 1) {  // Check if dot 70 gobbled already
    myGLCD.fillCircle(252, 221, 2, TFT_GREY); // dot 70
     }  
     if (dot[72] == 1) {  // Check if dot 72 gobbled already
    myGLCD.fillCircle(298, 221, 2, TFT_GREY); // dot 72
     }       
     if (dot[60] == 1) {  // Check if dot 60 gobbled already
    myGLCD.fillCircle(298, 201, 7, TFT_GREY); // Big dot 60
     }
  } else
  if (xP== 284) { // dot 72
     if (dot[71] == 1) {  // Check if dot 71 gobbled already
    myGLCD.fillCircle(275, 221, 2, TFT_GREY); // dot 71
     } 
     if (dot[60] == 1) {  // Check if dot 60 gobbled already
    myGLCD.fillCircle(298, 201, 7, TFT_GREY); // Big dot 60
     }     
  }
} else



// Check Columns


if (xP== 28) {  // if in Column 2
  if (yP== 66) { // dot 32
     if (dot[20] == 1) {  // Check if dot 20 gobbled already
    myGLCD.fillCircle(42, 60, 2, TFT_GREY); // dot 20
     }     
     if (dot[34] == 1) {  // Check if dot 34 gobbled already
    myGLCD.fillCircle(42, 100, 2, TFT_GREY); // dot 34
     }        
  } else
  if (yP== 86) { // dot 34
      if (dot[32] == 1) {  // Check if dot 32 gobbled already
    myGLCD.fillCircle(42, 80, 2, TFT_GREY); // dot 32
     }  
      if (dot[36] == 1) {  // Check if dot 36 gobbled already
    myGLCD.fillCircle(42, 120, 2, TFT_GREY); // dot 36
     }      
  } else
  if (yP== 106) { // dot 36
     if (dot[38] == 1) {  // Check if dot 38 gobbled already
    myGLCD.fillCircle(42, 140, 2, TFT_GREY); // dot 38
     }     
     if (dot[34] == 1) {  // Check if dot 34 gobbled already
    myGLCD.fillCircle(42, 100, 2, TFT_GREY); // dot 34
     }      
  } else
  if (yP== 126) { // dot 38
      if (dot[40] == 1) {  // Check if dot 40 gobbled already
    myGLCD.fillCircle(42, 160, 2, TFT_GREY); // dot 40
     } 
      if (dot[36] == 1) {  // Check if dot 36 gobbled already
    myGLCD.fillCircle(42, 120, 2, TFT_GREY); // dot 36
     }       
  } else
  if (yP== 146) { // dot 40
     if (dot[38] == 1) {  // Check if dot 38 gobbled already
    myGLCD.fillCircle(42, 140, 2, TFT_GREY); // dot 38
     }     
     if (dot[43] == 1) {  // Check if dot 43 gobbled already
    myGLCD.fillCircle(42, 181, 2, TFT_GREY); // dot 43
     }      
  } 

} else
if (xP== 262) {  // if in Column 7

  if (yP== 66) { // dot 33
      if (dot[30] == 1) {  // Check if dot 30 gobbled already
    myGLCD.fillCircle(275, 60, 2, TFT_GREY); // dot 30
     }   
      if (dot[35] == 1) {  // Check if dot 35 gobbled already
    myGLCD.fillCircle(275, 100, 2, TFT_GREY); // dot 35
     }   
  } else
  if (yP== 86) { // dot 35
      if (dot[33] == 1) {  // Check if dot 33 gobbled already
    myGLCD.fillCircle(275, 80, 2, TFT_GREY); // dot 33
     }  
      if (dot[37] == 1) {  // Check if dot 37 gobbled already
    myGLCD.fillCircle(275, 120, 2, TFT_GREY); // dot 37
     }     
  } else
  if (yP== 106) { // dot 37
      if (dot[35] == 1) {  // Check if dot 35 gobbled already
    myGLCD.fillCircle(275, 100, 2, TFT_GREY); // dot 35
     }  
      if (dot[39] == 1) {  // Check if dot 39 gobbled already
    myGLCD.fillCircle(275, 140, 2, TFT_GREY); // dot 39
     }      
  } else
  if (yP== 126) { // dot 39
      if (dot[37] == 1) {  // Check if dot 37 gobbled already
    myGLCD.fillCircle(275, 120, 2, TFT_GREY); // dot 37
     }
     if (dot[41] == 1) {  // Check if dot 41 gobbled already
    myGLCD.fillCircle(275, 160, 2, TFT_GREY); // dot 41
     }       
  } else
  if (yP== 146) { // dot 41
      if (dot[39] == 1) {  // Check if dot 39 gobbled already
    myGLCD.fillCircle(275, 140, 2, TFT_GREY); // dot 39
     } 
     if (dot[53] == 1) {  // Check if dot 53 gobbled already
    myGLCD.fillCircle(275, 181, 2, TFT_GREY); // dot 53
     }     
  } 
}



  
// increment Pacman Graphic Flag 0 = Closed, 1 = Medium Open, 2 = Wide Open
P=P+1; 
if(P==4){
  P=0; // Reset counter to closed
}

      
       
// Capture legacy direction to enable adequate blanking of trail
prevD = D;

/* Temp print variables for testing
  
  myGLCD.setColor(0, 0, 0);
  myGLCD.setBackColor(114, 198, 206);

  myGLCD.drawString(xT,100,140); // Print xP
  myGLCD.drawString(yT,155,140); // Print yP 
*/ 

// Check if Dot has been eaten before and incrementing score

// Check Rows

if (yP == 4) {  // if in Row 1 **********************************************************
  if (xP == 4) { // dot 1
     if (dot[1] == 1) {  // Check if dot gobbled already
        dot[1] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 28) { // dot 2
     if (dot[2] == 1) {  // Check if dot gobbled already
        dot[2] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 52) { // dot 3
     if (dot[3] == 1) {  // Check if dot gobbled already
        dot[3] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 74) { // dot 4
     if (dot[4] == 1) {  // Check if dot gobbled already
        dot[4] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 98) { // dot 5
     if (dot[5] == 1) {  // Check if dot gobbled already
        dot[5] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 120) { // dot 6
     if (dot[6] == 1) {  // Check if dot gobbled already
        dot[6] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 168) { // dot 7
     if (dot[7] == 1) {  // Check if dot gobbled already
        dot[7] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 192) { // dot 8
     if (dot[8] == 1) {  // Check if dot gobbled already
        dot[8] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 216) { // dot 9
     if (dot[9] == 1) {  // Check if dot gobbled already
        dot[9] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 238) { // dot 10
     if (dot[10] == 1) {  // Check if dot gobbled already
        dot[10] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 262) { // dot 11
     if (dot[11] == 1) {  // Check if dot gobbled already
        dot[11] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 284) { // dot 12
     if (dot[12] == 1) {  // Check if dot gobbled already
        dot[12] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } 

} else 
if (yP == 26) {  // if in Row 2  **********************************************************
  if (xP == 4) { // dot 13
     if (dot[13] == 1) {  // Check if dot gobbled already
        dot[13] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score 
        // Turn Ghost Blue if Pacman eats Big Dots
        fruiteatenpacman = true; // Turn Ghost blue      
     }     
  } else
  if (xP == 62) { // dot 14
     if (dot[14] == 1) {  // Check if dot gobbled already
        dot[14] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 120) { // dot 15
     if (dot[15] == 1) {  // Check if dot gobbled already
        dot[15] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 168) { // dot 16
     if (dot[16] == 1) {  // Check if dot gobbled already
        dot[16] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 228) { // dot 17
     if (dot[17] == 1) {  // Check if dot gobbled already
        dot[17] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 284) { // dot 18
     if (dot[18] == 1) {  // Check if dot gobbled already
        dot[18] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score
        // Turn Ghost Blue if Pacman eats Big Dots
        fruiteatenpacman = true; // Turn Ghost Blue       
     }     
  } 

} else
if (yP == 46) {  // if in Row 3  **********************************************************
  if (xP == 4) { // dot 19
     if (dot[19] == 1) {  // Check if dot gobbled already
        dot[19] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 28) { // dot 20
     if (dot[20] == 1) {  // Check if dot gobbled already
        dot[20] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 52) { // dot 21
     if (dot[21] == 1) {  // Check if dot gobbled already
        dot[21] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 74) { // dot 22
     if (dot[22] == 1) {  // Check if dot gobbled already
        dot[22] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 98) { // dot 23
     if (dot[23] == 1) {  // Check if dot gobbled already
        dot[23] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 120) { // dot 24
     if (dot[24] == 1) {  // Check if dot gobbled already
        dot[24] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 146) { // dot 25
     if (dot[25] == 1) {  // Check if dot gobbled already
        dot[25] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else

  if (xP == 168) { // dot 26
     if (dot[26] == 1) {  // Check if dot gobbled already
        dot[26] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 192) { // dot 27
     if (dot[27] == 1) {  // Check if dot gobbled already
        dot[27] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 216) { // dot 28
     if (dot[28] == 1) {  // Check if dot gobbled already
        dot[28] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 238) { // dot 29
     if (dot[29] == 1) {  // Check if dot gobbled already
        dot[29] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 262) { // dot 30
     if (dot[30] == 1) {  // Check if dot gobbled already
        dot[30] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 284) { // dot 31
     if (dot[31] == 1) {  // Check if dot gobbled already
        dot[31] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } 

} else
if (yP == 168) {  // if in Row 4  **********************************************************
  if (xP == 4) { // dot 42
     if (dot[42] == 1) {  // Check if dot gobbled already
        dot[42] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 28) { // dot 43
     if (dot[43] == 1) {  // Check if dot gobbled already
        dot[43] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 52) { // dot 44
     if (dot[44] == 1) {  // Check if dot gobbled already
        dot[44] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 74) { // dot 45
     if (dot[45] == 1) {  // Check if dot gobbled already
        dot[45] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 98) { // dot 46
     if (dot[46] == 1) {  // Check if dot gobbled already
        dot[46] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 120) { // dot 47
     if (dot[47] == 1) {  // Check if dot gobbled already
        dot[47] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 146) { // dot 48
     if (dot[48] == 1) {  // Check if dot gobbled already
        dot[48] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else

  if (xP == 168) { // dot 49
     if (dot[49] == 1) {  // Check if dot gobbled already
        dot[49] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 192) { // dot 50
     if (dot[50] == 1) {  // Check if dot gobbled already
        dot[50] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 216) { // dot 51
     if (dot[51] == 1) {  // Check if dot gobbled already
        dot[51] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 238) { // dot 52
     if (dot[52] == 1) {  // Check if dot gobbled already
        dot[52] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 262) { // dot 53
     if (dot[53] == 1) {  // Check if dot gobbled already
        dot[53] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 284) { // dot 54
     if (dot[54] == 1) {  // Check if dot gobbled already
        dot[54] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } 

} else
if (yP == 188) {  // if in Row 5  **********************************************************
  if (xP == 4) { // dot 55
     if (dot[55] == 1) {  // Check if dot gobbled already
        dot[55] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score
         // Turn Ghost Blue if Pacman eats Big Dots
        fruiteatenpacman = true; // Turn Ghost blue         
     }     
  } else
  if (xP == 62) { // dot 56
     if (dot[56] == 1) {  // Check if dot gobbled already
        dot[56] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 120) { // dot 57
     if (dot[57] == 1) {  // Check if dot gobbled already
        dot[57] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 168) { // dot 58
     if (dot[58] == 1) {  // Check if dot gobbled already
        dot[58] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 228) { // dot 59
     if (dot[59] == 1) {  // Check if dot gobbled already
        dot[59] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 284) { // dot 60
     if (dot[60] == 1) {  // Check if dot gobbled already
        dot[60] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score 
          // Turn Ghost Blue if Pacman eats Big Dots
        fruiteatenpacman = true; // Turn Ghost blue        
     }     
  } 

} else
if (yP == 208) {  // if in Row 6  **********************************************************
  if (xP == 4) { // dot 61
     if (dot[61] == 1) {  // Check if dot gobbled already
        dot[61] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 28) { // dot 62
     if (dot[62] == 1) {  // Check if dot gobbled already
        dot[62] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 52) { // dot 63
     if (dot[63] == 1) {  // Check if dot gobbled already
        dot[63] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 74) { // dot 64
     if (dot[64] == 1) {  // Check if dot gobbled already
        dot[64] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 98) { // dot 65
     if (dot[65] == 1) {  // Check if dot gobbled already
        dot[65] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 120) { // dot 66
     if (dot[66] == 1) {  // Check if dot gobbled already
        dot[66] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 168) { // dot 67
     if (dot[67] == 1) {  // Check if dot gobbled already
        dot[67] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 192) { // dot 68
     if (dot[68] == 1) {  // Check if dot gobbled already
        dot[68] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 216) { // dot 69
     if (dot[69] == 1) {  // Check if dot gobbled already
        dot[69] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 238) { // dot 70
     if (dot[70] == 1) {  // Check if dot gobbled already
        dot[70] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 262) { // dot 71
     if (dot[71] == 1) {  // Check if dot gobbled already
        dot[71] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 284) { // dot 72
     if (dot[72] == 1) {  // Check if dot gobbled already
        dot[72] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } 

}   



// Check Columns


if (xP == 28) {  // if in Column 2
  if (yP == 66) { // dot 32
     if (dot[32] == 1) {  // Check if dot gobbled already
        dot[32] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (yP == 86) { // dot 34
     if (dot[34] == 1) {  // Check if dot gobbled already
        dot[34] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (yP == 108) { // dot 36
     if (dot[36] == 1) {  // Check if dot gobbled already
        dot[36] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (yP == 126) { // dot 38
     if (dot[38] == 1) {  // Check if dot gobbled already
        dot[38] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (yP == 146) { // dot 40
     if (dot[40] == 1) {  // Check if dot gobbled already
        dot[40] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } 

} else
if (xP == 262) {  // if in Column 7
  if (yP == 66) { // dot 33
     if (dot[33] == 1) {  // Check if dot gobbled already
        dot[33] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (yP == 86) { // dot 35
     if (dot[35] == 1) {  // Check if dot gobbled already
        dot[35] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (yP == 106) { // dot 37
     if (dot[37] == 1) {  // Check if dot gobbled already
        dot[37] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (yP == 126) { // dot 39
     if (dot[39] == 1) {  // Check if dot gobbled already
        dot[39] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (yP == 146) { // dot 41
     if (dot[41] == 1) {  // Check if dot gobbled already
        dot[41] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } 
}

//Pacman wandering Algorithm 
// Note: Keep horizontal and vertical coordinates even numbers only to accomodate increment rate and starting point
// Pacman direction variable D where 0 = right, 1 = down, 2 = left, 3 = up

//****************************************************************************************************************************
//Right hand motion and ***************************************************************************************************
//****************************************************************************************************************************



if(D == 0){
// Increment xP and then test if any decisions required on turning up or down
  xP = xP+cstep; 

 // There are four horizontal rows that need rules

  // First Horizontal Row
  if (yP == 4) { 

    // Past first block decide to continue or go down
    if (xP == 62) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
    // Past second block only option is down
    if (xP == 120) { 
         D = 1; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
    // Past third block decide to continue or go down
    if (xP == 228) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
    // Past fourth block only option is down
    if (xP == 284) { 
         D = 1; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
  }

  // 2nd Horizontal Row
  if (yP == 46) { 

    // Past upper doorway on left decide to continue or go down
    if (xP == 28) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

    // Past first block decide to continue or go up
    if (xP == 62) { 
      direct = random(4); // generate random number between 0 and 3
      if (direct == 3){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
     // Past Second block decide to continue or go up
    if (xP == 120) { 
      direct = random(4); // generate random number between 0 and 3
      if (direct == 3){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

     // Past Mid Wall decide to continue or go up
    if (xP == 168) { 
      direct = random(4); // generate random number between 0 and 3
      if (direct == 3){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

    // Past third block decide to continue or go up
    if (xP == 228) { 
      direct = random(4); // generate random number between 0 and 3
      if (direct == 3){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

    // Past last clock digit decide to continue or go down
    if (xP == 262) { 
      direct = random(2); // generate random number between 0 and 2
      if (direct == 1){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

    // Past fourth block only option is up
    if (xP == 284) { 
         D = 3; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
  }

  // LHS Door Horizontal Row
  if (yP == 108) { 

    // Past upper doorway on left decide to go up or go down
    if (xP == 28) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = 1; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { D = 3;}    
    }
  }

  // 3rd Horizontal Row
  if (yP == 168) { 

    // Past lower doorway on left decide to continue or go up
    if (xP == 28) { 
      direct = random(4); // generate random number between 0 and 3
      if (direct == 3){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

    // Past first block decide to continue or go down
    if (xP == 62) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
     // Past Second block decide to continue or go down
    if (xP == 120) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

     // Past Mid Wall decide to continue or go down
    if (xP == 168) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

    // Past third block decide to continue or go down
    if (xP == 228) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

    // Past last clock digit decide to continue or go up
    if (xP == 262) { 
      direct = random(4); // generate random number between 0 and 3
      if (direct == 3){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

    // Past fourth block only option is down
    if (xP == 284) { 
         D = 1; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
  }
 
  
  // 4th Horizontal Row
  if (yP == 208) { 

    // Past first block decide to continue or go up
    if (xP == 62) { 
      direct = random(4); // generate random number between 0 and 3
      if (direct == 3){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
    // Past second block only option is up
    if (xP == 120) { 
         D = 3; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
    // Past third block decide to continue or go up
    if (xP == 228) { 
      direct = random(4); // generate random number between 0 and 3
      if (direct == 3){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
    // Past fourth block only option is up
    if (xP == 284) { 
         D = 3; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
   }
}

//****************************************************************************************************************************
//Left hand motion **********************************************************************************************************
//****************************************************************************************************************************

else if(D == 2){
// Increment xP and then test if any decisions required on turning up or down
  xP = xP-cstep; 

/* Temp print variables for testing
  
  myGLCD.setColor(0, 0, 0);
  myGLCD.setBackColor(114, 198, 206);
  myGLCD.drawString(xP,80,165); // Print xP
  myGLCD.drawString(yP,110,165); // Print yP
*/

 // There are four horizontal rows that need rules

  // First Horizontal Row  ******************************
  if (yP == 4) { 

     // Past first block only option is down
    if (xP == 4) { 
         D = 1; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
 
    // Past second block decide to continue or go down
    if (xP == 62) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
    // Past third block only option is down
    if (xP == 168) { 
         D = 1; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
    // Past fourth block decide to continue or go down
    if (xP == 228) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
  }

  // 2nd Horizontal Row ******************************
  if (yP == 46) { 

    // Meet LHS wall only option is up
    if (xP == 4) { 
         D = 3; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
    // Meet upper doorway on left decide to continue or go down
    if (xP == 28) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

    // Meet first block decide to continue or go up
    if (xP == 62) { 
      direct = random(4); // generate random number between 0 and 3
      if (direct == 3){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
     // Meet Second block decide to continue or go up
    if (xP == 120) { 
      direct = random(4); // generate random number between 0 and 3
      if (direct == 3){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

     // Meet Mid Wall decide to continue or go up
    if (xP == 168) { 
      direct = random(4); // generate random number between 0 and 3
      if (direct == 3){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

    // Meet third block decide to continue or go up
    if (xP == 228) { 
      direct = random(4); // generate random number between 0 and 3
      if (direct == 3){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

    // Meet last clock digit decide to continue or go down
    if (xP == 262) { 
      direct = random(2); // generate random number between 0 and 3
      if (direct == 1){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

  }
  

  // 3rd Horizontal Row ******************************
  if (yP == 168) { 

    // Meet LHS lower wall only option is down
    if (xP == 4) { 
         D = 1; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }

    // Meet lower doorway on left decide to continue or go up
    if (xP == 28) { 
      direct = random(4); // generate random number between 0 and 3
      if (direct == 3){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

    // Meet first block decide to continue or go down
    if (xP == 62) { 
      direct = random(2); // generate random number between 0 and 3
      if (direct == 1){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
     // Meet Second block decide to continue or go down
    if (xP == 120) { 
      direct = random(2); // generate random number between 0 and 3
      if (direct == 1){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

     // Meet Mid Wall decide to continue or go down
    if (xP == 168) { 
      direct = random(2); // generate random number between 0 and 3
      if (direct == 1){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

    // Meet third block decide to continue or go down
    if (xP == 228) { 
      direct = random(2); // generate random number between 0 and 3
      if (direct == 1){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

    // Meet last clock digit above decide to continue or go up
    if (xP == 262) { 
      direct = random(4); // generate random number between 0 and 3
      if (direct == 3){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    
    }
    
  }
   // 4th Horizontal Row ******************************
  if (yP == 208) { 

    // Meet LHS wall only option is up
    if (xP == 4) { 
         D = 3; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }  
    // Meet first block decide to continue or go up
    if (xP == 62) { 
      direct = random(4); // generate random number between 0 and 3
      if (direct == 3){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
    // Meet bottom divider wall only option is up
    if (xP == 168) { 
         D = 3; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
    // Meet 3rd block decide to continue or go up
    if (xP == 228) { 
      direct = random(4); // generate random number between 0 and 3
      if (direct == 3){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
   
  
  }
}  
  


//****************************************************************************************************************************
//Down motion **********************************************************************************************************
//****************************************************************************************************************************

else if(D == 1){
// Increment yP and then test if any decisions required on turning up or down
  yP = yP+cstep; 

/* Temp print variables for testing
  
  myGLCD.setColor(0, 0, 0);
  myGLCD.setBackColor(114, 198, 206);
  myGLCD.drawString(xP,80,165); // Print xP
  myGLCD.drawString(yP,110,165); // Print yP
*/

 // There are vertical rows that need rules

  // First Vertical Row  ******************************
  if (xP == 4) { 

     // Past first block only option is right
    if (yP == 46) { 
         D = 0; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
 
    // Towards bottom wall only option right
    if (yP == 208) { 
         D = 0; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    } 
  }

  // 2nd Vertical Row ******************************
  if (xP == 28) { 

    // Meet bottom doorway on left decide to go left or go right
    if (yP == 168) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { D = 0;}    
    }
  }

  // 3rd Vertical Row ******************************
  if (xP == 62) { 

    // Meet top lh digit decide to go left or go right
    if (yP == 46) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { D = 0;}    
    }

    // Meet top lh digit decide to go left or go right
    if (yP == 208) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { D = 0;}    
    }
  }

  // 5th Vertical Row ******************************
  if (xP == 120) { 

    // Meet top lh digit decide to go left or go right
    if (yP == 46) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { D = 0;}    
    }

    // Meet bottom wall only opgion to go left
    if (yP == 208) { 
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up   
    }
  }

  // 6th Vertical Row ******************************
  if (xP == 168) { 

    // Meet top lh digit decide to go left or go right
    if (yP == 46) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { D = 0;}    
    }

    // Meet bottom wall only opgion to go right
    if (yP == 208) { 
         D = 0; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up   
    }
  }

  // 8th Vertical Row ******************************
  if (xP == 228) { 

    // Meet top lh digit decide to go left or go right
    if (yP == 46) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { D = 0;}    
    }

    // Meet bottom wall
    if (yP == 208) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { D = 0;}    
    }
  }

  // 9th Vertical Row ******************************
  if (xP == 262) { 

    // Meet bottom right doorway  decide to go left or go right
    if (yP == 168) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { D = 0;}    
    }
  }

  // 10th Vertical Row  ******************************
  if (xP == 284) { 

     // Past first block only option is left
    if (yP == 46) { 
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
 
    // Towards bottom wall only option right
    if (yP == 208) { 
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    } 
  } 
}  

//****************************************************************************************************************************
//Up motion **********************************************************************************************************
//****************************************************************************************************************************

else if(D == 3){
// Decrement yP and then test if any decisions required on turning up or down
  yP = yP-cstep; 

/* Temp print variables for testing
  
  myGLCD.setColor(0, 0, 0);
  myGLCD.setBackColor(114, 198, 206);
  myGLCD.drawString(xP,80,165); // Print xP
  myGLCD.drawString(yP,110,165); // Print yP
*/


 // There are vertical rows that need rules

  // First Vertical Row  ******************************
  if (xP == 4) { 

     // Past first block only option is right
    if (yP == 4) { 
         D = 0; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
 
    // Towards bottom wall only option right
    if (yP == 168) { 
         D = 0; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    } 
  }

  // 2nd Vertical Row ******************************
  if (xP == 28) { 

    // Meet top doorway on left decide to go left or go right
    if (yP == 46) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { D = 0;}    
    }
  }

  // 3rd Vertical Row ******************************
  if (xP == 62) { 

    // Meet top lh digit decide to go left or go right
    if (yP == 4) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { D = 0;}    
    }

    // Meet top lh digit decide to go left or go right
    if (yP == 168) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { D = 0;}    
    }
  }

  // 5th Vertical Row ******************************
  if (xP == 120) { 

    // Meet bottom lh digit decide to go left or go right
    if (yP == 168) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { D = 0;}    
    }

    // Meet top wall only opgion to go left
    if (yP == 4) { 
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up   
    }
  }

  // 6th Vertical Row ******************************
  if (xP == 168) { 

    // Meet bottom lh digit decide to go left or go right
    if (yP == 168) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { D = 0;}    
    }

    // Meet top wall only opgion to go right
    if (yP == 4) { 
         D = 0; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up   
    }
  }

  // 8th Vertical Row ******************************
  if (xP == 228) { 

    // Meet bottom lh digit decide to go left or go right
    if (yP == 168) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { D = 0;}    
    }

    // Meet top wall go left or right
    if (yP == 4) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { D = 0;}    
    }
  }

  // 9th Vertical Row ******************************
  if (xP == 262) { 

    // Meet top right doorway  decide to go left or go right
    if (yP == 46) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { D = 0;}    
    }
  }

  // 10th Vertical Row  ******************************
  if (xP == 284) { 

     // Past first block only option is left
    if (yP == 168) { 
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
 
    // Towards top wall only option right
    if (yP == 4) { 
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    } 
  } 
 }  
}

}

void displayghost(){ // Draw Ghost in position on screen
//******************************************************************************************************************
//Ghost ; 
// Note: Keep horizontal and verticalcoordinates even numbers only to accomodateincrement rate and starting point
// Ghost direction variable  D where 0 = right, 1 = down, 2 = left, 3 = up

//****************************************************************************************************************************
//Right hand motion **********************************************************************************************************
//****************************************************************************************************************************


// If ghost captured then ghost dissapears until reset
if ((fruiteatenpacman == true)&&(abs(xG-xP)<=5)&&(abs(yG-yP)<=5)){ 
  
  if (ghostlost == false){
    pacmanscore++;
    pacmanscore++;  
  }

  ghostlost = true;

  dly = gamespeed; // slowdown now only drawing one item
  }
  
  
if (ghostlost == false){ // only draw ghost if still alive

drawGhost(xG,yG,GD,prevGD); // Draws Ghost at these coordinates


// If Ghost is on a dot then print the adjacent dots if they are valid

//  myGLCD.setColor(200, 200, 200);
  
// Check Rows

if (yG == 4) {  // if in Row 1 **********************************************************
  if (xG == 4) { // dot 1
     if (dot[2] == 1) {  // Check if dot 2 gobbled already
    myGLCD.fillCircle(42, 19, 2, TFT_GREY); // dot 2
     }    
      if (dot[13] == 1) {  // Check if dot 13 gobbled already
    myGLCD.fillCircle(19, 40, 7, TFT_GREY); // Big dot 13
     }    

  } else
  if (xG == 28) { // dot 2
     if (dot[1] == 1) {  // Check if dot 1 gobbled already
    myGLCD.fillCircle(19, 19, 2, TFT_GREY); // dot 1
     }    
      if (dot[3] == 1) {  // Check if dot 3 gobbled already
    myGLCD.fillCircle(65, 19, 2, TFT_GREY); // dot 3
     }    

  } else
  if (xG == 52) { // dot 3
     if (dot[2] == 1) {  // Check if dot 2 gobbled already
    myGLCD.fillCircle(42, 19, 2, TFT_GREY); // dot 2
     }    
      if (dot[4] == 1) {  // Check if dot 4 gobbled already
    myGLCD.fillCircle(88, 19, 2, TFT_GREY); // dot 4
     } 
      if (dot[14] == 1) {  // Check if dot 14 gobbled already
    myGLCD.fillCircle(77, 40, 2, TFT_GREY); // dot 14
     }   
  } else
  if (xG == 74) { // dot 4
     if (dot[3] == 1) {  // Check if dot 3 gobbled already
    myGLCD.fillCircle(65, 19, 2, TFT_GREY); // dot 3
     }    
      if (dot[5] == 1) {  // Check if dot 5 gobbled already
    myGLCD.fillCircle(112, 19, 2, TFT_GREY); // dot 5
     }   
      if (dot[14] == 1) {  // Check if dot 14 gobbled already
    myGLCD.fillCircle(77, 40, 2, TFT_GREY); // dot 14
     }    
  } else
  if (xG == 98) { // dot 5
     if (dot[4] == 1) {  // Check if dot 3 gobbled already
    myGLCD.fillCircle(88, 19, 2, TFT_GREY); // dot 4
     }    
      if (dot[6] == 1) {  // Check if dot 5 gobbled already
    myGLCD.fillCircle(136, 19, 2, TFT_GREY); // dot 6
     }     
  } else
  if (xG == 120) { // dot 6
     if (dot[5] == 1) {  // Check if dot 5 gobbled already
    myGLCD.fillCircle(136, 19, 2, TFT_GREY); // dot 5
     }    
      if (dot[15] == 1) {  // Check if dot 15 gobbled already
    myGLCD.fillCircle(136, 40, 2, TFT_GREY); // dot 15
     }     
  } else
 

 if (xG == 168) { // dot 7
      if (dot[16] == 1) {  // Check if dot 16 gobbled already
    myGLCD.fillCircle(183, 40, 2, TFT_GREY); // dot 16
     }    
      if (dot[8] == 1) {  // Check if dot 8 gobbled already
    myGLCD.fillCircle(206, 19, 2, TFT_GREY); // dot 8
     }     
  } else
  if (xG == 192) { // dot 8
      if (dot[7] == 1) {  // Check if dot 7 gobbled already
    myGLCD.fillCircle(183, 19, 2, TFT_GREY); // dot 7
     }    
      if (dot[9] == 1) {  // Check if dot 9 gobbled already
    myGLCD.fillCircle(229, 19, 2, TFT_GREY); // dot 9
     }    
  } else
  if (xG == 216) { // dot 9
      if (dot[10] == 1) {  // Check if dot 10 gobbled already
    myGLCD.fillCircle(252, 19, 2, TFT_GREY); // dot 10
     }    
      if (dot[8] == 1) {  // Check if dot 8 gobbled already
    myGLCD.fillCircle(206, 19, 2, TFT_GREY); // dot 8
     }      
      if (dot[17] == 1) {  // Check if dot 17 gobbled already
    myGLCD.fillCircle(241, 40, 2, TFT_GREY); // dot 17
     }   
 } else
  if (xG == 238) { // dot 10
      if (dot[11] == 1) {  // Check if dot 11 gobbled already
    myGLCD.fillCircle(275, 19, 2, TFT_GREY); // dot 11
     }    
      if (dot[9] == 1) {  // Check if dot 9 gobbled already
    myGLCD.fillCircle(229, 19, 2, TFT_GREY); // dot 9
     }      
      if (dot[17] == 1) {  // Check if dot 17 gobbled already
    myGLCD.fillCircle(241, 40, 2, TFT_GREY); // dot 17
     }   
  } else
  if (xG == 262) { // dot 11
      if (dot[10] == 1) {  // Check if dot 10 gobbled already
    myGLCD.fillCircle(252, 19, 2, TFT_GREY); // dot 10
     }    
      if (dot[12] == 1) {  // Check if dot 12 gobbled already
    myGLCD.fillCircle(298, 19, 2, TFT_GREY); // dot 12
     }    
      if (dot[18] == 1) {  // Check if Big dot 18 gobbled already
    myGLCD.fillCircle(298, 40, 7, TFT_GREY); // dot 18
     } 
  } else
  if (xG == 284) { // dot 12
      if (dot[11] == 1) {  // Check if dot 11 gobbled already
    myGLCD.fillCircle(275, 19, 2, TFT_GREY); // dot 11
     }    
      if (dot[18] == 1) {  // Check if dot 18 gobbled already
    myGLCD.fillCircle(298, 40, 7, TFT_GREY); // dot 18
     }  
  }
} else 
if (yG == 26) {  // if in Row 2  **********************************************************
  if (xG == 4) { // dot 13
     if (dot[1] == 1) {  // Check if dot 1 gobbled already
    myGLCD.fillCircle(19, 19, 2, TFT_GREY); // dot 1
     }    
      if (dot[19] == 1) {  // Check if dot 19 gobbled already
    myGLCD.fillCircle(19, 60, 2, TFT_GREY); //  dot 19
     }   
  } else
  
    if (xG == 62) { // dot 14
      if (dot[3] == 1) {  // Check if dot 3 gobbled already
    myGLCD.fillCircle(65, 19, 2, TFT_GREY); // dot 3
     }   
         if (dot[4] == 1) {  // Check if dot 4 gobbled already
    myGLCD.fillCircle(88, 19, 2, TFT_GREY); // dot 4
     } 
         if (dot[21] == 1) {  // Check if dot 21 gobbled already
    myGLCD.fillCircle(65, 60, 2, TFT_GREY); // dot 21
     }   
      if (dot[22] == 1) {  // Check if dot 22 gobbled already
    myGLCD.fillCircle(88, 60, 2, TFT_GREY); // dot 22
     }    
     
  } else
  
  if (xG == 120) { // dot 15
     if (dot[24] == 1) {  // Check if dot 24 gobbled already
    myGLCD.fillCircle(136, 60, 2, TFT_GREY); // dot 24
     }    
      if (dot[6] == 1) {  // Check if dot 6 gobbled already
    myGLCD.fillCircle(136, 19, 2, TFT_GREY); // dot 6
     }      
  } else
  if (xG == 168) { // dot 16
      if (dot[7] == 1) {  // Check if dot 7 gobbled already
    myGLCD.fillCircle(183, 19, 2, TFT_GREY); // dot 7
     }    
      if (dot[26] == 1) {  // Check if dot 26 gobbled already
    myGLCD.fillCircle(183, 60, 2, TFT_GREY); // dot 26
     }          
  } else
    if (xG == 228) { // dot 17
      if (dot[9] == 1) {  // Check if dot 9 gobbled already
    myGLCD.fillCircle(229, 19, 2, TFT_GREY); // dot 9
     }      
       if (dot[10] == 1) {  // Check if dot 10 gobbled already
    myGLCD.fillCircle(252, 19, 2, TFT_GREY); // dot 10
     }  
      if (dot[28] == 1) {  // Check if dot 28 gobbled already
    myGLCD.fillCircle(229, 60, 2, TFT_GREY); // dot 28
     }  
       if (dot[29] == 1) {  // Check if dot 29 gobbled already
    myGLCD.fillCircle(252, 60, 2, TFT_GREY); // dot 29
     }     
     
  } else
  if (xG == 284) { // dot 18
      if (dot[31] == 1) {  // Check if dot 31 gobbled already
    myGLCD.fillCircle(298, 60, 2, TFT_GREY); // dot 31
     }    
      if (dot[12] == 1) {  // Check if dot 12 gobbled already
    myGLCD.fillCircle(298, 19, 2, TFT_GREY); // dot 12
     }  
  }
} else
if (yG == 46) {  // if in Row 3  **********************************************************
  if (xG == 4) { // dot 19
     if (dot[20] == 1) {  // Check if dot 20 gobbled already
    myGLCD.fillCircle(42, 60, 2, TFT_GREY); // dot 20
     }    
      if (dot[13] == 1) {  // Check if dot 13 gobbled already
    myGLCD.fillCircle(19, 40, 7, TFT_GREY); // Big dot 13
     }  
  } else
  if (xG == 28) { // dot 20
     if (dot[19] == 1) {  // Check if dot 19 gobbled already
    myGLCD.fillCircle(19, 60, 2, TFT_GREY); // dot 19
     }    
      if (dot[21] == 1) {  // Check if dot 21 gobbled already
    myGLCD.fillCircle(65, 60, 2, TFT_GREY); // dot 21
     }   
      if (dot[32] == 1) {  // Check if dot 32 gobbled already
    myGLCD.fillCircle(42, 80, 2, TFT_GREY); // dot 32
     }
      if (dot[13] == 1) {  // Check if dot 13 gobbled already
    myGLCD.fillCircle(19, 40, 7, TFT_GREY); // Big dot 13
     }     
  } else
  if (xG == 52) { // dot 21
     if (dot[20] == 1) {  // Check if dot 20 gobbled already
    myGLCD.fillCircle(42, 60, 2, TFT_GREY); // dot 20
     }    
      if (dot[22] == 1) {  // Check if dot 22 gobbled already
    myGLCD.fillCircle(88, 60, 2, TFT_GREY); // dot 22
     } 
      if (dot[14] == 1) {  // Check if dot 14 gobbled already
    myGLCD.fillCircle(77, 40, 2, TFT_GREY); // dot 14
     }         
  } else
  if (xG == 74) { // dot 22
      if (dot[21] == 1) {  // Check if dot 21 gobbled already
    myGLCD.fillCircle(65, 60, 2, TFT_GREY); // dot 21
     }    
      if (dot[23] == 1) {  // Check if dot 23 gobbled already
    myGLCD.fillCircle(112, 60, 2, TFT_GREY); // dot 23
     } 
      if (dot[14] == 1) {  // Check if dot 14 gobbled already
    myGLCD.fillCircle(77, 40, 2, TFT_GREY); // dot 14
     }    
  } else
  if (xG == 98) { // dot 23
     if (dot[24] == 1) {  // Check if dot 24 gobbled already
    myGLCD.fillCircle(136, 60, 2, TFT_GREY); // dot 24
     }    
      if (dot[22] == 1) {  // Check if dot 22 gobbled already
    myGLCD.fillCircle(88, 60, 2, TFT_GREY); // dot 22
     }  
    
  } else
  if (xG == 120) { // dot 24
      if (dot[25] == 1) {  // Check if dot 25 gobbled already
    myGLCD.fillCircle(160, 60, 2, TFT_GREY); // dot 25
     }    
      if (dot[23] == 1) {  // Check if dot 23 gobbled already
    myGLCD.fillCircle(112, 60, 2, TFT_GREY); // dot 23
     }
      if (dot[15] == 1) {  // Check if dot 15 gobbled already
    myGLCD.fillCircle(136, 40, 2, TFT_GREY); // dot 15
     }        
  } else
  if (xG == 146) { // dot 25
     if (dot[24] == 1) {  // Check if dot 24 gobbled already
    myGLCD.fillCircle(136, 60, 2, TFT_GREY); // dot 24
     }    
      if (dot[26] == 1) {  // Check if dot 26 gobbled already
    myGLCD.fillCircle(183, 60, 2, TFT_GREY); // dot 26
     }    
  } else
  if (xG == 168) { // dot 26
      if (dot[25] == 1) {  // Check if dot 25 gobbled already
    myGLCD.fillCircle(160, 60, 2, TFT_GREY); // dot 25
     }    
      if (dot[27] == 1) {  // Check if dot 27 gobbled already
    myGLCD.fillCircle(206, 60, 2, TFT_GREY); // dot 27
     }
      if (dot[16] == 1) {  // Check if dot 16 gobbled already
    myGLCD.fillCircle(183, 40, 2, TFT_GREY); // dot 16
     }    
  } else
  if (xG == 192) { // dot 27
     if (dot[28] == 1) {  // Check if dot 28 gobbled already
    myGLCD.fillCircle(229, 60, 2, TFT_GREY); // dot 28
     }    
      if (dot[26] == 1) {  // Check if dot 26 gobbled already
    myGLCD.fillCircle(183, 60, 2, TFT_GREY); // dot 26
     }      
  } else
  if (xG == 216) { // dot 28
      if (dot[29] == 1) {  // Check if dot 29 gobbled already
    myGLCD.fillCircle(252, 60, 2, TFT_GREY); // dot 29
     }    
      if (dot[27] == 1) {  // Check if dot 27 gobbled already
    myGLCD.fillCircle(206, 60, 2, TFT_GREY); // dot 27
     }      
      if (dot[17] == 1) {  // Check if dot 17 gobbled already
    myGLCD.fillCircle(241, 40, 2, TFT_GREY); // dot 17
     }   
  } else
  if (xG == 238) { // dot 29
     if (dot[28] == 1) {  // Check if dot 28 gobbled already
    myGLCD.fillCircle(229, 60, 2, TFT_GREY); // dot 28
     }    
      if (dot[30] == 1) {  // Check if dot 30 gobbled already
    myGLCD.fillCircle(275, 60, 2, TFT_GREY); // dot 30
     }      
      if (dot[17] == 1) {  // Check if dot 17 gobbled already
    myGLCD.fillCircle(241, 40, 2, TFT_GREY); // dot 17
     }   
   
  } else
  if (xG == 262) { // dot 30
      if (dot[29] == 1) {  // Check if dot 29 gobbled already
    myGLCD.fillCircle(252, 60, 2, TFT_GREY); // dot 29
     }    
      if (dot[33] == 1) {  // Check if dot 33 gobbled already
    myGLCD.fillCircle(275, 80, 2, TFT_GREY); // dot 33
     }      
      if (dot[31] == 1) {  // Check if dot 31 gobbled already
    myGLCD.fillCircle(298, 60, 2, TFT_GREY); // dot 31
     }  
   if (dot[18] == 1) {  // Check if Big dot 18 gobbled already
    myGLCD.fillCircle(298, 40, 7, TFT_GREY); // dot 18
   }  
  } else
  if (xG == 284) { // dot 31
   if (dot[18] == 1) {  // Check if Big dot 18 gobbled already
    myGLCD.fillCircle(298, 40, 7, TFT_GREY); // dot 18
   }     
   if (dot[30] == 1) {  // Check if dot 30 gobbled already
    myGLCD.fillCircle(275, 60, 2, TFT_GREY); // dot 30
   } 
  }
} else

if (yG == 168) {  // if in Row 4  **********************************************************
  if (xG == 4) { // dot 42
     if (dot[43] == 1) {  // Check if dot 43 gobbled already
    myGLCD.fillCircle(42, 181, 2, TFT_GREY); // dot 43
     }     
     if (dot[55] == 1) {  // Check if dot 55 gobbled already
    myGLCD.fillCircle(19, 201, 7, TFT_GREY); // dot 55
     }     
  } else
  if (xG == 28) { // dot 43
     if (dot[42] == 1) {  // Check if dot 42 gobbled already
    myGLCD.fillCircle(19, 181, 2, TFT_GREY); // dot 42
     }     
     if (dot[44] == 1) {  // Check if dot 44 gobbled already
    myGLCD.fillCircle(65, 181, 2, TFT_GREY); // dot 44
     }   
      if (dot[40] == 1) {  // Check if dot 40 gobbled already
    myGLCD.fillCircle(42, 160, 2, TFT_GREY); // dot 40
     }       
  } else
  if (xG == 52) { // dot 44
     if (dot[43] == 1) {  // Check if dot 43 gobbled already
    myGLCD.fillCircle(42, 181, 2, TFT_GREY); // dot 43
     }     
     if (dot[45] == 1) {  // Check if dot 45 gobbled already
    myGLCD.fillCircle(88, 181, 2, TFT_GREY); // dot 45
     } 
     if (dot[56] == 1) {  // Check if dot 56 gobbled already
    myGLCD.fillCircle(77, 201, 2, TFT_GREY); // dot 56 
     }    
  } else
  if (xG == 74) { // dot 45
     if (dot[46] == 1) {  // Check if dot 46 gobbled already
    myGLCD.fillCircle(112, 181, 2, TFT_GREY); // dot 46
     }     
     if (dot[44] == 1) {  // Check if dot 44 gobbled already
    myGLCD.fillCircle(65, 181, 2, TFT_GREY); // dot 44
     } 
     if (dot[56] == 1) {  // Check if dot 56 gobbled already
    myGLCD.fillCircle(77, 201, 2, TFT_GREY); // dot 56 
     }    
     
  } else
  if (xG == 98) { // dot 46
     if (dot[47] == 1) {  // Check if dot 47 gobbled already
    myGLCD.fillCircle(136, 181, 2, TFT_GREY); // dot 47
     }     
     if (dot[45] == 1) {  // Check if dot 45 gobbled already
    myGLCD.fillCircle(88, 181, 2, TFT_GREY); // dot 45
     }  
  } else
  if (xG == 120) { // dot 47
     if (dot[48] == 1) {  // Check if dot 48 gobbled already
    myGLCD.fillCircle(160, 181, 2, TFT_GREY); // dot 48
    // Draw fruit
    if ((fruitdrawn == true)&&(fruitgone == false)){ // draw fruit again
        drawicon(146, 168, fruit); //   draw fruit 
    }
     }     
     if (dot[46] == 1) {  // Check if dot 46 gobbled already
    myGLCD.fillCircle(112, 181, 2, TFT_GREY); // dot 46
     } 
     if (dot[57] == 1) {  // Check if dot 57 gobbled already
    myGLCD.fillCircle(136, 201, 2, TFT_GREY); // dot 57 
     }
    // Draw fruit
    if ((fruitdrawn == true)&&(fruitgone == false)){ // draw fruit again
        drawicon(146, 168, fruit); //   draw fruit 
    } 
  } else
  if (xG == 146) { // dot 48
     if (dot[47] == 1) {  // Check if dot 47 gobbled already
    myGLCD.fillCircle(136, 181, 2, TFT_GREY); // dot 47
     }     
     if (dot[49] == 1) {  // Check if dot 49 gobbled already
    myGLCD.fillCircle(183, 181, 2, TFT_GREY); // dot 49
     }  
  } else

  if (xG == 168) { // dot 49
     if (dot[48] == 1) {  // Check if dot 48 gobbled already
    myGLCD.fillCircle(160, 181, 2, TFT_GREY); // dot 48
     }     
    // Draw fruit
    if ((fruitdrawn == true)&&(fruitgone == false)){ // draw fruit again
        drawicon(146, 168, fruit); //   draw fruit 
    }
     if (dot[50] == 1) {  // Check if dot 50 gobbled already
    myGLCD.fillCircle(206, 181, 2, TFT_GREY); // dot 50
     } 
     if (dot[58] == 1) {  // Check if dot 58 gobbled already
    myGLCD.fillCircle(183, 201, 2, TFT_GREY); // dot 58
     }        
    // Draw fruit
    if ((fruitdrawn == true)&&(fruitgone == false)){ // draw fruit again
        drawicon(146, 168, fruit); //   draw fruit 
    }
  } else
  if (xG == 192) { // dot 50
     if (dot[51] == 1) {  // Check if dot 51 gobbled already
    myGLCD.fillCircle(229, 181, 2, TFT_GREY); // dot 51
     }     
     if (dot[49] == 1) {  // Check if dot 49 gobbled already
    myGLCD.fillCircle(183, 181, 2, TFT_GREY); // dot 49
     }      
  } else
  if (xG == 216) { // dot 51
     if (dot[50] == 1) {  // Check if dot 50 gobbled already
    myGLCD.fillCircle(206, 181, 2, TFT_GREY); // dot 50
     }    
     if (dot[52] == 1) {  // Check if dot 52 gobbled already
    myGLCD.fillCircle(252, 181, 2, TFT_GREY); // dot 52
     } 
     if (dot[59] == 1) {  // Check if dot 59 gobbled already
    myGLCD.fillCircle(241, 201, 2, TFT_GREY); // dot 59
     }     
  } else
  if (xG == 238) { // dot 52
     if (dot[53] == 1) {  // Check if dot 53 gobbled already
    myGLCD.fillCircle(275, 181, 2, TFT_GREY); // dot 53
     }    
     if (dot[51] == 1) {  // Check if dot 51 gobbled already
    myGLCD.fillCircle(229, 181, 2, TFT_GREY); // dot 51
     }  
     if (dot[59] == 1) {  // Check if dot 59 gobbled already
    myGLCD.fillCircle(241, 201, 2, TFT_GREY); // dot 59
     }     
  } else
 if (xG == 262) { // dot 53
     if (dot[41] == 1) {  // Check if dot 41 gobbled already
    myGLCD.fillCircle(275, 160, 2, TFT_GREY); // dot 41
     }    
     if (dot[52] == 1) {  // Check if dot 52 gobbled already
    myGLCD.fillCircle(252, 181, 2, TFT_GREY); // dot 52
     } 
     if (dot[54] == 1) {  // Check if dot 54 gobbled already
    myGLCD.fillCircle(298, 181, 2, TFT_GREY); // dot 54
     } 
     if (dot[60] == 1) {  // Check if dot 60 gobbled already
    myGLCD.fillCircle(298, 201, 7, TFT_GREY); // Big dot 60
     }         
  } else
  if (xG == 284) { // dot 54
     if (dot[53] == 1) {  // Check if dot 53 gobbled already
    myGLCD.fillCircle(275, 181, 2, TFT_GREY); // dot 53
     }    
     if (dot[60] == 1) {  // Check if dot 60 gobbled already
    myGLCD.fillCircle(298, 201, 7, TFT_GREY); // Big dot 60
     }      
  } 

} else
if (yG == 188) {  // if in Row 5  **********************************************************
  if (xG == 4) { // dot 55
     if (dot[42] == 1) {  // Check if dot 42 gobbled already
    myGLCD.fillCircle(19, 181, 2, TFT_GREY); // dot 42
     } 
     if (dot[61] == 1) {  // Check if dot 61 gobbled already
    myGLCD.fillCircle(19, 221, 2, TFT_GREY); // dot 61
     }    
  } else
   if (xG == 62) { // dot 56
     if (dot[44] == 1) {  // Check if dot 44 gobbled already
    myGLCD.fillCircle(65, 181, 2, TFT_GREY); // dot 44
     } 
     if (dot[45] == 1) {  // Check if dot 45 gobbled already
    myGLCD.fillCircle(88, 181, 2, TFT_GREY); // dot 45
     } 
     if (dot[63] == 1) {  // Check if dot 63 gobbled already
    myGLCD.fillCircle(65, 221, 2, TFT_GREY); // dot 63
     }
     if (dot[64] == 1) {  // Check if dot 64 gobbled already
    myGLCD.fillCircle(88, 221, 2, TFT_GREY); // dot 64
     }      
     
  } else
  
  if (xG == 120) { // dot 57
     if (dot[47] == 1) {  // Check if dot 47 gobbled already
    myGLCD.fillCircle(136, 181, 2, TFT_GREY); // dot 47
     }     
     if (dot[66] == 1) {  // Check if dot 66 gobbled already
    myGLCD.fillCircle(136, 221, 2, TFT_GREY); // dot 66
     }    
    // Draw fruit
    if ((fruitdrawn == true)&&(fruitgone == false)){ // draw fruit again
        drawicon(146, 168, fruit); //   draw fruit 
    }
  } else
  if (xG == 168) { // dot 58
     if (dot[67] == 1) {  // Check if dot 67 gobbled already
    myGLCD.fillCircle(183, 221, 2, TFT_GREY); // dot 67
     }     
     if (dot[49] == 1) {  // Check if dot 49 gobbled already
    myGLCD.fillCircle(183, 181, 2, TFT_GREY); // dot 49
     }       
    // Draw fruit
    if ((fruitdrawn == true)&&(fruitgone == false)){ // draw fruit again
        drawicon(146, 168, fruit); //   draw fruit 
    }
  } else
  
  if (xG == 228) { // dot 59
     if (dot[51] == 1) {  // Check if dot 51 gobbled already
    myGLCD.fillCircle(229, 181, 2, TFT_GREY); // dot 51
     }
     if (dot[52] == 1) {  // Check if dot 52 gobbled already
    myGLCD.fillCircle(252, 181, 2, TFT_GREY); // dot 52
     } 
     if (dot[69] == 1) {  // Check if dot 69 gobbled already
    myGLCD.fillCircle(229, 221, 2, TFT_GREY); // dot 69
     } 
     if (dot[70] == 1) {  // Check if dot 70 gobbled already
    myGLCD.fillCircle(252, 221, 2, TFT_GREY); // dot 70
     }      
     
  } else
  
  if (xG == 284) { // dot 60
     if (dot[72] == 1) {  // Check if dot 72 gobbled already
    myGLCD.fillCircle(298, 221, 2, TFT_GREY); //  dot 72
     } 
     if (dot[54] == 1) {  // Check if dot 54 gobbled already
    myGLCD.fillCircle(298, 181, 2, TFT_GREY); // dot 54
     }    
  } 

} else


if (yG == 208) {  // if in Row 6  **********************************************************
  if (xG == 4) { // dot 61
     if (dot[55] == 1) {  // Check if dot 55 gobbled already
    myGLCD.fillCircle(19, 201, 7, TFT_GREY); // dot 55
     } 
     if (dot[62] == 1) {  // Check if dot 62 gobbled already
    myGLCD.fillCircle(42, 221, 2, TFT_GREY); // dot 62
     }   
  } else
  if (xG == 28) { // dot 62
     if (dot[61] == 1) {  // Check if dot 61 gobbled already
    myGLCD.fillCircle(19, 221, 2, TFT_GREY); // dot 61
     }  
     if (dot[63] == 1) {  // Check if dot 63 gobbled already
    myGLCD.fillCircle(65, 221, 2, TFT_GREY); // dot 63
     }      
  } else
  if (xG == 52) { // dot 63
     if (dot[64] == 1) {  // Check if dot 64 gobbled already
    myGLCD.fillCircle(88, 221, 2, TFT_GREY); // dot 64
     } 
     if (dot[62] == 1) {  // Check if dot 62 gobbled already
    myGLCD.fillCircle(42, 221, 2, TFT_GREY); // dot 62
     }  
     if (dot[56] == 1) {  // Check if dot 56 gobbled already
    myGLCD.fillCircle(77, 201, 2, TFT_GREY); // dot 56 
     }      
  } else
  if (xG == 74) { // dot 64
     if (dot[65] == 1) {  // Check if dot 65 gobbled already
    myGLCD.fillCircle(112, 221, 2, TFT_GREY); // dot 65
     } 
     if (dot[63] == 1) {  // Check if dot 63 gobbled already
    myGLCD.fillCircle(65, 221, 2, TFT_GREY); // dot 63
     }  
     if (dot[56] == 1) {  // Check if dot 56 gobbled already
    myGLCD.fillCircle(77, 201, 2, TFT_GREY); // dot 56 
     }     
  } else
  if (xG == 98) { // dot 65
     if (dot[64] == 1) {  // Check if dot 64 gobbled already
    myGLCD.fillCircle(88, 221, 2, TFT_GREY); // dot 64
     } 
     if (dot[66] == 1) {  // Check if dot 66 gobbled already
    myGLCD.fillCircle(136, 221, 2, TFT_GREY); // dot 66
     }    
  } else
  if (xG == 120) { // dot 66
     if (dot[65] == 1) {  // Check if dot 65 gobbled already
    myGLCD.fillCircle(112, 221, 2, TFT_GREY); // dot 65
     } 
     if (dot[57] == 1) {  // Check if dot 57 gobbled already
    myGLCD.fillCircle(136, 201, 2, TFT_GREY); // dot 57 
     }    
  } else
  if (xG == 168) { // dot 67
     if (dot[68] == 1) {  // Check if dot 68 gobbled already
    myGLCD.fillCircle(206, 221, 2, TFT_GREY); // dot 68
     } 
     if (dot[58] == 1) {  // Check if dot 58 gobbled already
    myGLCD.fillCircle(183, 201, 2, TFT_GREY); // dot 58
     }     
  } else
  if (xG == 192) { // dot 68
     if (dot[67] == 1) {  // Check if dot 67 gobbled already
    myGLCD.fillCircle(183, 221, 2, TFT_GREY); // dot 67
     } 
     if (dot[69] == 1) {  // Check if dot 69 gobbled already
    myGLCD.fillCircle(229, 221, 2, TFT_GREY); // dot 69
     }    
  } else
  if (xG == 216) { // dot 69
     if (dot[68] == 1) {  // Check if dot 68 gobbled already
    myGLCD.fillCircle(206, 221, 2, TFT_GREY); // dot 68
     } 
     if (dot[59] == 1) {  // Check if dot 59 gobbled already
    myGLCD.fillCircle(241, 201, 2, TFT_GREY); // dot 59
     }
     if (dot[70] == 1) {  // Check if dot 70 gobbled already
    myGLCD.fillCircle(252, 221, 2, TFT_GREY); // dot 70
     }    
  } else
  if (xG == 238) { // dot 70
     if (dot[69] == 1) {  // Check if dot 69 gobbled already
    myGLCD.fillCircle(229, 221, 2, TFT_GREY); // dot 69
     } 
     if (dot[59] == 1) {  // Check if dot 59 gobbled already
    myGLCD.fillCircle(241, 201, 2, TFT_GREY); // dot 59
     }
     if (dot[71] == 1) {  // Check if dot 71 gobbled already
    myGLCD.fillCircle(275, 221, 2, TFT_GREY); // dot 71
     }       
  } else
  if (xG == 262) { // dot 71
     if (dot[70] == 1) {  // Check if dot 70 gobbled already
    myGLCD.fillCircle(252, 221, 2, TFT_GREY); // dot 70
     }  
     if (dot[72] == 1) {  // Check if dot 72 gobbled already
    myGLCD.fillCircle(298, 221, 2, TFT_GREY); // dot 72
     }       
     if (dot[60] == 1) {  // Check if dot 60 gobbled already
    myGLCD.fillCircle(298, 201, 7, TFT_GREY); // Big dot 60
     }
  } else
  if (xG == 284) { // dot 72
     if (dot[71] == 1) {  // Check if dot 71 gobbled already
    myGLCD.fillCircle(275, 221, 2, TFT_GREY); // dot 71
     } 
     if (dot[60] == 1) {  // Check if dot 60 gobbled already
    myGLCD.fillCircle(298, 201, 7, TFT_GREY); // Big dot 60
     }     
  }
} else



// Check Columns


if (xG == 28) {  // if in Column 2
  if (yG == 66) { // dot 32
     if (dot[20] == 1) {  // Check if dot 20 gobbled already
    myGLCD.fillCircle(42, 60, 2, TFT_GREY); // dot 20
     }     
     if (dot[34] == 1) {  // Check if dot 34 gobbled already
    myGLCD.fillCircle(42, 100, 2, TFT_GREY); // dot 34
     }        
  } else
  if (yG == 86) { // dot 34
      if (dot[32] == 1) {  // Check if dot 32 gobbled already
    myGLCD.fillCircle(42, 80, 2, TFT_GREY); // dot 32
     }  
      if (dot[36] == 1) {  // Check if dot 36 gobbled already
    myGLCD.fillCircle(42, 120, 2, TFT_GREY); // dot 36
     }      
  } else
  if (yG == 106) { // dot 36
     if (dot[38] == 1) {  // Check if dot 38 gobbled already
    myGLCD.fillCircle(42, 140, 2, TFT_GREY); // dot 38
     }     
     if (dot[34] == 1) {  // Check if dot 34 gobbled already
    myGLCD.fillCircle(42, 100, 2, TFT_GREY); // dot 34
     }      
  } else
  if (yG == 126) { // dot 38
      if (dot[40] == 1) {  // Check if dot 40 gobbled already
    myGLCD.fillCircle(42, 160, 2, TFT_GREY); // dot 40
     } 
      if (dot[36] == 1) {  // Check if dot 36 gobbled already
    myGLCD.fillCircle(42, 120, 2, TFT_GREY); // dot 36
     }       
  } else
  if (yG == 146) { // dot 40
     if (dot[38] == 1) {  // Check if dot 38 gobbled already
    myGLCD.fillCircle(42, 140, 2, TFT_GREY); // dot 38
     }     
     if (dot[43] == 1) {  // Check if dot 43 gobbled already
    myGLCD.fillCircle(42, 181, 2, TFT_GREY); // dot 43
     }      
  } 

} else
if (xG == 262) {  // if in Column 7

  if (yG == 66) { // dot 33
      if (dot[30] == 1) {  // Check if dot 30 gobbled already
    myGLCD.fillCircle(275, 60, 2, TFT_GREY); // dot 30
     }   
      if (dot[35] == 1) {  // Check if dot 35 gobbled already
    myGLCD.fillCircle(275, 100, 2, TFT_GREY); // dot 35
     }   
  } else
  if (yG == 86) { // dot 35
      if (dot[33] == 1) {  // Check if dot 33 gobbled already
    myGLCD.fillCircle(275, 80, 2, TFT_GREY); // dot 33
     }  
      if (dot[37] == 1) {  // Check if dot 37 gobbled already
    myGLCD.fillCircle(275, 120, 2, TFT_GREY); // dot 37
     }     
  } else
  if (yG == 106) { // dot 37
      if (dot[35] == 1) {  // Check if dot 35 gobbled already
    myGLCD.fillCircle(275, 100, 2, TFT_GREY); // dot 35
     }  
      if (dot[39] == 1) {  // Check if dot 39 gobbled already
    myGLCD.fillCircle(275, 140, 2, TFT_GREY); // dot 39
     }      
  } else
  if (yG == 126) { // dot 39
      if (dot[37] == 1) {  // Check if dot 37 gobbled already
    myGLCD.fillCircle(275, 120, 2, TFT_GREY); // dot 37
     }
     if (dot[41] == 1) {  // Check if dot 41 gobbled already
    myGLCD.fillCircle(275, 160, 2, TFT_GREY); // dot 41
     }       
  } else
  if (yG == 146) { // dot 41
      if (dot[39] == 1) {  // Check if dot 39 gobbled already
    myGLCD.fillCircle(275, 140, 2, TFT_GREY); // dot 39
     } 
     if (dot[53] == 1) {  // Check if dot 53 gobbled already
    myGLCD.fillCircle(275, 181, 2, TFT_GREY); // dot 53
     }     
  } 
}




// Capture legacy direction to enable adequate blanking of trail
prevGD = GD;

if(GD == 0){
// Increment xG and then test if any decisions required on turning up or down
  xG = xG+cstep; 

/* Temp print variables for testing
  
  myGLCD.setColor(0, 0, 0);
  myGLCD.setBackColor(114, 198, 206);
  myGLCD.drawString(xG,80,165); // Print xG
  myGLCD.drawString(yP,110,165); // Print yP
*/



 // There are four horizontal rows that need rules

  // First Horizontal Row
  if (yG== 4) { 

    // Past first block decide to continue or go down
    if (xG == 62) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = gdirect; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
    // Past second block only option is down
    if (xG == 120) { 
         GD = 1; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
    // Past third block decide to continue or go down
    if (xG == 228) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = gdirect; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
    // Past fourth block only option is down
    if (xG == 284) { 
         GD = 1; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
  }

  // 2nd Horizontal Row
  if (yG == 46) { 

    // Past upper doorway on left decide to continue right or go down
    if (xG == 28) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = gdirect; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

    // Past first block decide to continue right or go up
    if (xG == 62) { 
      if (random(2) == 0){
         GD = 0; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else { GD = 3;}
    }
     // Past Second block decide to continue right or go up
    if (xG == 120) { 
      if (random(2) == 0){
         GD = 0; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else { GD = 3;}
    }

     // Past Mid Wall decide to continue right or go up
    if (xG == 168) { 
      if (random(2) == 0){
         GD = 0; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else { GD = 3;}
    }

    // Past third block decide to continue right or go up
    if (xG == 228) { 
      if (random(2) == 0){
         GD = 0; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else { GD = 3;}
    }

    // Past last clock digit decide to continue or go down
    if (xG == 262) { 
      gdirect = random(2); // generate random number between 0 and 2
      if (gdirect == 1){
         GD = gdirect; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

    // Past fourth block only option is up
    if (xG == 284) { 
         GD = 3; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
  }

  // 3rd Horizontal Row
  if (yG== 168) { 

    // Past lower doorway on left decide to continue right or go up
    if (xG == 28) { 
      if (random(2) == 0){
         GD = 0; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else { GD = 3;}
    }

    // Past first block decide to continue or go down
    if (xG == 62) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = gdirect; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
     // Past Second block decide to continue or go down
    if (xG == 120) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = gdirect; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

     // Past Mid Wall decide to continue or go down
    if (xG == 168) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = gdirect; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

    // Past third block decide to continue or go down
    if (xG == 228) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = gdirect; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

    // Past last clock digit decide to continue right or go up
    if (xG == 262) { 
      if (random(2) == 0){
         GD = 0; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else { GD = 3;}
    }

    // Past fourth block only option is down
    if (xG == 284) { 
         GD = 1; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
  }
 
  
  // 4th Horizontal Row
  if (yG== 208) { 

    // Past first block decide to continue right or go up
    if (xG == 62) { 
      if (random(2) == 0){
         GD = 0; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else { GD = 3;}
    }
    // Past second block only option is up
    if (xG == 120) { 
         GD = 3; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
    // Past third block decide to continue right or go up
    if (xG == 228) { 
      if (random(2) == 0){
         GD = 0; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else { GD = 3;}
    }
    // Past fourth block only option is up
    if (xG == 284) { 
         GD = 3; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
   }
}

//****************************************************************************************************************************
//Left hand motion **********************************************************************************************************
//****************************************************************************************************************************

else if(GD == 2){
// Increment xG and then test if any decisions required on turning up or down
  xG = xG-cstep; 

/* Temp print variables for testing
  
  myGLCD.setColor(0, 0, 0);
  myGLCD.setBackColor(114, 198, 206);
  myGLCD.drawString(xG,80,165); // Print xG
  myGLCD.drawString(yP,110,165); // Print yP
*/

 // There are four horizontal rows that need rules

  // First Horizontal Row  ******************************
  if (yG== 4) { 

     // Past first block only option is down
    if (xG == 4) { 
         GD = 1; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
 
    // Past second block decide to continue or go down
    if (xG == 62) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = gdirect; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
    // Past third block only option is down
    if (xG == 168) { 
         GD = 1; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
    // Past fourth block decide to continue or go down
    if (xG == 228) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = gdirect; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
  }

  // 2nd Horizontal Row ******************************
  if (yG== 46) { 

    // Meet LHS wall only option is up
    if (xG == 4) { 
         GD = 3; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
    // Meet upper doorway on left decide to continue left or go down
    if (xG == 28) { 
      if (random(2) == 0){
         GD = 1; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else { GD = 2;}
    }

    // Meet first block decide to continue left or go up
    if (xG == 62) { 
      if (random(2) == 0){
         GD = 3; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else { GD = 2;}
    }
     // Meet Second block decide to continue left or go up
    if (xG == 120) { 
      if (random(2) == 0){
         GD = 3; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else { GD = 2;}
    }

     // Meet Mid Wall decide to continue left or go up
    if (xG == 168) { 
      if (random(2) == 0){
         GD = 3; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else { GD = 2;}
    }

    // Meet third block decide to continue left or go up
    if (xG == 228) { 
      if (random(2) == 0){
         GD = 3; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else { GD = 2;}
    }

    // Meet last clock digit decide to continue or go down
    if (xG == 262) { 
      gdirect = random(2); // generate random number between 0 and 3
      if (gdirect == 1){
         GD = gdirect; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

  }
 
   // RHS Door Horizontal Row
  if (yG == 108) { 

    // Past upper doorway on left decide to go up or go down
    if (xG == 262) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = 1; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { GD = 3;}    
    }
  } 

  // 3rd Horizontal Row ******************************
  if (yG== 168) { 

    // Meet LHS lower wall only option is down
    if (xG == 4) { 
         GD = 1; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }

    // Meet lower doorway on left decide to continue left or go up
    if (xG == 28) { 
      if (random(2) == 0){
         GD = 3; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else { GD = 2;}
    }

    // Meet first block decide to continue or go down
    if (xG == 62) { 
      gdirect = random(2); // generate random number between 0 and 3
      if (gdirect == 1){
         GD = gdirect; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
     // Meet Second block decide to continue or go down
    if (xG == 120) { 
      gdirect = random(2); // generate random number between 0 and 3
      if (gdirect == 1){
         GD = gdirect; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

     // Meet Mid Wall decide to continue or go down
    if (xG == 168) { 
      gdirect = random(2); // generate random number between 0 and 3
      if (gdirect == 1){
         GD = gdirect; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

    // Meet third block decide to continue or go down
    if (xG == 228) { 
      gdirect = random(2); // generate random number between 0 and 3
      if (gdirect == 1){
         GD = gdirect; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

    // Meet last clock digit above decide to continue left or go up
    if (xG == 262) { 
      if (random(2) == 0){
         GD = 3; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else { GD = 2;}
    
    }
    
  }
   // 4th Horizontal Row ******************************
  if (yG== 208) { 

    // Meet LHS wall only option is up
    if (xG == 4) { 
         GD = 3; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }  
    // Meet first block decide to continue left or go up
    if (xG == 62) { 
      if (random(2) == 0){
         GD = 3; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else { GD = 2;}
    }
    // Meet bottom divider wall only option is up
    if (xG == 168) { 
         GD = 3; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
    // Meet 3rd block decide to continue left or go up
    if (xG == 228) { 
      if (random(2) == 0){
         GD = 3; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else { GD = 2;}
    }
   
  
  }
}  
  


//****************************************************************************************************************************
//Down motion **********************************************************************************************************
//****************************************************************************************************************************

else if(GD == 1){
// Increment yGand then test if any decisions required on turning up or down
  yG= yG+cstep; 

/* Temp print variables for testing
  
  myGLCD.setColor(0, 0, 0);
  myGLCD.setBackColor(114, 198, 206);
  myGLCD.drawString(xG,80,165); // Print xG
  myGLCD.drawString(yP,110,165); // Print yP
*/

 // There are vertical rows that need rules

  // First Vertical Row  ******************************
  if (xG == 4) { 

     // Past first block only option is right
    if (yG== 46) { 
         GD = 0; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
 
    // Towards bottom wall only option right
    if (yG== 208) { 
         GD = 0; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    } 
  }

  // 2nd Vertical Row ******************************
  if (xG == 28) { 

    // Meet bottom doorway on left decide to go left or go right
    if (yG== 168) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { GD = 0;}    
    }
  }

  // 3rd Vertical Row ******************************
  if (xG == 62) { 

    // Meet top lh digit decide to go left or go right
    if (yG== 46) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { GD = 0;}    
    }

    // Meet top lh digit decide to go left or go right
    if (yG== 208) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { GD = 0;}    
    }
  }

  // 5th Vertical Row ******************************
  if (xG == 120) { 

    // Meet top lh digit decide to go left or go right
    if (yG== 46) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { GD = 0;}    
    }

    // Meet bottom wall only opgion to go left
    if (yG== 208) { 
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up   
    }
  }

  // 6th Vertical Row ******************************
  if (xG == 168) { 

    // Meet top lh digit decide to go left or go right
    if (yG== 46) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { GD = 0;}    
    }

    // Meet bottom wall only opgion to go right
    if (yG== 208) { 
         GD = 0; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up   
    }
  }

  // 8th Vertical Row ******************************
  if (xG == 228) { 

    // Meet top lh digit decide to go left or go right
    if (yG== 46) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { GD = 0;}    
    }

    // Meet bottom wall
    if (yG== 208) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { GD = 0;}    
    }
  }

  // 9th Vertical Row ******************************
  if (xG == 262) { 

    // Meet bottom right doorway  decide to go left or go right
    if (yG== 168) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { GD = 0;}    
    }
  }

  // 10th Vertical Row  ******************************
  if (xG == 284) { 

     // Past first block only option is left
    if (yG== 46) { 
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
 
    // Towards bottom wall only option right
    if (yG== 208) { 
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    } 
  } 
}  

//****************************************************************************************************************************
//Up motion **********************************************************************************************************
//****************************************************************************************************************************

else if(GD == 3){
// Decrement yGand then test if any decisions required on turning up or down
  yG= yG-cstep; 

/* Temp print variables for testing
  
  myGLCD.setColor(0, 0, 0);
  myGLCD.setBackColor(114, 198, 206);
  myGLCD.drawString(xG,80,165); // Print xG
  myGLCD.drawString(yP,110,165); // Print yP
*/


 // There are vertical rows that need rules

  // First Vertical Row  ******************************
  if (xG == 4) { 

     // Past first block only option is right
    if (yG== 4) { 
         GD = 0; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
 
    // Towards bottom wall only option right
    if (yG== 168) { 
         GD = 0; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    } 
  }

  // 2nd Vertical Row ******************************
  if (xG == 28) { 

    // Meet top doorway on left decide to go left or go right
    if (yG== 46) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { GD = 0;}    
    }
  }

  // 3rd Vertical Row ******************************
  if (xG == 62) { 

    // Meet top lh digit decide to go left or go right
    if (yG== 4) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { GD = 0;}    
    }

    // Meet top lh digit decide to go left or go right
    if (yG== 168) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { GD = 0;}    
    }
  }

  // 5th Vertical Row ******************************
  if (xG == 120) { 

    // Meet bottom lh digit decide to go left or go right
    if (yG== 168) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { GD = 0;}    
    }

    // Meet top wall only opgion to go left
    if (yG== 4) { 
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up   
    }
  }

  // 6th Vertical Row ******************************
  if (xG == 168) { 

    // Meet bottom lh digit decide to go left or go right
    if (yG== 168) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { GD = 0;}    
    }

    // Meet top wall only opgion to go right
    if (yG== 4) { 
         GD = 0; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up   
    }
  }

  // 8th Vertical Row ******************************
  if (xG == 228) { 

    // Meet bottom lh digit decide to go left or go right
    if (yG== 168) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { GD = 0;}    
    }

    // Meet top wall go left or right
    if (yG== 4) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { GD = 0;}    
    }
  }

  // 9th Vertical Row ******************************
  if (xG == 262) { 

    // Meet top right doorway  decide to go left or go right
    if (yG== 46) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { GD = 0;}    
    }
  }

  // 10th Vertical Row  ******************************
  if (xG == 284) { 

     // Past first block only option is left
    if (yG== 168) { 
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
 
    // Towards top wall only option right
    if (yG== 4) { 
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    } 
  } 
}  

}
  




//******************************************************************************************************************

//******************************************************************************************************************



/*
//temp barriers

if (yP>200) {
  yP=46;
}
if(xP>260){
  xP=4;
}
*/
}




void UpdateDisp(){
 
  printLocalTime(); // Recalculate current time

   
  int h; // Hour value in 24 hour format
  int e; // Minute value in minute format
  int pm = 0; // Flag to detrmine if PM or AM
  
  // There are four digits that need to be drawn independently to ensure consisitent positioning of time
  int d1;  // Tens hour digit
  int d2;  // Ones hour digit
  int d3;  // Tens minute digit
  int d4;  // Ones minute digit
  

  h = clockhour; // 24 hour RT clock value
  e = clockminute;

/* TEST
h = 12;
e = 8;
*/


// Calculate hour digit values for time

if ((h >= 10) && (h <= 12)) {     // AM hours 10,11,12
  d1 = 1; // calculate Tens hour digit
  d2 = h - 10;  // calculate Ones hour digit 0,1,2
  } else  
  if ( (h >= 22) && (h <= 24)) {    // PM hours 10,11,12
  d1 = 1; // calculate Tens hour digit
  d2 = h - 22;  // calculate Ones hour digit 0,1,2    
  } else 
  if ((h <= 9)&&(h >= 1)) {     // AM hours below ten
  d1 = 0; // calculate Tens hour digit
  d2 = h;  // calculate Ones hour digit 0,1,2    
  } else
  if ( (h >= 13) && (h <= 21)) { // PM hours below 10
  d1 = 0; // calculate Tens hour digit
  d2 = h - 12;  // calculate Ones hour digit 0,1,2 
  } else { 
    // If hour is 0
  d1 = 1; // calculate Tens hour digit
  d2 = 2;  // calculate Ones hour digit 0,1,2   
  }
    
    
// Calculate minute digit values for time

if ((e >= 10)) {  
  d3 = e/10 ; // calculate Tens minute digit 1,2,3,4,5
  d4 = e - (d3*10);  // calculate Ones minute digit 0,1,2
  } else {
    // e is less than 10
  d3 = 0;
  d4 = e;
  }  

/* Print test results

myGLCD.drawString(d1,10,200); // Print 0
myGLCD.drawString(d2,40,200); // Print 0
myGLCD.drawString(d3,70,200); // Print 0
myGLCD.drawString(d4,100,200); // Print 0
*/


if (h>=12){ // Set 
//  h = h-12; // Work out value
  pm = 1;  // Set PM flag
} 

// *************************************************************************
// Print each digit if it has changed to reduce screen impact/flicker

// Set digit font colour to white

//  myGLCD.setColor(255, 255, 255);
//  myGLCD.setBackColor(0, 0, 0);
//  myGLCD.setFont(SevenSeg_XXXL_Num);
 myGLCD.setTextColor(TFT_WHITE,TFT_BLACK); 
 myGLCD.setTextSize(2);
 myGLCD.setFreeFont(FF20);
/*

 //  myGLCD.fillScreen(TFT_BLACK);
  myGLCD.setTextColor(TFT_GREY,TFT_BLACK);   
  myGLCD.setFreeFont(FF20);
  myGLCD.setTextSize(2);
  myGLCD.drawNumber(12,80,80); // Print 0
  myGLCD.drawString("AM", 170, 80);
  
 */

  
// First Digit
if(((d1 != c1)||(xsetup == true))&&(d1 != 0)){ // Do not print zero in first digit position
    myGLCD.drawNumber(d1,51,85); // Printing thisnumber impacts LFH walls so redraw impacted area   
// ---------------- reprint two left wall pillars
//    myGLCD.setColor(1, 73, 240);
    
    myGLCD.drawRoundRect(0 , 80  , 27  , 25  , 2 , TFT_BLUE); 
    myGLCD.drawRoundRect(2 , 85  , 23  , 15  , 2 , TFT_BLUE); 

    myGLCD.drawRoundRect(0 , 140 , 27  , 25  , 2 , TFT_BLUE); 
    myGLCD.drawRoundRect(2 , 145 , 23  , 15  , 2 , TFT_BLUE); 

// ---------------- Clear lines on Outside wall
//    myGLCD.setColor(0,0,0);
    myGLCD.drawRoundRect(1 , 1 , 317 , 237 , 2 , TFT_BLACK); 



}
//If prevous time 12:59 or 00:59 and change in time then blank First Digit

if((c1 == 1) && (c2 == 2) && (c3 == 5) && (c4 == 9) && (d2 != c2) ){ // Clear the previouis First Digit and redraw wall

//    myGLCD.setColor(0,0,0);
    myGLCD.fillRect(50  , 70  , 45  , 95, TFT_BLACK);


}

if((c1 == 0) && (c2 == 0) && (c3 == 5) && (c4 == 9) && (d2 != c2) ){ // Clear the previouis First Digit and redraw wall

//    myGLCD.setColor(0,0,0);
    myGLCD.fillRect(50  , 70  , 45  , 95, TFT_BLACK);


}

// Reprint the dots that have not been gobbled
//    myGLCD.setColor(200,200,200);
// Row 4
if ( dot[32] == 1) {
  myGLCD.fillCircle(42, 80, 2,TFT_GREY);
} 

// Row 5

if ( dot[34] == 1) {
  myGLCD.fillCircle(42, 100, 2,TFT_GREY);
}

// Row 6
if ( dot[36] == 1) {
  myGLCD.fillCircle(42, 120, 2,TFT_GREY);
}

// Row 7
if ( dot[38] == 1) {
  myGLCD.fillCircle(42, 140, 2,TFT_GREY);
}

// Row 8
if ( dot[40] == 1) {
  myGLCD.fillCircle(42, 160, 2,TFT_GREY);
}


 myGLCD.setTextColor(TFT_WHITE,TFT_BLACK);  // myGLCD.setTextSize(20);
  
// Second Digit
if((d2 != c2)||(xsetup == true)){
  myGLCD.drawNumber(d2,91,85); // Print 0
}

// Third Digit
if((d3 != c3)||(xsetup == true)){
  myGLCD.drawNumber(d3,156,85); // Was 145    
}

// Fourth Digit
if((d4 != c4)||(xsetup == true)){
  myGLCD.drawNumber(d4,211,85); // Was 205  
}

if (xsetup == true){
  xsetup = false; // Reset Flag now leaving setup mode
  } 
 // Print PM or AM
 
 myGLCD.setTextColor(TFT_WHITE,TFT_BLACK); 
 myGLCD.setTextSize(1);
 myGLCD.setFreeFont(NULL);

  if (pm == 0) {
      myGLCD.drawString("AM", 300, 148); 
   } else {
      myGLCD.drawString("PM", 300, 148);  
   }

// ----------- Alarm Set on LHS lower pillar
if (alarmstatus == true) { // Print AS on fron screenleft hand side
      myGLCD.drawString("AS", 7, 147); 
}


  // Round dots

//  myGLCD.setColor(255, 255, 255);
//  myGLCD.setBackColor(0, 0, 0);
  myGLCD.fillCircle(148, 112, 4,TFT_WHITE);
  myGLCD.fillCircle(148, 132, 4,TFT_WHITE);





//--------------------- copy exising time digits to global variables so that these can be used to test which digits change in future

c1 = d1;
c2 = d2;
c3 = d3;
c4 = d4;

}




// ===== initiateGame - Custom Function
void drawscreen() {

 // test only 

//  myGLCD.fillRect(100, 100, 40, 80, TFT_RED);   

  //Draw Background lines

//      myGLCD.setColor(1, 73, 240);
 
// ---------------- Outside wall

//    e.g    myGLCD.drawRoundRect(0, 0, 319, 239,10,   TFT_BLUE ); 

//        myGLCD.drawRoundRect(0, 239, 319, 0, 2, TFT_BLUE); 
//        myGLCD.drawRoundRect(2, 237, 317, 2, 2, TFT_BLUE); 

//        myGLCD.drawRoundRect(0, 0, 319, 239, 2, TFT_BLUE); // X,Y location then X,Y Size 
//        myGLCD.drawRoundRect(2, 2, 315, 235, 2, TFT_BLUE); 

        myGLCD.drawRoundRect(0, 0, 319, 239, 2, TFT_BLUE); // X,Y location then X,Y Size 
        myGLCD.drawRoundRect(2, 2, 315, 235, 2, TFT_BLUE); 

//        myGLCD.drawRoundRect(2 , 2 , 316 , 236 , 2, TFT_GREEN);         



// ---------------- Four top spacers and wall pillar
 
//        myGLCD.drawRoundRect(35, 35, 60, 45, 2, TFT_BLUE); 
//        myGLCD.drawRoundRect(37, 37, 58, 43, 2, TFT_BLUE);

        myGLCD.drawRoundRect(35 , 35  , 25  , 10 , 2 ,  TFT_BLUE); 
        myGLCD.drawRoundRect(37 , 37  , 21  ,  6 , 2, TFT_BLUE);



//        myGLCD.drawRoundRect(93, 35, 118, 45, 2, TFT_BLUE); 
//        myGLCD.drawRoundRect(95, 37, 116, 43, 2, TFT_BLUE);

        myGLCD.drawRoundRect(93 , 35  , 25  , 10  , 2 , TFT_BLUE); 
        myGLCD.drawRoundRect(95 , 37  , 21  , 6 , 2 , TFT_BLUE);
        
//        myGLCD.drawRoundRect(201, 35, 226, 45, 2, TFT_BLUE); 
//        myGLCD.drawRoundRect(203, 37, 224, 43, 2, TFT_BLUE);

        myGLCD.drawRoundRect(201 , 35  , 25  , 10  , 2 , TFT_BLUE); 
        myGLCD.drawRoundRect(203 , 37  , 21  , 6 , 2 , TFT_BLUE);

//        myGLCD.drawRoundRect(258, 35, 283, 45, 2, TFT_BLUE); 
//        myGLCD.drawRoundRect(260, 37, 281, 43, 2, TFT_BLUE);         

        myGLCD.drawRoundRect(258 , 35  , 25  , 10  , 2 , TFT_BLUE); 
        myGLCD.drawRoundRect(260 , 37  , 21  , 6 , 2 , TFT_BLUE); 
      

//        myGLCD.drawRoundRect(155, 0, 165, 45, 2, TFT_BLUE); 
//        myGLCD.drawRoundRect(157, 2, 163, 43, 2, TFT_BLUE); 

        myGLCD.drawRoundRect(155 , 0 , 10  , 45  , 2 , TFT_BLUE); 
        myGLCD.drawRoundRect(157 , 2 , 6 , 41  , 2 , TFT_BLUE); 
 

// ---------------- Four bottom spacers and wall pillar

//        myGLCD.drawRoundRect(35, 196, 60, 206, 2, TFT_BLUE); 
//        myGLCD.drawRoundRect(37, 198, 58, 204, 2, TFT_BLUE);

        myGLCD.drawRoundRect(35 , 196 , 25  , 10  , 2 , TFT_BLUE); 
        myGLCD.drawRoundRect(37 , 198 , 21  , 6 , 2 , TFT_BLUE);

 //       myGLCD.drawRoundRect(93, 196, 118, 206, 2, TFT_BLUE); 
 //       myGLCD.drawRoundRect(95, 198, 116, 204, 2, TFT_BLUE);

        myGLCD.drawRoundRect(93 , 196 , 25  , 10  , 2 , TFT_BLUE); 
        myGLCD.drawRoundRect(95 , 198 , 21  , 6 , 2 , TFT_BLUE);

//        myGLCD.drawRoundRect(201, 196, 226, 206, 2, TFT_BLUE); 
//        myGLCD.drawRoundRect(203, 198, 224, 204, 2, TFT_BLUE);

        myGLCD.drawRoundRect(201 , 196 , 25  , 10  , 2 , TFT_BLUE); 
        myGLCD.drawRoundRect(203 , 198 , 21  , 6 , 2 , TFT_BLUE);
        
//        myGLCD.drawRoundRect(258, 196, 283, 206, 2, TFT_BLUE); 
//        myGLCD.drawRoundRect(260, 198, 281, 204, 6,TFT_BLUE);          

        myGLCD.drawRoundRect(258 , 196 , 25  , 10  , 2 , TFT_BLUE); 
        myGLCD.drawRoundRect(260 , 198 , 21  , 6 , 2 ,TFT_BLUE);          


//        myGLCD.drawRoundRect(155, 196, 165, 239, 2, TFT_BLUE); 
//        myGLCD.drawRoundRect(157, 198, 163, 237, 2, TFT_BLUE); 

        myGLCD.drawRoundRect(155 , 196 , 10  , 43  , 2 , TFT_BLUE); 
        myGLCD.drawRoundRect(157 , 198 , 6 , 39  , 2 , TFT_BLUE); 


// ---------- Four Door Pillars 

//        myGLCD.drawRoundRect(0, 80, 27, 105, 2, TFT_BLUE); 
//        myGLCD.drawRoundRect(2, 85, 25, 100, 2, TFT_BLUE); 

        myGLCD.drawRoundRect(0 , 80  , 27  , 25  , 2 , TFT_BLUE); 
        myGLCD.drawRoundRect(2 , 85  , 23  , 15  , 2 , TFT_BLUE); 

//        myGLCD.drawRoundRect(0, 140, 27, 165, 2, TFT_BLUE); 
//        myGLCD.drawRoundRect(2, 145, 25, 160, 2, TFT_BLUE); 

        myGLCD.drawRoundRect(0 , 140 , 27  , 25  , 2 , TFT_BLUE); 
        myGLCD.drawRoundRect(2 , 145 , 23  , 15  , 2 , TFT_BLUE); 
        
//        myGLCD.drawRoundRect(292, 80, 319, 105, 2, TFT_BLUE); 
//        myGLCD.drawRoundRect(294, 85, 317, 100, 2, TFT_BLUE);
        
        myGLCD.drawRoundRect(292 , 80  , 27  , 25  , 2 , TFT_BLUE); 
        myGLCD.drawRoundRect(294 , 85  , 23  , 15  , 2 , TFT_BLUE); 

//        myGLCD.drawRoundRect(292, 140, 319, 165, 2, TFT_BLUE); 
//        myGLCD.drawRoundRect(294, 145, 317, 160, 2, TFT_BLUE);  
        
        myGLCD.drawRoundRect(292 , 140 , 27  , 25  , 2 , TFT_BLUE); 
        myGLCD.drawRoundRect(294 , 145 , 23  , 15  , 2 , TFT_BLUE);  


 
// ---------------- Clear lines on Outside wall
//        myGLCD.setColor(0,0,0);
//        myGLCD.drawRoundRect(1, 238, 318, 1, 2, TFT_BLACK);   
        myGLCD.drawRoundRect(1 , 1 , 317 , 237 , 2 , TFT_BLACK);  
        
        myGLCD.fillRect(0  , 106 , 3 , 33  ,  TFT_BLACK); 
        myGLCD.fillRect(316  , 106 , 3 , 33 , TFT_BLACK); 

// Draw Dots
//  myGLCD.setColor(200, 200, 200);
//  myGLCD.setBackColor(0, 0, 0);

// delay(10000); 

/*
// Row 4
if ( dot[32] == 1) {
  myGLCD.fillCircle(42, 80, 2,TFT_GREY);
} 
*/



// Row 1
if ( dot[1] == 1) {
  myGLCD.fillCircle(19, 19, 2,TFT_GREY); // dot 1
  }
if ( dot[2] == 1) {  
  myGLCD.fillCircle(42, 19, 2,TFT_GREY); // dot 2
  }
if ( dot[3] == 1) {
  myGLCD.fillCircle(65, 19, 2,TFT_GREY); // dot 3
  }
if ( dot[4] == 1) {
  myGLCD.fillCircle(88, 19, 2,TFT_GREY); // dot 4
  }
if ( dot[5] == 1) {
  myGLCD.fillCircle(112, 19, 2,TFT_GREY); // dot 5
  }
if ( dot[6] == 1) {
  myGLCD.fillCircle(136, 19, 2,TFT_GREY); // dot 6   
  }  
// 
if ( dot[7] == 1) {
  myGLCD.fillCircle(183, 19, 2,TFT_GREY); // dot 7
  }
if ( dot[8] == 1) {  
  myGLCD.fillCircle(206, 19, 2,TFT_GREY);  // dot 8 
  }
if ( dot[9] == 1) {  
  myGLCD.fillCircle(229, 19, 2,TFT_GREY); // dot 9
  }
if ( dot[10] == 1) {  
  myGLCD.fillCircle(252, 19, 2,TFT_GREY); // dot 10
  }
if ( dot[11] == 1) {  
  myGLCD.fillCircle(275, 19, 2,TFT_GREY);  // dot 11
  }
if ( dot[12] == 1) {
  myGLCD.fillCircle(298, 19, 2,TFT_GREY);  // dot 12
  }
// Row 2
if ( dot[13] == 1) {
  myGLCD.fillCircle(19, 40, 7,TFT_GREY); // Big dot 13
  }
if ( dot[14] == 1) {
  myGLCD.fillCircle(77, 40, 2,TFT_GREY);  // dot 14
  }
if ( dot[15] == 1) {
  myGLCD.fillCircle(136, 40, 2,TFT_GREY);  // dot 15
  }
if ( dot[16] == 1) {
  myGLCD.fillCircle(183, 40, 2,TFT_GREY);  // dot 16
  }
if ( dot[17] == 1) {
  myGLCD.fillCircle(241, 40, 2,TFT_GREY);  // dot 17
  }
if ( dot[18] == 1) {
  myGLCD.fillCircle(298, 40, 7,TFT_GREY); // Big dot 18
  }  

  
// Row 3

if ( dot[19] == 1) {
  myGLCD.fillCircle(19, 60, 2,TFT_GREY);
}
if ( dot[20] == 1) {
  myGLCD.fillCircle(42, 60, 2,TFT_GREY);
}
if ( dot[21] == 1) {
  myGLCD.fillCircle(65, 60, 2,TFT_GREY); 
}
if ( dot[22] == 1) {
  myGLCD.fillCircle(88, 60, 2,TFT_GREY);
}
if ( dot[23] == 1) {
  myGLCD.fillCircle(112, 60, 2,TFT_GREY);
}
if ( dot[24] == 1) {
  myGLCD.fillCircle(136, 60, 2,TFT_GREY); 
}
if ( dot[25] == 1) { 
  myGLCD.fillCircle(160, 60, 2,TFT_GREY);
}
if ( dot[26] == 1) {
  myGLCD.fillCircle(183, 60, 2,TFT_GREY);
}
if ( dot[27] == 1) {
  myGLCD.fillCircle(206, 60, 2,TFT_GREY);  
}
if ( dot[28] == 1) {
  myGLCD.fillCircle(229, 60, 2,TFT_GREY);
}
if ( dot[29] == 1) {
  myGLCD.fillCircle(252, 60, 2,TFT_GREY);
}
if ( dot[30] == 1) {
  myGLCD.fillCircle(275, 60, 2,TFT_GREY); 
}
if ( dot[31] == 1) {
  myGLCD.fillCircle(298, 60, 2,TFT_GREY);   
}

// Row 4
if ( dot[32] == 1) {
  myGLCD.fillCircle(42, 80, 2,TFT_GREY);
}
if ( dot[33] == 1) {
  myGLCD.fillCircle(275, 80, 2,TFT_GREY);   
}
// Row 5
if ( dot[34] == 1) {
  myGLCD.fillCircle(42, 100, 2,TFT_GREY);
}
if ( dot[35] == 1) {
  myGLCD.fillCircle(275, 100, 2,TFT_GREY);
}
// Row 6
if ( dot[36] == 1) {
  myGLCD.fillCircle(42, 120, 2,TFT_GREY);
}
if ( dot[37] == 1) {
  myGLCD.fillCircle(275, 120, 2,TFT_GREY);
}
// Row 7
if ( dot[38] == 1) {
  myGLCD.fillCircle(42, 140, 2,TFT_GREY);
}
if ( dot[39] == 1) {
  myGLCD.fillCircle(275, 140, 2,TFT_GREY);
}
// Row 8
if ( dot[40] == 1) {
  myGLCD.fillCircle(42, 160, 2,TFT_GREY);
}
if ( dot[41] == 1) {
  myGLCD.fillCircle(275, 160, 2,TFT_GREY);
}
// Row 9
if ( dot[42] == 1) {
  myGLCD.fillCircle(19, 181, 2,TFT_GREY);
}
if ( dot[43] == 1) {
  myGLCD.fillCircle(42, 181, 2,TFT_GREY);
}
if ( dot[44] == 1) {
  myGLCD.fillCircle(65, 181, 2,TFT_GREY); 
}
if ( dot[45] == 1) {
  myGLCD.fillCircle(88, 181, 2,TFT_GREY);
}
if ( dot[46] == 1) {
  myGLCD.fillCircle(112, 181, 2,TFT_GREY);
}
if ( dot[47] == 1) {
  myGLCD.fillCircle(136, 181, 2,TFT_GREY); 
}
if ( dot[48] == 1) { 
  myGLCD.fillCircle(160, 181, 2,TFT_GREY);
}
if ( dot[49] == 1) {
  myGLCD.fillCircle(183, 181, 2,TFT_GREY);
}
if ( dot[50] == 1) {
  myGLCD.fillCircle(206, 181, 2,TFT_GREY);  
}
if ( dot[51] == 1) {
  myGLCD.fillCircle(229, 181, 2,TFT_GREY);
}
if ( dot[52] == 1) {
  myGLCD.fillCircle(252, 181, 2,TFT_GREY);
}
if ( dot[53] == 1) {
  myGLCD.fillCircle(275, 181, 2,TFT_GREY); 
}
if ( dot[54] == 1) {
  myGLCD.fillCircle(298, 181, 2,TFT_GREY);   
}
// Row 10
if ( dot[55] == 1) {
  myGLCD.fillCircle(19, 201, 7,TFT_GREY); // Big dot
}
if ( dot[56] == 1) {
  myGLCD.fillCircle(77, 201, 2,TFT_GREY);
}
if ( dot[57] == 1) {
  myGLCD.fillCircle(136, 201, 2,TFT_GREY);
}
if ( dot[58] == 1) {
  myGLCD.fillCircle(183, 201, 2,TFT_GREY);
}
if ( dot[59] == 1) {
  myGLCD.fillCircle(241, 201, 2,TFT_GREY);
}
if ( dot[60] == 1) {
  myGLCD.fillCircle(298, 201, 7,TFT_GREY); // Big dot
}  

  

 
  // Row 11
if ( dot[61] == 1) {
  myGLCD.fillCircle(19, 221, 2,TFT_GREY);
}
if ( dot[62] == 1) {
  myGLCD.fillCircle(42, 221, 2,TFT_GREY);
}
if ( dot[63] == 1) {
  myGLCD.fillCircle(65, 221, 2,TFT_GREY); 
}
if ( dot[64] == 1) { 
  myGLCD.fillCircle(88, 221, 2,TFT_GREY);
}
if ( dot[65] == 1) {
  myGLCD.fillCircle(112, 221, 2,TFT_GREY);
}
if ( dot[66] == 1) {
  myGLCD.fillCircle(136, 221, 2,TFT_GREY);   
}  
//  myGLCD.fillCircle(160, 19, 2,TFT_GREY);

if ( dot[67] == 1) {
  myGLCD.fillCircle(183, 221, 2,TFT_GREY);
}
if ( dot[68] == 1) {
  myGLCD.fillCircle(206, 221, 2,TFT_GREY);  
}
if ( dot[69] == 1) {
  myGLCD.fillCircle(229, 221, 2,TFT_GREY);
}
if ( dot[70] == 1) {
  myGLCD.fillCircle(252, 221, 2,TFT_GREY);
}
if ( dot[71] == 1) {
  myGLCD.fillCircle(275, 221, 2,TFT_GREY); 
}
if ( dot[72] == 1) {
  myGLCD.fillCircle(298, 221, 2,TFT_GREY); 
}


// TempTest delay

// delay(100000);

 }
 
//***************************************************************************************************** 
//====== Draws the Pacman - bitmap
//*****************************************************************************************************
void drawPacman(int x, int y, int p, int d, int pd) {



  // Draws the Pacman - bitmap
//  // Pacman direction d == 0 = right, 1 = down, 2 = left, 3 = up
//  myGLCD.setColor(0, 0, 0);
//  myGLCD.setBackColor(0, 0, 0);

if ( d == 0){ // Right

if (pd == 0){ // Legacy direction Right
  myGLCD.fillRect(x-1, y, 2, 28, TFT_BLACK); // Clear trail off graphic before printing new position
  }
if (pd == 3){ // Legacy direction Up
 myGLCD.fillRect(x, y+28, 28, 2, TFT_BLACK); // Clear trail off graphic before printing new position
}
if (pd == 1){ // Legacy direction Down
 myGLCD.fillRect(x, y-1, 28, 2, TFT_BLACK); // Clear trail off graphic before printing new position 
}
if (pd == 2){ // Legacy direction Left
 myGLCD.fillRect(x+28, y, 2, 28, TFT_BLACK); // Clear trail off graphic before printing new positi 
}

if (p == 0) { 


   if (mspacman == false){
    drawicon(x, y, c_pacman); //   Closed Pacman  
   } else {
    drawicon(x, y, ms_c_pacman_r); //   Closed Pacman        
   }


 } else if( p == 1) {

   if (mspacman == false){
   drawicon(x, y, r_m_pacman); //  Medium open Pacman 
   } else {
   drawicon(x, y, ms_r_m_pacman); //  Medium open Pacman       
   }
   
 } else if( p == 2) {

   if (mspacman == false){
   drawicon(x, y, r_o_pacman); //  Open Mouth Pacman  
   } else {
   drawicon(x, y, ms_r_o_pacman); //  Open Mouth Pacman       
   }
 }
} else  if ( d == 1){ // Down

if (pd == 0){ // Legacy direction Right
  myGLCD.fillRect(x-1, y, 2, 28, TFT_BLACK); // Clear trail off graphic before printing new position
  }
if (pd == 3){ // Legacy direction Up
 myGLCD.fillRect(x, y+28, 28, 2, TFT_BLACK); // Clear trail off graphic before printing new position
}
if (pd == 1){ // Legacy direction Down
 myGLCD.fillRect(x, y-1, 28, 2, TFT_BLACK); // Clear trail off graphic before printing new position 
}
if (pd == 2){ // Legacy direction Left
 myGLCD.fillRect(x+28, y, 2, 28, TFT_BLACK); // Clear trail off graphic before printing new positi 
}

 if (p == 0) { 
   
   if (mspacman == false){
    drawicon(x, y, c_pacman); //   Closed Pacman  
   } else {
    drawicon(x, y, ms_c_pacman_d); //   Closed Pacman        
   }
    
    
 } else if( p == 1) {

   if (mspacman == false){
   drawicon(x, y, d_m_pacman); //  Medium open Pacman   
   } else {
   drawicon(x, y, ms_d_m_pacman); //  Medium open Pacman     
   }

 } else if( p == 2) {

   if (mspacman == false){
     drawicon(x, y, d_o_pacman); //  Open Mouth Pacman  
   } else {
     drawicon(x, y, ms_d_o_pacman); //  Open Mouth Pacman     
   }

 }
} else  if ( d == 2){ // Left

if (pd == 0){ // Legacy direction Right
  myGLCD.fillRect(x-1, y, 2, 28, TFT_BLACK); // Clear trail off graphic before printing new position
  }
if (pd == 3){ // Legacy direction Up
 myGLCD.fillRect(x, y+28, 28, 2, TFT_BLACK); // Clear trail off graphic before printing new position
}
if (pd == 1){ // Legacy direction Down
 myGLCD.fillRect(x, y-1, 28, 2, TFT_BLACK); // Clear trail off graphic before printing new position 
}
if (pd == 2){ // Legacy direction Left
 myGLCD.fillRect(x+28, y, 2, 28, TFT_BLACK); // Clear trail off graphic before printing new positi 
}

 if (p == 0) { 

   if (mspacman == false){
    drawicon(x, y, c_pacman); //   Closed Pacman  
   } else {
    drawicon(x, y, ms_c_pacman_l); //   Closed Pacman        
   }


 } else if( p == 1) {

   if (mspacman == false){
     drawicon(x, y, l_m_pacman); //  Medium open Pacman   
   } else {
     drawicon(x, y, ms_l_m_pacman); //  Medium open Pacman   
   }
   
 } else if( p == 2) {
 
   if (mspacman == false){
     drawicon(x, y, l_o_pacman); //  Open Mouth Pacman   
   } else {
     drawicon(x, y, ms_l_o_pacman); //  Open Mouth Pacman  
   }

 }
} else  if ( d == 3){ // Up

if (pd == 0){ // Legacy direction Right
  myGLCD.fillRect(x-1, y, 2, 28, TFT_BLACK); // Clear trail off graphic before printing new position
  }
if (pd == 3){ // Legacy direction Up
 myGLCD.fillRect(x, y+28, 28, 2, TFT_BLACK); // Clear trail off graphic before printing new position
}
if (pd == 1){ // Legacy direction Down
 myGLCD.fillRect(x, y-1, 28, 2, TFT_BLACK); // Clear trail off graphic before printing new position 
}
if (pd == 2){ // Legacy direction Left
 myGLCD.fillRect(x+28, y, 2, 28, TFT_BLACK); // Clear trail off graphic before printing new positi 
}

 if (p == 0) { 

   if (mspacman == false){
    drawicon(x, y, c_pacman); //   Closed Pacman  
   } else {
    drawicon(x, y, ms_c_pacman_u); //   Closed Pacman        
   }


 } else if( p == 1) {

   if (mspacman == false){
     drawicon(x, y, u_m_pacman); //  Medium open Pacman    
   } else {
     drawicon(x, y, ms_u_m_pacman); //  Medium open Pacman   
   }
   

 } else if( p == 2) {

   if (mspacman == false){
     drawicon(x, y, u_o_pacman); //  Open Mouth Pacman    
   } else {
     drawicon(x, y, ms_u_o_pacman); //  Open Mouth Pacman  
   }
   
 }

}
  
}

//**********************************************************************************************************
//====== Draws the Ghost - bitmap
void drawGhost(int x, int y, int d, int pd) {


  // Draws the Ghost - bitmap
//  // Ghost direction d == 0 = right, 1 = down, 2 = left, 3 = up


if ( d == 0){ // Right

if (pd == 0){ // Legacy direction Right
  myGLCD.fillRect(x-1, y, 2, 28, TFT_BLACK); // Clear trail off graphic before printing new position
  }
if (pd == 3){ // Legacy direction Up
 myGLCD.fillRect(x, y+28, 28, 2, TFT_BLACK); // Clear trail off graphic before printing new position
}
if (pd == 1){ // Legacy direction Down
 myGLCD.fillRect(x, y-1, 28, 2, TFT_BLACK); // Clear trail off graphic before printing new position 
}
if (pd == 2){ // Legacy direction Left
 myGLCD.fillRect(x+28, y, 2, 28, TFT_BLACK); // Clear trail off graphic before printing new positi 
}

  if (fruiteatenpacman == true){ 
    drawicon(x, y, bluepacman); //   Closed Ghost 
  } else {
    drawicon(x, y, rr_ghost); //   Closed Ghost 
  }
  
} else  if ( d == 1){ // Down

if (pd == 0){ // Legacy direction Right
  myGLCD.fillRect(x-1, y, 2, 28, TFT_BLACK); // Clear trail off graphic before printing new position
  }
if (pd == 3){ // Legacy direction Up
 myGLCD.fillRect(x, y+28, 28, 2, TFT_BLACK); // Clear trail off graphic before printing new position
}
if (pd == 1){ // Legacy direction Down
 myGLCD.fillRect(x, y-1, 28, 2, TFT_BLACK); // Clear trail off graphic before printing new position 
}
if (pd == 2){ // Legacy direction Left
 myGLCD.fillRect(x+28, y, 2, 28, TFT_BLACK); // Clear trail off graphic before printing new positi 
}

  if (fruiteatenpacman == true){ 
    drawicon(x, y, bluepacman); //   Closed Ghost 
  } else {
    drawicon(x, y, rd_ghost); //   Closed Ghost 
  }

} else  if ( d == 2){ // Left

if (pd == 0){ // Legacy direction Right
  myGLCD.fillRect(x-1, y, 2, 28, TFT_BLACK); // Clear trail off graphic before printing new position
  }
if (pd == 3){ // Legacy direction Up
 myGLCD.fillRect(x, y+28, 28, 2, TFT_BLACK); // Clear trail off graphic before printing new position
}
if (pd == 1){ // Legacy direction Down
 myGLCD.fillRect(x, y-1, 28, 2, TFT_BLACK); // Clear trail off graphic before printing new position 
}
if (pd == 2){ // Legacy direction Left
 myGLCD.fillRect(x+28, y, 2, 28, TFT_BLACK); // Clear trail off graphic before printing new positi 
}

  if (fruiteatenpacman == true){ 
    drawicon(x, y, bluepacman); //   Closed Ghost 
  } else {
    drawicon(x, y, rl_ghost); //   Closed Ghost 
  }

} else  if ( d == 3){ // Up

if (pd == 0){ // Legacy direction Right
  myGLCD.fillRect(x-1, y, 2, 28, TFT_BLACK); // Clear trail off graphic before printing new position
  }
if (pd == 3){ // Legacy direction Up
 myGLCD.fillRect(x, y+28, 28, 2, TFT_BLACK); // Clear trail off graphic before printing new position
}
if (pd == 1){ // Legacy direction Down
 myGLCD.fillRect(x, y-1, 28, 2, TFT_BLACK); // Clear trail off graphic before printing new position 
}
if (pd == 2){ // Legacy direction Left
 myGLCD.fillRect(x+28, y, 2, 28, TFT_BLACK); // Clear trail off graphic before printing new positi 
}

     if (fruiteatenpacman == true){ 
      drawicon(x, y, bluepacman); //   Closed Ghost 
    } else {
      drawicon(x, y, ru_ghost); //   Closed Ghost 
    }

  }
  
}



 
 // ================= Decimal to BCD converter

byte decToBcd(byte val) {
  return ((val/10*16) + (val%10));
} 



void drawicon(int x, int y, unsigned short *icon) { // Draws the graphics icons based on stored image bitmaps  

Gposition = 0;
byte xcount = 0; // Pointer to the pixel in the 28 pixel row
byte ycount = 0; // Pointer to the current row in the graphic being drawn
 
  for (int gpos = 0; gpos < 784; gpos++) {
            
      if (xcount < 28) {       
           myGLCD.drawPixel(x+xcount, y+ycount, icon[gpos]); 
           xcount++; 
      } else
      {
          xcount = 0; // Reset the xposition counter
          ycount++; // Increment the row pointer
          myGLCD.drawPixel(x+xcount, y+ycount, icon[gpos]); 
          xcount++;       
      }
 

  }
  
}

void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S"); 

  clockhour = timeinfo.tm_hour;
  clockminute = timeinfo.tm_min;

}

//*************************************************
void setupclockmenu() { // Nested menu to setup Clock Wifi, Time Parameters, Alarm and Pacman/ Ms Pacman option


   UpdateDisp(); // update value to clock
   
// Firstly Clear screen and setup layout of main clock setup screen - Wifi, Time Zone, Alarm and Character menus
   myGLCD.setTextColor(TFT_WHITE,TFT_BLACK); 
   myGLCD.setTextSize(1);
   myGLCD.setFreeFont(NULL);
   myGLCD.fillScreen(TFT_BLACK);

// ---------------- Outside wall
    myGLCD.drawRoundRect(0  , 0 , 319 , 239 , 2 , TFT_YELLOW); 
    myGLCD.drawRoundRect(2  , 2 , 315 , 235 , 2 , TFT_YELLOW); 
   
//Reset screenpressed flag
    screenPressed = false;

// Setup Main Menu buttons

    myGLCD.setTextColor(TFT_WHITE,TFT_BLACK);   myGLCD.setTextSize(2);  
    
//    myGLCD.drawString("Alarm", 222, 18);       
//    myGLCD.drawRoundRect(190 , 10 , 120  , 30  , 2 , TFT_YELLOW);     

    myGLCD.drawString("Alarm", 238, 25);       
    myGLCD.drawRoundRect(220 , 10 , 90  , 50  , 2 , TFT_YELLOW);
    
    myGLCD.drawString("Exit", 243, 195);       
    myGLCD.drawRoundRect(220 , 180 , 90  , 50  , 2 , TFT_YELLOW);  

// Setup the speed buttons

         if (userspeedsetting == 1){ // flag where false is off and true is on
            myGLCD.drawString("Normal", 18, 195);          
          } else 
          if (userspeedsetting == 2){ // flag where false is off and true is on
            myGLCD.drawString("Fast", 25, 195);
          } else
          if (userspeedsetting == 3){ // flag where false is off and true is on
            myGLCD.drawString("Crazy!!", 15, 195);
          }   
        myGLCD.drawRoundRect(10 , 180 , 90  , 50  , 2 , TFT_YELLOW);     


// Get your Ghost on
   drawicon(80, 50, rr_ghost); //   Closed Ghost 
   drawicon(160, 50, bluepacman); //   Closed Ghost 

    // Display MS Pacman or Pacman in menu - push to change

    myGLCD.drawString("Character =", 80, 106);
           
    // Display MS Pacman or Pacman in menu - push to change
  if (mspacman == false) {
      drawicon(220, 100, r_o_pacman); //   Closed Ghost  
  } else {
      drawicon(220, 100, ms_r_o_pacman); //   Closed Ghost     
  }

delay(500); // Gives user time to remove finger from screen so as to not trigger the next button

// Stay in Setup Loop until exit button pushed
   
   xsetup = true;  // Toggle flag to indicate in main setup menu mode

while (xsetup == true){

// Read the Touch Screen Locations
    TSPoint p = ts.getPoint();

    // if sharing pins, you'll need to fix the directions of the touchscreen pins
   pinMode(XM, OUTPUT);
   pinMode(YP, OUTPUT);

  //        Serial.print("\tPressure = "); Serial.println(p.z); 

 
  // we have some minimum pressure we consider 'valid'
  // pressure of 0 means no pressing!

  if  (( p.z < MAXPRESSURE) && ( p.z > 0)) { 

    // Capture input command from user (tolerance +/- 200 on each parameter

    if (readtouchscreen(Cx-tvar, Cx+tvar, Cy-tvar, Cy+tvar) == true) { 
       if (readtouchscreen(Cx-tvar, Cx+tvar, Cy-tvar, Cy+tvar) == true) { 
          xsetup = false; // Exit setupmode  X = 1483 Y = 2600
          playalarmsound2(); // Play button confirmation sound
        }
    }   else

    if (readtouchscreen(Lx-tvar, Lx+tvar, Ly-tvar, Ly+tvar) == true) { 
       if (readtouchscreen(Lx-tvar, Lx+tvar, Ly-tvar, Ly+tvar) == true) { 
          playalarmsound2(); // Play button confirmation sound
          setupalarmmenu(); // X = 392  Y = 3061
       }   
      } else  

    if (readtouchscreen(Gx-tvar, Gx+tvar, Gy-tvar, Gy+tvar) == true) { 
       if (readtouchscreen(Gx-tvar, Gx+tvar, Gy-tvar, Gy+tvar) == true) {   
          playalarmsound2(); // Play button confirmation sound
         setupacmancharacter();// X = 1400  Y = 2220_
       }  
      } else

    if (readtouchscreen(Fx-tvar, Fx+tvar, Fy-tvar, Fy+tvar) == true) { // Set the alarm X = 1905  Y = 1800_
        if (readtouchscreen(Fx-tvar, Fx+tvar, Fy-tvar, Fy+tvar) == true) {
          userspeedsetting++;
          if(userspeedsetting == 4){
            userspeedsetting = 1;
          }
        
        // Setup the speed buttons
          myGLCD.fillRoundRect(10 , 185 , 85  , 30  , 2 , TFT_BLACK);
         if (userspeedsetting == 1){ // flag where false is off and true is on
            myGLCD.drawString("Normal", 18, 195);          
          } else 
          if (userspeedsetting == 2){ // flag where false is off and true is on
            myGLCD.drawString("Fast", 25, 195);
          } else
          if (userspeedsetting == 3){ // flag where false is off and true is on
            myGLCD.drawString("Crazy!!", 15, 195);
          }   
        myGLCD.drawRoundRect(10 , 180 , 90  , 50  , 2 , TFT_YELLOW);          
        playalarmsound2(); // Play button confirmation sound
        }
      } 

 


           
      // Should mean changes should scroll if held down
        delay(250);      
    } 



 }

    //* Clear Screen
    myGLCD.fillRect(0, 0, 320, 240, TFT_BLACK);
    xsetup = true; // Set Flag now leaving setup mode in order to draw Clock Digits 
    drawscreen(); // Initiate the screen
    UpdateDisp(); // update value to clock


}




void setupalarmmenu() { // Menu used to set the Alarm time

   UpdateDisp(); // update value to clock
   
// Firstly Clear screen and setup layout of main clock setup screen - Wifi, Time Zone, Alarm and Character menus
   myGLCD.setTextColor(TFT_WHITE,TFT_BLACK); 
   myGLCD.setTextSize(2);
   myGLCD.setFreeFont(NULL);
   myGLCD.fillScreen(TFT_BLACK);

// ---------------- Outside wall
    myGLCD.drawRoundRect(0  , 0 , 319 , 239 , 2 , TFT_YELLOW); 
    myGLCD.drawRoundRect(2  , 2 , 315 , 235 , 2 , TFT_YELLOW); 
  
    // Draw Alarm Status
        myGLCD.setTextSize(2);
       if (alarmstatus == true){ // flag where false is off and true is on
          myGLCD.setTextColor(TFT_GREEN,TFT_BLACK); 
          myGLCD.drawString("Alarm", 20, 188);
          myGLCD.drawString("Set", 28, 208);          
        } else {
          myGLCD.setTextColor(TFT_WHITE,TFT_BLACK);  
          myGLCD.drawString("Alarm", 20, 188);
          myGLCD.drawString("Off", 28, 208);
        }   
        myGLCD.drawRoundRect(10 , 180 , 80  , 50 , 2 , TFT_YELLOW);
        myGLCD.setTextColor(TFT_WHITE,TFT_BLACK); 
        
    //Display Current Alarm Setting
    displayalarmsetting();

// ------------


delay(800); // Give user time to remove finger from display

//Reset screenpressed flag
boolean exitmenu = false;

while (exitmenu == false){

// Read the Touch Screen Locations
    TSPoint p = ts.getPoint();
    
    // if sharing pins, you'll need to fix the directions of the touchscreen pins
   pinMode(XM, OUTPUT);
   pinMode(YP, OUTPUT);


 
  if (( p.z < MAXPRESSURE) && ( p.z > 0)) {

          
    // Capture input command from user 
     
    if (readtouchscreen(Ax-tvar, Ax+tvar, Ay-tvar, Ay+tvar) == true) { // Increment alarm hour X = 670  Y = 2494, X = 841  Y = 2362
          if (readtouchscreen(Ax-tvar, Ax+tvar, Ay-tvar, Ay+tvar) == true) {
          alarmhour++; 
           if (alarmhour > 24) {
            alarmhour = 1;
           }
         }        
    } else
  
    if (readtouchscreen(Dx-tvar, Dx+tvar, Dy-tvar, Dy+tvar) == true) { // Decrement alarm hour X = 1718 Y = 1921_
        if (readtouchscreen(Dx-tvar, Dx+tvar, Dy-tvar, Dy+tvar) == true) {
           alarmhour--; 
           if (alarmhour < 0) { //1705, 2105, 1700, 1900
              alarmhour = 24;
           }
        }  
      }  else 

    if (readtouchscreen(Bx-tvar, Bx+tvar, By-tvar, By+tvar) == true) { // Increment alarm minute X = 459 Y = 3040
        if (readtouchscreen(Bx-tvar, Bx+tvar, By-tvar, By+tvar) == true) {
          alarmminute++; 
           if (alarmminute > 59) {
            alarmminute = 0;
           }
        }
      } else

  
    if (readtouchscreen(Ex-tvar, Ex+tvar, Ey-tvar, Ey+tvar) == true) {  // Decrement alarm minute X = 1182  Y = 2810
        if (readtouchscreen(Ex-tvar, Ex+tvar, Ey-tvar, Ey+tvar) == true) { 
           alarmminute--; 
           if (alarmminute < 0) {
            alarmminute = 59;
           }
        }
      } else
 
    if (readtouchscreen(Cx-tvar, Cx+tvar, Cy-tvar, Cy+tvar) == true) { // Save and exit setings X = 1398  Y = 2827_
        if (readtouchscreen(Cx-tvar, Cx+tvar, Cy-tvar, Cy+tvar) == true) {
          playalarmsound2(); // Play button confirmation sound
          exitmenu = true;
        }
      } else

    if (readtouchscreen(Fx-tvar, Fx+tvar, Fy-tvar, Fy+tvar) == true) { // Set the alarm X = 1905  Y = 1800_
          if (readtouchscreen(Fx-tvar, Fx+tvar, Fy-tvar, Fy+tvar) == true) { 
            playalarmsound2(); // Play button confirmation sound
            alarmstatus = !alarmstatus;
          }
    }
      
    //Display Current Alarm Setting
    displayalarmsetting();


      // Should mean changes should scroll if held down
        delay(300);      
    } 

}


/*
// Read Alarm Set Time from Eeprom

  // read a byte from the current address of the EEPROM
  ahour = EEPROM.read(100);
  alarmhour = (int)ahour;
  if (alarmhour >24 ) {
    alarmhour = 0;
  }

  amin = EEPROM.read(101);
  alarmminute = (int)amin;
  if (alarmminute >60 ) {
    alarmminute = 0;
  }

  boolean savetimealarm = false; // If save button pushed save the time and alarm

 */




}

void displayalarmsetting(){ // Refresh the alarm settings


// Erase Current Alarm Time

    myGLCD.fillRect(60,50,200,50,TFT_BLACK);

    myGLCD.setTextSize(4); 


    if(alarmhour < 13){ // Annoyingly if number less than 10 doesnt print on RHS and misses zero so need to compensate

      if(alarmhour >9){
      myGLCD.drawNumber(alarmhour, 90, 55);   // If >= 10 just print hour
      } else {
      myGLCD.drawNumber(alarmhour, 110, 55);   // If >= 10 just print hour        
      }
      
       
    } else {
      if((alarmhour-12) >9){
      myGLCD.drawNumber((alarmhour-12), 90, 55);   // If >= 10 just print hour
      } else {
      myGLCD.drawNumber((alarmhour-12), 105, 55);   // If >= 10 just print hour        
      }
     
                
    }
// Draw am/pm label
   if ((alarmhour < 12) || (alarmhour == 24 )) {
        myGLCD.setTextSize(3); myGLCD.drawString("am", 260, 58); myGLCD.setTextSize(4);
   }    else { 
        myGLCD.setTextSize(3); myGLCD.drawString("pm", 260, 58); myGLCD.setTextSize(4); 
   }
    myGLCD.drawString(":", 160, 55);       

    if(alarmminute>=10){ // Annoyingly if number less than 10 doesnt print on RHS and misses zero so need to compensate
        myGLCD.drawNumber(alarmminute, 205, 55);   // If >= 10 just print minute
    } else {    
        myGLCD.drawString("0", 205, 55);
        myGLCD.drawNumber(alarmminute, 233, 55);      
    }    

    // Draw Alarm Status
        myGLCD.setTextSize(2);
       if (alarmstatus == true){ // flag where false is off and true is on
          myGLCD.setTextColor(TFT_GREEN,TFT_BLACK); 
          myGLCD.drawString("Alarm", 20, 188);
          myGLCD.drawString("Set", 28, 208);          
        } else {
          myGLCD.setTextColor(TFT_WHITE,TFT_BLACK);  
          myGLCD.drawString("Alarm", 20, 188);
          myGLCD.drawString("Off", 28, 208);
        }   
        myGLCD.drawRoundRect(10 , 180 , 80  , 50 , 2 , TFT_YELLOW);
        myGLCD.setTextColor(TFT_WHITE,TFT_BLACK); 
  
   // Alarm Set buttons
    myGLCD.setTextSize(3);
    myGLCD.drawString("+      +", 107, 20); 
    myGLCD.drawString("-      -", 107, 100);

// Setup the Alarm Save and Exit options 
    myGLCD.setTextSize(2);

    myGLCD.setTextColor(TFT_WHITE,TFT_BLACK);   myGLCD.setTextSize(2);
  
    myGLCD.drawString("Exit", 240, 188);   
    myGLCD.drawString("&", 294, 188); 
    myGLCD.drawString("Save", 245, 208);        
    myGLCD.drawRoundRect(230 , 180 , 80  , 50  , 2 , TFT_YELLOW);
  
}

void setupacmancharacter() { // Menu used to choose Pacman or Ms Pacman


// Toggle the pacman graphic 

         mspacman =  !mspacman;

    myGLCD.drawString("Character =", 80, 106);
           
    // Display MS Pacman or Pacman in menu - push to change
  if (mspacman == false) {
      drawicon(220, 100, r_o_pacman); //   Closed Ghost  
  } else {
      drawicon(220, 100, ms_r_o_pacman); //   Closed Ghost     
  }
  delay(500);
}

void playalarmsound1(){

  // UNMUTE sound to speaker via pin 5 of PAM8403 using ESP32 pin 32
  digitalWrite(32,HIGH);

 
  for (int startsound = 1; startsound < 8500000; startsound++) {
      
  DacAudio.FillBuffer();                // Fill the sound buffer with data
  if(Pacman.Playing==false)       // if not playing,
      DacAudio.Play(&Pacman);
  }

  // MUTE sound to speaker via pin 5 of PAM8403 using ESP32 pin 32
  digitalWrite(32,LOW);

}

void playalarmsound2(){

  // UNMUTE sound to speaker via pin 5 of PAM8403 using ESP32 pin 32
  digitalWrite(32,HIGH);

 
  for (int startsound = 1; startsound < 1500000; startsound++) {
      
  DacAudio.FillBuffer();                // Fill the sound buffer with data
  if(pacmangobble.Playing==false)       // if not playing,
      DacAudio.Play(&pacmangobble);
  }

  // MUTE sound to speaker via pin 5 of PAM8403 using ESP32 pin 32
  digitalWrite(32,LOW);

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

 
