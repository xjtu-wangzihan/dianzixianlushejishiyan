#ifndef LCD_H
#define LCD_H

#include "c8051f020.h"
#include <intrins.h>
#include <stdio.h>

extern volatile int wavevalue[128];
extern volatile int setwavevalue[128];
void Lcd_Port_Init(void);
void LcdInit(void);


void SendByte(uchar Dbyte);
uchar ReceiveByte(void);
void CheckBusy(void);

void WriteData(uchar WDLCM);
void WriteCommand (uchar Cbyte);

void LineClear(uchar l);
void LcdClear( void );

void WriteChar(uchar row,uchar col,uchar ch);
void WriteStr(uchar row,uchar col,uchar *puts);

void ImageShow(uchar *imagePtr);
//void LcdShowPoint(unsigned char x);
void drawPoint2(uchar x_index,uchar y,unsigned char tempH,unsigned char tempL, uchar color);
void ScreenShow(uchar *imagePtr,uchar *reference);
void LcdShowPoint(unsigned char x);
void LcdShowSetPoint(unsigned char x);

#endif
