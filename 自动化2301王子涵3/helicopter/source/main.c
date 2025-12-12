#include "ADC.h"
#include "DAC.h"
#include "Delay.h"
#include "SYS_Init.h"
#include "Timer.h"
#include "led.h"
#include "lcd.h" 
#include "c8051f020.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "picture.h"
#include "queue.h"
#include "pid.h"

uchar cure_data[2][1024];
uchar cure_data_flag = 0;

// Timers value
unsigned int  timer0_value;
unsigned int  timer1_value;
unsigned int  timer2_value;
unsigned int  timer3_value;
unsigned int  timer4_value;

// ADC & DAC variables
unsigned char channel;   // ADC 通道转换
unsigned int  vref;      // VREF
unsigned int  vtarget;   // VTARGET
unsigned int  vadc;      // ADC 取值
unsigned int  vadc_dec;  // ADC 取值
unsigned int  vdac;      // DAC 输出
unsigned int  vdac_dec;  // DAC 输出
unsigned char string[5];		//数字转换为字符串

unsigned int KEY_FLAG;

bit ifstart=0;
uchar column=0;
uchar Position=0;
uint vvalue=0;

uchar code str0[]="wzh 2232112665   ";
uchar code str1[]="helicopter        ";
uchar code str2[]="key2 set PID   ";
uchar code str3[]="key1 start/stop";

//PID展示
uchar code str10[]="      change Kp";
uchar code str11[]="Kp---";
uchar code str12[]="Ki:  ";
uchar code str13[]="Kd:  ";

uchar code str20[]="      change Ki";
uchar code str21[]="Kp:  ";
uchar code str22[]="Ki---";
uchar code str23[]="Kd:  ";

uchar code str30[]="      change Kd";
uchar code str31[]="Kp:  ";
uchar code str32[]="Ki:  ";
uchar code str33[]="D---";

PID pid={0,0,0,0,0,0,0,0};
Queue points;

//显示PID参数
void PID_display()
{
			WriteStr(0, 0, str0);
			WriteStr(1, 0, str1);
			WriteStr(2, 0, str2);
			WriteStr(3, 0, str3);
}

//根据索引显示PID参数
void display_pid_parameter(int i)
{
	switch(i)
	{
		case 0:
			// 显示PID参数
			WriteStr(0, 0, str10);
			WriteStr(1, 0, str11);
			WriteStr(2, 0, str12);
			WriteStr(3, 0, str13);
		  sprintf(string,"%.3f",pid.P);
		  WriteStr(1, 5, string);
			sprintf(string,"%.3f",pid.I);
			WriteStr(2, 5, string);
			sprintf(string,"%.3f",pid.D);
			WriteStr(3, 5, string);
			break;
		case 1:
			WriteStr(0, 0, str20);
			WriteStr(1, 0, str21);
			WriteStr(2, 0, str22);
			WriteStr(3, 0, str23);
			sprintf(string,"%.3f",pid.P);
			WriteStr(1, 5, string);
			sprintf(string,"%.3f",pid.I);
			WriteStr(2, 5, string);
			sprintf(string,"%.3f",pid.D);
			WriteStr(3, 5, string);
			break;
		case 2:
			WriteStr(0, 0, str30);
			WriteStr(1, 0, str31);
			WriteStr(2, 0, str32);
			WriteStr(3, 0, str33);
			sprintf(string,"%.3f",pid.P);
			WriteStr(1, 5, string);
			sprintf(string,"%.3f",pid.I);
			WriteStr(2, 5, string);
			sprintf(string,"%.3f",pid.D);
			WriteStr(3, 5, string);
			break;
	}
}

//定时器 1 初始化
void Timer1_Init_Main(void) 
{
    Enable_Timer1;       // 设定 IE 标志位 1，允许 Timer1 溢出中断请求
    Timer1_Set_Method1;  // 设置定时器 1 为方式 1 定时器功能
}

//定时器 1 中断服务函数
void Timer1_ISR(void) interrupt 3 
{
    timer1_value++;

    TH1 = 0x00; // 重新初始化Timer0 高位寄存器
    TL1 = 0x00;
}

//设备初始化
void Device_Init(void) 
{
    SYS_Init();  // SYS 初始化
		LcdInit();   
		LedInit();
    Timer1_Init_Main();
    Timer3_Init_ADC0(SYSCLK / SAMPLERATE);  // TIMER3 初始化 

    INT_Init();  // INT 中断初始化

    ADC0_Init();     // ADC0 初始化
    ADC0_Enable(1);  // 使能 ADC0
    DAC0_Init();     // DAC 初始化

    Timer1_Start;  // 设定 TCON 中断标志位 6，定时器 1 开启

    timer1_value = 0;
    timer3_value = 0;

    channel  = 1;
    vref     = 5244;
    vtarget  = 2800;
    vadc     = 0x0000;
    vadc_dec = 0;
    vdac     = 0x0000;
    vdac_dec = 0;
}

void Do(void) 
{
    if ((timer1_value & 0x0007) == 0x0001) 
		{     
        if (channel == 1) 	 
				{
            // 从 ADC0 AIN1 取得 10 位 16 进制数 vadc
            vadc = ADC_ValueReturn(channel);
            // 将 vadc 转化为 10 进制数进行计算
            vadc_dec = (unsigned long int)vadc * (unsigned long int)vref / 4096;
					//调用PID控制函数
					  PID_contrl(&pid,vadc_dec);
						vdac_dec=vadc_dec+pid.result;
						//vdac_dec=pid.result;
						// 将 10 进制数 vdac_dec 转化为 16 进制数
            vdac = (unsigned long int)vdac_dec * 4096 / (unsigned long int)vref;
            // 从 ADC0 输出 10 位 16 进制数 vdac
            DAC0_Output(vdac);			
						//调节目标霍尔电压值
						if(KEY_FLAG==1)
						{
							pid.setpoint-=50;
							KEY_FLAG=0;
						}
						else
						{
							if(KEY_FLAG==2)
							{
								pid.setpoint+=50;
								KEY_FLAG=0;
							}
          }
      }
		}
}

void setPID(void)
{
	uint k=0;
	while(k!=3)
	{
		//显示PID参数
		display_pid_parameter(k);
		//显示数字
		my_LedDispNum(vadc_dec,pid.setpoint,vdac_dec);
		Delay_ms(30);

		if(KEY_FLAG==1)
		{
			switch(k)
			{
				case 0:
					pid.P-=0.1;
				break;
				case 1:
					pid.I-=0.001;
				break;
				case 2:
					pid.D-=0.001;
				break;
			}
			//重置按键标志
			KEY_FLAG=0;
		}
		//按键标志为2	
		if(KEY_FLAG==2)
		{
			switch(k)
			{
				case 0:
					pid.P+=0.1;
				break;
				case 1:
					pid.I+=0.001;
				break;
				case 2:
					pid.D+=0.001;
				break;
			}
			KEY_FLAG=0;
		}
		  Delay_ms(30);
				//按键标志为 3
		if(KEY_FLAG==3)
		{
			Delay_ms(30);
			KEY_FLAG=0;
			k+=1;
		}
	}
	k=0;
}


void draw_buffer(uchar *buffer,Queue *Q)
{
	uchar num=Q->number;
	uchar index=Q->front;
	uchar count=0;
	while(num--!=0)
	{ 
		buffer[(63-Q->base[index])*16+(count>>3)]|=0x80>>(count&0x07);
		index=(index+1)%129;//实现index逐次往队尾移一位
		++count;
	}
}



uchar scale(uint vadc_dec)
{
	uchar level = 0;
//	level = (uchar)((vadc_dec-2000)/40);
		level = (uchar)((vadc_dec-2500)/32);

	return level;
}


int main(void) 
{
    int i=0,j=0,k=0;
		Device_Init();
	  PID_init(&pid);
		KEY_FLAG=0;
		InitQueue(&points);	
    while(1)
		{
			//Delay_ms(10);
			//LcdClear();
			//显示数字
			my_LedDispNum(vadc_dec,pid.setpoint,vdac_dec);
			//显示PID控制信息
			PID_display();
			if(KEY_FLAG==2)
			{			
				//清空LCD显示
				LcdClear();
				ImageShow(blank);
				KEY_FLAG=0;
				//设置PID参数
				setPID();
			}
			
			if(KEY_FLAG==1)
			{
				LcdClear();
				ImageShow(blank);
				KEY_FLAG=0;
				//当按键标志不等于3时
				while(KEY_FLAG!=3)
				{	
					//显示数字
					my_LedDispNum(vadc_dec,pid.setpoint,vdac_dec);
					Do();
					i+=1;
					if(i == 2)  //调
					{
						//重置
						i = 0;
						EnQueue(&points,scale(vadc_dec));
						
						//清空缓冲器
						memset(cure_data[cure_data_flag],0,1024*sizeof(uchar));//刷新缓冲器，全部赋0
						draw_buffer(cure_data[cure_data_flag],&points);//将队列中的点绘制到缓冲区
						ScreenShow(cure_data[cure_data_flag],cure_data[!cure_data_flag]);//画点，利用2个缓冲区	
						cure_data_flag=!cure_data_flag;//缓冲区互换角色
					}
				}
				pid.sumerror = 0;
				pid.lasterror = 0;
	pid.preverror = 0;	
					//设置vdac=0
					vdac=0;
					//输出vdac到DAC0
					DAC0_Output(vdac);
					//重置按键标志
					KEY_FLAG=0;
					//初始化LCD
					LcdInit();
				}
    }
}

// 中断服务函数
void INT1_ISR(void) interrupt 2
{
	Delay_ms(20);
	
	//根据P5的值进行判断
	switch(P5)
	{
		case 0xfb://左键
      KEY_FLAG=1;
			break;		
		
		case 0xfd://中键
			KEY_FLAG=2;
			break;
		
		case 0xfe://右键
			KEY_FLAG=3;
			break;
	}
}
