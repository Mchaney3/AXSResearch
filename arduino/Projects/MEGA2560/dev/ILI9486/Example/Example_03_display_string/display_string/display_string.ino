// IMPORTANT: LCDWIKI_KBV LIBRARY MUST BE SPECIFICALLY
// CONFIGURED FOR EITHER THE TFT SHIELD OR THE BREAKOUT BOARD.

//This program is a demo of displaying string
//Set the pins to the correct ones for your development shield or breakout board.
//the 16bit mode only use in Mega.you must modify the mode in the file of lcd_mode.h
//when using the BREAKOUT BOARD only and using these 16 data lines to the LCD,
//pin usage as follow:
//             CS  CD  WR  RD  RST  D0  D1  D2  D3  D4  D5  D6  D7  D8  D9  D10  D11  D12  D13  D14  D15 
//Arduino Mega 40  38  39  /   41   37  36  35  34  33  32  31  30  22  23  24   25   26   27   28   29

//Remember to set the pins to suit your display module!

/**********************************************************************************
* @attention
*
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
* TIME. AS A RESULT, QD electronic SHALL NOT BE HELD LIABLE FOR ANY
* DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
* FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE 
* CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
**********************************************************************************/

#include <LCDWIKI_GUI.h> //Core graphics library
#include <LCDWIKI_KBV.h> //Hardware-specific library

//the definiens of 16bit mode as follow:
//if the IC model is known or the modules is unreadable,you can use this constructed function
LCDWIKI_KBV mylcd(ILI9486,40,38,39,-1,41); //model,cs,cd,wr,rd,reset

//define some colour values
#define  BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

void setup() 
{
  mylcd.Init_LCD();
  mylcd.Fill_Screen(BLACK); 
}

void loop() 
{
  mylcd.Set_Rotation(1); 
  mylcd.Set_Text_Mode(0);
  mylcd.Set_Text_colour(RED);
  mylcd.Set_Text_Back_colour(BLACK);
  mylcd.Set_Text_Size(1);
  mylcd.Print_String("a b c d e f g h i j k l m n o p q r s t u v w x y z", 87, 10);
  mylcd.Print_String("A B C D E F G H I J K L M N O P Q R S T U V W X Y Z", 87, 20);

  mylcd.Set_Text_colour(GREEN);
  mylcd.Set_Text_Size(2);
  mylcd.Print_String("a b c d e f g h i j k l m", 90, 40);
  mylcd.Print_String("n o p q r s t u v w x y z", 90, 60);
  mylcd.Print_String("A B C D E F G H I J K L M", 90, 80);
  mylcd.Print_String("N O P Q R S T U V W X Y Z", 90, 100);

  mylcd.Set_Text_colour(BLUE);
  mylcd.Set_Text_Size(3);
  mylcd.Print_String("a b c d e f g h i j k l m", 15, 130);
  mylcd.Print_String("n o p q r s t u v w x y z", 15, 160);
  mylcd.Print_String("A B C D E F G H I J K L M", 15, 190);
  mylcd.Print_String("N O P Q R S T U V W X Y Z", 15, 220);

  mylcd.Set_Text_colour(CYAN);
  mylcd.Set_Text_Size(1);
  mylcd.Print_String("0 1 2 3 4 5 6 7 8 9", 183, 255);
  mylcd.Set_Text_colour(WHITE);
  mylcd.Set_Text_Size(2);
  mylcd.Print_String("0 1 2 3 4 5 6 7 8 9", 126, 265);
  mylcd.Set_Text_colour(MAGENTA);
  mylcd.Set_Text_Size(3);
  mylcd.Print_String("0 1 2 3 4 5 6 7 8 9", 69, 285);  
  //display 1 times string
  delay(2000);
  mylcd.Set_Rotation(0); 
  mylcd.Fill_Screen(0x0000);
  mylcd.Set_Text_colour(RED);
  mylcd.Set_Text_Back_colour(BLACK);
  mylcd.Set_Text_Size(1);
  mylcd.Print_String("Hello World!", 0, 0);
  mylcd.Print_Number_Float(01234.56789, 2, 0, 8, '.', 0, ' ');  
  mylcd.Print_Number_Int(0xDEADBEF, 0, 16, 0, ' ',16);
  //mylcd.Print_String("DEADBEF", 0, 16);

  //display 2 times string
  mylcd.Set_Text_colour(GREEN);
  mylcd.Set_Text_Size(2);
  mylcd.Print_String("Hello World!", 0, 40);
  mylcd.Print_Number_Float(01234.56789, 2, 0, 56, '.', 0, ' ');  
  mylcd.Print_Number_Int(0xDEADBEF, 0, 72, 0, ' ',16);
  //mylcd.Print_String("DEADBEEF", 0, 72);

  //display 3 times string
  mylcd.Set_Text_colour(BLUE);
  mylcd.Set_Text_Size(3);
  mylcd.Print_String("Hello World!", 0, 104);
  mylcd.Print_Number_Float(01234.56789, 2, 0, 128, '.', 0, ' ');  
  mylcd.Print_Number_Int(0xDEADBEF, 0, 152, 0, ' ',16);
 // mylcd.Print_String("DEADBEEF", 0, 152);

  //display 4 times string
  mylcd.Set_Text_colour(WHITE);
  mylcd.Set_Text_Size(4);
  mylcd.Print_String("Hello!", 0, 192);

  //display 5 times string
  mylcd.Set_Text_colour(YELLOW);
  mylcd.Set_Text_Size(5);
  mylcd.Print_String("Hello!", 0, 224);

  //display 6 times string
  mylcd.Set_Text_colour(CYAN);
  mylcd.Set_Text_Size(6);
  mylcd.Print_String("Hello!", 0, 266);

  //display 7 times string
  mylcd.Set_Text_colour(RED);
  mylcd.Set_Text_Size(7);
  mylcd.Print_String("Hello!", 0, 315);

  //display 8 times string
  mylcd.Set_Text_colour(MAGENTA);
  mylcd.Set_Text_Size(8);
  mylcd.Print_String("Hello!", 0, 372);
  delay(2000);
  mylcd.Fill_Screen(0x0000);
}
