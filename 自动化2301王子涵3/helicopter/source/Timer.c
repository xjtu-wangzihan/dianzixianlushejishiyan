#include "Timer.h"


//void Timer2_Init (void)
//{
//	RCAP2H=0x10;
//	RCAP2L=0x00;
//	TH2=0x10;
//	TL2=0x00;
//	T2CON=0x04;
//	IE|= 0x20;
//}



//void Time2_ISR(void) interrupt 5
//{
//	int placelast=0;
//	unsigned int xdata temp;
//	vvalue=(vvalue+1)%64;
//	switch(waveflag)
//	{
//		case 1:
//			DAC0_result[0]=vvalue*64;
//			break;
//		case 2:
//			if(vvalue==0)
//			DAC0_result[0]=4095-DAC0_result[0];
//			break;
//		case 3:
//			DAC0_result[0]=sinn[vvalue];
//			break;
//		default:
//			DAC0_result[0]=0;
//			break;
//	}

//	TF2=0;
//	temp=DAC0_result[0];
//	temp&=0x00ff;
//	DAC0L=temp;
//	temp=DAC0_result[0];
//	temp>>=8;
//	DAC0H=temp;

//	if(waveflag)
//	{
//		int i,place=DAC0_result[0]/128;
//		int dir=(place>placelast)?1:-1;
//		if(column==0&&Position==0)
//			ClearAll();
//		for(i=placelast; i!=place; i+=dir) 
//		{
//			if(column<8) 
//			{
//				toshow[62-i*2]|=(0x80>>column);
//			}
//			else
//			{
//				toshow[62-i*2+1]|=(0x80>>(column-8));
//			}
//			WriteCommand(0x34);
//			WriteCommand(0x80+31-i);//row :0~32
//			WriteCommand(0x80+Position);//column	:80~87uphalf;88~8fdownhalf
//			WriteCommand(0x30);
//			WriteData(toshow[62-i*2]);
//			WriteData(toshow[62-i*2+1]);
//		}
//		if(column==0)
//			memset(toshow,0,sizeof(uchar)*64);
//		if(column<8) 
//		{
//			toshow[62-place*2]|=(0x80>>column);
//		}
//		else 
//		{
//			toshow[62-place*2+1]|=(0x80>>(column-8));
//		}

//		WriteCommand(0x34);
//		//set start position
//		WriteCommand(0x80+31-place);//row :0~32
//		WriteCommand(0x80+Position);//column	:80~87uphalf;88~8fdownhalf
//		//RE = 0
//		WriteCommand(0x30);
//		//write two bytes continuously
//		WriteData(toshow[62-place*2]);
//		WriteData(toshow[62-place*2+1]);
//		WriteCommand(0x32);
//		WriteCommand(0x36);
//		column++;
//		if(column==16) 
//		{
//			column=0;
//			Position=(Position+1)%8;
//		}
//		placelast=place;
//	}
//}