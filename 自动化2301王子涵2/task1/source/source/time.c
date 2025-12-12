#include "sysclk.h"
#include "time.h"
//11059200Hz
#define TIMER0_RELOAD_HIGH 0xdc // Reload value for Timer0 high byte
#define TIMER0_RELOAD_LOW 0x00   // Reload value for Timer0 low byte
//10ms

uint Time_num = 0;

void Timer0_Init()
{
	TH0 = TIMER0_RELOAD_HIGH;           // 设置高8位
	TL0 = TIMER0_RELOAD_LOW;
	TMOD = 0x01;                        // 设置为16位模式
	ET0 = 1;                            // 使能定时器0
	TR0 = 1; //开启定时器0
}

void Timer0_ISR (void) interrupt 1
{
	TH0 = TIMER0_RELOAD_HIGH;           // Reinit Timer0 High register
	TL0 = TIMER0_RELOAD_LOW;
	Time_num++;
	if(Time_num >= 100) 
	{
		Time_num = 0;
		sec++;
		if(sec==60)
		{
			sec=0;
			min++;
		}
		if(min==60)
		{
			min=0;
			hour++;
		}
		if(hour==24)
		{
			hour=0;
			day++;
		}
	}
}