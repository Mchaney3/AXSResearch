#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "usart.h"
#include "lcd.h"
#include "spi.h"
#include "flash.h"
#include "mmc_sd.h"
#include "ff.h"
#include "integer.h"
#include "diskio.h"
#include "text.h"
#include "24cxx.h"
#include "fontupd.h"
#include "stdio.h"
#include "string.h"
#include "picdecoder.h"

/*********************************************************/
//#define PIC_QVGA	    1
//#define PIC_WQVGA	 	1
#define PIC_HVGA		1

#ifdef PIC_QVGA
#define IMAGE_W 240 
#define IMAGE_H 320 
#define IMAGE_DIR "0:/PICTURE/240320"
#endif

#ifdef PIC_WQVGA
#define IMAGE_W 240 
#define IMAGE_H 400 
#define IMAGE_DIR "0:/PICTURE/240400"
#endif

#ifdef PIC_HVGA
#define IMAGE_W 320 
#define IMAGE_H 480 
#define IMAGE_DIR "0:/PICTURE/320480"
#endif
/*********************************************************/
void viewPictures(const char *fileDir){
    DIR dir;
	BOOL result;
	FRESULT res;
	FILINFO fileInfo;
	char *filename;
	char  temp[256];
#if _USE_LFN
    static char lfn[_MAX_LFN + 1];
    fileInfo.lfname = lfn;
    fileInfo.lfsize = sizeof(lfn);
#endif
	delay_ms(300);
	LCD_Clear(BRRED);
 	while(1)
	{
  	   res= f_opendir(&dir,  fileDir);
	   if(res==FR_OK)
       {  	
	     for(;;)
		 {
			 //if(1==KEY_Scan())
			 //{
							res =f_readdir(&dir,&fileInfo);
						if(res!=0||fileInfo.fname[0]==0)break;
						if(fileInfo.fname[0]=='.')continue  ;
	#if _USE_LFN
							filename = *fileInfo.lfname ? fileInfo.lfname : fileInfo.fname;
	#else
							filename = fileInfo.fname;
	#endif
							if ( !(fileInfo.fattrib & AM_DIR) ) 
					{
						 strcpy(temp,fileDir);
					 strcat(temp,"/");
					 LCD_Clear(WHITE);//清屏,加载下一幅图片的时候,一定清屏
							 result=AI_LoadPicFile(((u8*)strcat(temp,filename)),0,0,IMAGE_W,IMAGE_H);
							 if(result==FALSE) continue;
						 POINT_COLOR=RED;
						 Show_Str(0,0,(u8 *)filename,16,1);//显示图片名字
						 //while(!KEY_Scan());
							 delay_ms(1000); delay_ms(1000); delay_ms(1000); 
						delay_ms(1000); delay_ms(1000); 
						}
				//}
	      } 
	   }	 
	 }
  }
 int main(void)
 {
	const char filedir[]=IMAGE_DIR;
	FATFS fs;
 	SystemInit();
	delay_init(72);	     //延时初始化
	NVIC_Configuration();
	uart_init(9600);
 	LED_Init();
  	KEY_Init();	
    LCD_Init();
	//LCD_Clear(RED);delay_ms(1000);	delay_ms(1000);delay_ms(1000);
	//LCD_Clear(GREEN);delay_ms(1000);	delay_ms(1000);delay_ms(1000);
	//LCD_Clear(BLUE);delay_ms(1000);	delay_ms(1000);delay_ms(1000);
	SPI_Flash_Init();	//SPI FLASH初始化

	f_mount(0, &fs);
   /*
	while(font_init())
	{
    	POINT_COLOR=RED;      
		LCD_Clear(WHITE);
		LCD_ShowString(60,50,"Mini STM32");	
		LCD_ShowString(60,70,"Font Updating..."); 	 
 		while(update_font())//从SD卡更新
	    {   
	       	LCD_ShowString(60,90,"FAT SYS ERROR.      ");	 
			delay_ms(200);     
			LED0=!LED0;	
		}
	} */
	POINT_COLOR=RED;      
	Show_Str(60,50,"Mini STM32 DevelopBoard",16,0);				    	 
	Show_Str(60,70,"PicDecode Demo",16,0);				    	 
	Show_Str(60,90,"xiaoFeng@QDtech",16,0);				    	 
	Show_Str(60,110,"2014/05/02",16,0);  
	Show_Str(60,130,"Star Display...",16,0); 
	delay_ms(1000);

	viewPictures(filedir);	//一级目录浏览

 }
