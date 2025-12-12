#include <stdio.h>
#include "c8051f020.h"
#include "lcd.h"
#include "dight.h"
#include "port.h"
#include "sysclk.h"
#include "time.h"
#include <string.h>
#include "1.h"
#define uint unsigned int
#define uchar unsigned char

sbit INT1 = P0^3; //外部中断按键
volatile uchar mode = 2; // 0计算器 2日历 1设置时间 3设置闹钟
volatile bit flag1 = 0; //1代表闹钟模式切回计算器
volatile bit flag2 = 0; //1代表计算器变成设置模式
volatile bit flag3 = 0; //1代表设置模式变成日历
volatile bit flag4 = 0; //1代表设置日历模式变成闹钟设置
volatile bit keyflag = 0;
uchar idata tmp[15];
uchar keyval;
uchar idata keystr[17];
volatile uchar key = 0x11;
volatile uchar key_index=0;
uchar idata time_buf[15]; // 存储 "YYYYMMDDHHMMSS" 字符串，+1 终止
uchar time_buf_pos = 0;
const uchar code str_mode0[]="Calculator";
unsigned int data sec=2,min=9,hour=15;
unsigned int data day=20, month=10;
unsigned int data year=2025;
unsigned int data sec1=30,min1=9;
unsigned int data hour1=14;
unsigned int idata hh;
unsigned int idata mm,ss;

uchar idata key_table[16]=
{0x11,0x12,0x14,0x18,0x21,0x22,0x24,0x28,0x41,0x42,0x44,0x48,0x81,0x82,0x84,0x88}; // 0-9  +-*/ = #

//初始化外部中断1
void INT1_Init()
{
	//INT1
	//low level triggle
	IT1 = 0;
	//enable INT1
	EX1 = 1;
	//enable all interrupt
	EA = 1;
}

void INT1_ISR(void) interrupt 2
{
	Delay_ms(50);
	if(INT1 == 0)
	{
	switch(P5)
	{
			case 0xfb:  //  左键
			{
				mode++;
				if(mode > 3)
				{
					mode = 0;
					flag1=1;
				}
				else if(mode==1)
				{
					flag2=1;
				}
				else if(mode==2)
				{
					flag3=1;
				}
				else if(mode==3)
				{	
					flag4=1;
				}
			}
			break;
			
	  	case 0xfd:   //  中间键
			{
				key_index++;
				if(key_index>=16)
					key_index = 0;
				key = key_table[key_index];
			}
			break;	
		
		  case 0xfe:    //  右键
			{	
				keyflag = 1;
			}
				break;		
	}
		}
	while(!INT1);
}

uchar getkey(void)
{
	switch(key)
	{
		case 0x11:
			keyval=48; //0的ASCII码
			break;
		case 0x12:
			keyval=49;  //1
			break;
		case 0x14:
			keyval=50;  //2
			break;
		case 0x18:
			keyval=51;
			break;
		case 0x21:
			keyval=52;
			break;
		case 0x22:
			keyval=53;
			break;
		case 0x24:
			keyval=54;
			break;
		case 0x28:
			keyval=55;
			break;
		case 0x41:
			keyval=56;
			break;
		case 0x42:
			keyval=57;
			break;
		case 0x44:
			keyval=43;// +
			break;
		case 0x48:
			keyval=45;// -
			break;
		case 0x81:
			keyval=42;//  *
			break;
		case 0x82:
			keyval=47;//  /
			break;
		case 0x84:
			keyval=61;// =
			break;
		case 0x88:
			keyval=35; //"#"清除表达式
			break;
		default:
			keyval=0xff;
			break;
	}
	if(keyval!=0xff)
		return(1);
	else
		return(0);
}

// 基本日期合法性检查（简化：未处理闰年2月天数精确）
int validate_date(int y,int m,int d,int hh,int mm,int ss)
{
	if(y < 1970 || y > 9999) return 0;
	if(m < 1 || m > 12) return 0;
	if(d < 1) return 0;
	if(hh < 0 || hh > 23) return 0;
	if(mm < 0 || mm > 59) return 0;
	if(ss < 0 || ss > 59) return 0;
	// 简化每月天数判断
	if(m==1||m==3||m==5||m==7||m==8||m==10||m==12)
	{
		if(d>31) return 0;
	}
	else if(m==4||m==6||m==9||m==11)
	{
		if(d>30) return 0;
	}
	else     // 2月
	{
		// 闰年判断
		int leap = ( (y%4==0 && y%100!=0) || (y%400==0) );
		if(leap)
		{
			if(d>29) return 0;
		}
		else
		{
			if(d>28) return 0;
		}
	}
	return 1;
}

void enter_time_set_mode()
{
	time_buf_pos = 0;
	memset(time_buf,0,sizeof(time_buf));

	LineClear(0);
	Delay_ms(50);
	WriteStr(0,0,"SET TIME:");
	// 初始显示占位 YYYYMMDDHHMMSS -> show underscores
	{
		uchar i;
		for(i=0; i<14; i++) tmp[i] = '_';
		tmp[14]=0;
		LineClear(1);
		WriteStr(1,0,tmp);
	}
}

void update_time_set_display()
{
	uchar i;
	for(i=0; i<14; i++)
	{
		if(i < time_buf_pos)
			tmp[i] = time_buf[i];
		else tmp[i] = '_';
	}
	tmp[14]=0;
	WriteStr(1,0,tmp);
}

void exit_time_set_mode_save()
{
	unsigned int y,mo,d;
	// 期望 time_buf 中有 14 位数字 YYYYMMDDHHMMSS
	if(time_buf_pos != 14)
	{
		LineClear(0);
		WriteStr(0,0,"NEED 14 DIGITS");
		LineClear(1);
		Delay_ms(1000);
		// 回到设置模式等待补全
		LineClear(0);
		WriteStr(0,0,"SET TIME:");
		update_time_set_display();
		return;
	}
	// 解析
	y = (time_buf[0]-'0')*1000 + (time_buf[1]-'0')*100 + (time_buf[2]-'0')*10 + (time_buf[3]-'0');
	mo = (time_buf[4]-'0')*10 + (time_buf[5]-'0');
	d  = (time_buf[6]-'0')*10 + (time_buf[7]-'0');
	hh = (time_buf[8]-'0')*10 + (time_buf[9]-'0');
	mm = (time_buf[10]-'0')*10 + (time_buf[11]-'0');
	ss = (time_buf[12]-'0')*10 + (time_buf[13]-'0');

	if(!validate_date(y,mo,d,hh,mm,ss))
	{
		WriteStr(0,0,"WRONG TIME");
		LineClear(1);
		Delay_ms(500);
		// 仍然在设置模式，等待重新输入
		WriteStr(0,0,"SET TIME:  ");
		update_time_set_display();
		return;
	}
	year = y;
	month = mo;
	day = d;
	hour = hh;
	min = mm;
	sec = ss;
	mode = 2;
	flag3=1;
	WriteStr(0,0,"TIME SAVED");
	LineClear(1);
	Delay_ms(300);
}

int calculate_expression(uchar *exp)
{
	int num1 = 0, num2 = 0;
	char op = 0;
	int i = 0;

	// 解析第一个数
	while (exp[i] >= '0' && exp[i] <= '9')
	{
		num1 = num1 * 10 + (exp[i] - '0');
		i++;
	}

	op = exp[i++]; // 运算符 + - * /

	// 解析第二个数
	while (exp[i] >= '0' && exp[i] <= '9')
	{
		num2 = num2 * 10 + (exp[i] - '0');
		i++;
	}

	// 执行运算
	switch(op)
	{
		case '+':
			return num1 + num2;
		case '-':
			return num1 - num2;
		case '*':
			return num1 * num2;
		case '/':
			if (num2 == 0) return 0;  // 除零保护
			return num1 / num2;
		default:
			return 0;
	}
}

void enter_clock_set_mode()
{
	time_buf_pos = 0;
	memset(time_buf,0,sizeof(time_buf));
	memset(tmp,0,sizeof(tmp));
	LineClear(0);
	Delay_ms(50);
	WriteStr(0,0,"SET CLOCK:");
	sprintf(tmp, "%02d:%02d:%02d", hour1, min1, sec1);
	WriteStr(1,0,tmp);
	Delay_ms(1000);
	memset(tmp,0,sizeof(tmp));
	// 初始显示占位 HHMMSS -> show underscores
	{
		uchar i;
		for(i=0; i<6; i++) tmp[i] = '_';
		tmp[6]=0;
		LineClear(1);
		WriteStr(1,0,tmp);
	}
}

void update_clock_set_display()
{
	uchar i;
	for(i=0; i<6; i++)
	{
		if(i < time_buf_pos)
			tmp[i] = time_buf[i];
		else tmp[i] = '_';
	}
	tmp[6]=0;
	WriteStr(1,0,tmp);
}

void exit_clock_set_mode_save()
{
	// 期望 time_buf 中有 6位数字 HHMMSS
	if(time_buf_pos != 6)
	{
		LineClear(0);
		WriteStr(0,0,"NEED 6 DIGITS");
		LineClear(1);
		Delay_ms(1000);
		// 回到设置模式等待补全
		LineClear(0);
		WriteStr(0,0,"SET CLOCK:");
		update_clock_set_display();
		return;
	}
	// 解析
	hh = (time_buf[0]-'0')*10 + (time_buf[1]-'0');
	mm = (time_buf[2]-'0')*10 + (time_buf[3]-'0');
	ss = (time_buf[4]-'0')*10 + (time_buf[5]-'0');

	if(!validate_date(2000,1,1,hh,mm,ss))
	{
		WriteStr(0,0,"WRONG CLOCK");
		LineClear(1);
		Delay_ms(500);
		// 仍然在设置模式，等待重新输入
		WriteStr(0,0,"SET CLOCK: ");
		update_clock_set_display();
		return;
	}
	hour1 = hh;
	min1 = mm;
	sec1 = ss;
	mode = 2;
	flag3=1;
	WriteStr(0,0,"SAVE CLOCK");
	LineClear(1);
	Delay_ms(1000);
}

void Display_Calendar()
{
	uchar idata line0[11];
	uchar idata line1[9];
	// 格式化为 "YYYY-MM-DD"
	sprintf(line0, "%04d-%02d-%02d", year, month, day);
	// 格式化为 "HH:MM:SS"
	sprintf(line1, "%02d:%02d:%02d", hour, min, sec);

	WriteStr(0,0,line0);
	WriteStr(1,0,line1);
}

void Init_Device(void)
{
    //disable watchdog
    EA = 0;
    WDTCN = 0xde;
    WDTCN = 0xad;
    EA = 1;
    SYSCLK_Init();
    Port_IO_Init();
		Timer0_Init();
		INT1_Init();
    LcdInit();
		INT1 = 1;
}


void main(void)
{
		uchar idata buf[5];
		int i = 0;
		uchar num1=0;
		uchar j;
		Init_Device();
		
		WriteStr(0,0,"欢迎使用");
		WriteStr(1,0,"左键切换模式");
		WriteStr(2,0,"中键选择输入");
		WriteStr(3,0,"右键确认输入");
		Delay_ms(3000);
		LcdClear();
	
	  while(1)
		{
			P5 = 0xFB;Delay_us(5);
			if(hour1==hour&&min1==min&&(sec1==sec||sec1==sec+1||sec1==sec-1))
			{
				LcdClear();
				ImageShow2(clock);
				for(j=0; j<1000; j++)
				{	
					P4=0XFF;
					Delay_ms(200);
					P4=0XFD;
					Delay_ms(200);
					if(flag1||flag2||flag3||flag4)
					{LcdInit();
						break;}
				}
			}
			P5 = 0xFB;Delay_us(5);
			P5 = 0xFD;Delay_us(5);
			P5 = 0xFE;Delay_us(5);
			if(mode == 0)
		{
			if(flag1==1)
			{
				WriteStr(0,0,str_mode0);
				LineClear(1);
				flag1=0;
			}
			if(getkey())
			{
				up_dight(key_index);
				if(keyval==35&&keyflag == 1)
				{
					memset(keystr, 0, sizeof(keystr));
					num1=0;
					LineClear(1);
					keyflag = 0;
				}
				else
				{
					if(keyflag == 1)
					{
						keystr[num1]=keyval;
						num1++;
						num1 = num1%16;
						if(keyval== '='&& keyflag==1)
						{
						sprintf(buf, "%d", calculate_expression(keystr));
						for(i=num1; i<(num1+strlen(buf)); i++)
							keystr[i]=buf[i-num1];
						}
					keyflag = 0;
					}
					keystr[16] = '\0';
					WriteStr(1,0,keystr);					
				}
			}
		}
		else if(mode==1)
		{
			if(flag2==1)
			{
				enter_time_set_mode();
				flag2=0;
			}
			// 在设置模式下，只响应数字键与 '=' 与 '#'
			if(getkey())
			{
				up_dight(key_index);
				if(keyval==35 && keyflag ==1)  // '#' 用作清除当前已输入（重新开始）
				{
					time_buf_pos = 0;
					memset(time_buf,0,sizeof(time_buf));
					update_time_set_display();
					keyflag = 0;
				}
				else if(keyval==61&& keyflag ==1)    // '=' 保存并退出（如果合法）
				{
					exit_time_set_mode_save();
					keyflag =0;
				}
				else if(keyval>='0' && keyval<='9' &&keyflag ==1)
				{
					if(time_buf_pos < 14)
					{
						time_buf[time_buf_pos++] = (uchar)keyval;
						update_time_set_display();
					}
					else
					{
						// 已满14位，短提示
						WriteStr(0,0,"FULL     ");
						LineClear(1);
						Delay_ms(1000);
						WriteStr(0,0,"SET TIME:");
						update_time_set_display();
					}
					keyflag =0;
				}
			}
		}
		else if(mode==2)
		{
			if(flag3==1)
			{
				LineClear(1);
				LineClear(0);
				LineClear(2);
				LineClear(3);
				flag3=0;
			}
			Display_Calendar();
		}
		else
		{
			if(flag4==1)
			{
				enter_clock_set_mode();
				flag4=0;
			}
			// 在设置模式下，只响应数字键与 '=' 与 '#'
			if(getkey())
			{
				up_dight(key_index);
				if(keyval==35 && keyflag ==1)  // '#' 用作清除当前已输入（重新开始）
				{
					time_buf_pos = 0;
					memset(time_buf,0,sizeof(time_buf));
					update_clock_set_display();
					keyflag = 0;
				}
				else if(keyval==61&&keyflag ==1)    // '=' 保存并退出（如果合法）
				{
					exit_clock_set_mode_save();
					keyflag =0;
				}
				else if(keyval>='0' && keyval<='9'&&keyflag == 1)
				{
					if(time_buf_pos < 6)
					{
						time_buf[time_buf_pos++] = (uchar)keyval;
						update_clock_set_display();
					}
					else
					{
						// 已满6位，短提示
						WriteStr(0,0,"FULL     ");
						LineClear(1);
						Delay_ms(1000);
						WriteStr(0,0,"SET CLOCK:");
						update_clock_set_display();
					}
					keyflag = 0;
				}
			}
		}
	}
}
