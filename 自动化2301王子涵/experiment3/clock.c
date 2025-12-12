#include <reg51.h>
#include <stdio.h>
#include <string.h>
#define uint unsigned int
typedef unsigned char uint8;
#define uchar unsigned char
#define LCD1602_PORT P2
#define LCD1602_FLAG
#define LED P0

sbit BEEP = P3^4;
sbit KEY = P3^2;   // 按键，外部中断0
//LCD引脚
sbit lcd1602_RS=P3^0;
sbit lcd1602_RW=P3^1;
sbit lcd1602_EN=P3^7;
sbit lcd1602_busy=P2^7;
uchar idata tmp[15];
uint8 code LED_CODE[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};//0-9

volatile uint8 mode = 2; // 0计算器 2日历 1设置时间
volatile bit flag1 = 0; //1代表闹钟模式切回计算器
volatile bit flag2 = 0; //1代表计算器变成设置模式
volatile bit flag3 = 0; //1代表设置模式变成日历
volatile bit flag4 = 0; //1代表设置日历模式变成闹钟设置
const uchar code str_mode0[]="Calculator";
uchar keyval,keystr[12];

uchar data time_buf[15]; // 存储 "YYYYMMDDHHMMSS" 字符串，+1 终止
uchar time_buf_pos = 0;
unsigned int data sec=2,min=9,hour=15;
unsigned int data day=20, month=10;
unsigned int data year=2025;
unsigned int data sec1=30,min1=9;
unsigned int data hour1=14;
unsigned int data hh;
unsigned int data mm,ss;
#define TIMER0_RELOAD (65536 - 2000)

// 简单延时
void delay(uint t)
{
	uint i,j;
	for(i=0; i<t; i++)
		for(j=0; j<200; j++);
}

// 外部中断0服务函数
void ExtInt0_ISR() interrupt 0
{
	delay(20); // 防抖
	if(KEY == 0)
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
		{	flag4=1;
		}
	}
	while(KEY == 0);  // 等待松开
}

// 外部中断初始化
void InitExtInt0()
{
	IT0 = 1;  // 下降沿触发
	EX0 = 1;  // 允许外部中断0
	EA  = 1;  // 总中断允许
}

void lcd1602_CheckBusy()
{
	do
	{
		lcd1602_busy=1;
		lcd1602_RS=0;
		lcd1602_RW=1;
		lcd1602_EN=0;
		lcd1602_EN=1;
	}
	while(lcd1602_busy);
}

void lcd1602_WriteCmd(const uchar cmd)
{
	lcd1602_CheckBusy();
	lcd1602_RS=0;
	lcd1602_RW=0;
	lcd1602_EN=1;
	LCD1602_PORT=cmd;
	lcd1602_EN=0;
}


void lcd1602_WriteData(const uchar c)
{
	lcd1602_CheckBusy();
	lcd1602_RS=1;
	lcd1602_RW=0;
	lcd1602_EN=1;
	LCD1602_PORT=c;
	lcd1602_EN=0;
}


void lcd1602_Init()
{
	lcd1602_WriteCmd(0x38);
	lcd1602_WriteCmd(0x0c); //display enable,flag enable,flash enable,
	lcd1602_WriteCmd(0x06); //flag move to right,screen don't move
	lcd1602_WriteCmd(0x01); //clear screen
}

void lcd1602_Display(const uchar *ptr,uchar line)
{
	uchar data i=0;
	uchar *data q;

	q=ptr;
	switch(line)
	{
		case 0:
			lcd1602_WriteCmd(0x80);
			while(q!=NULL && (*q!='\0') && i<16)
			{
				lcd1602_WriteData(*q);
				q++;
				i++;
			}
			break;
		case 1:
			lcd1602_WriteCmd(0xc0);
			while(q!=NULL && (*q!='\0') && i<16)
			{
				lcd1602_WriteData(*q);
				q++;
				i++;
			}
			break;
	}
}
void lcd1602_ClearLine(uchar line)
{
	uchar i;
	if(line == 0) lcd1602_WriteCmd(0x80);
	else          lcd1602_WriteCmd(0xC0);
	for(i=0; i<16; i++) lcd1602_WriteData(' ');
	if(line == 0) lcd1602_WriteCmd(0x80);
	else          lcd1602_WriteCmd(0xC0);
}

uchar kbscan(void)
{
	unsigned char sccode,recode;
	P1=0x0f;
	if ((P1 & 0x0f) != 0x0f)
	{
		delay(20);
		if ((P1&0x0f)!= 0x0f)
		{
			sccode = 0xef;
			while((sccode&0x01)!=0)
			{
				P1=sccode;
				if((P1&0x0f)!=0x0f)
				{
					recode=(P1&0x0f)|0xf0;
					while((P1&0x0f)!=0x0f);
					return((~sccode)+(~recode));
				}
				else
					sccode=(sccode<<1)|0x01;
			}
		}
	}
	return 0;
}

uchar getkey(void)
{
	uchar key;
	key=kbscan();
	if(key==0)
	{
		keyval=0xff;
		return(0);
	}
	switch(key)
	{
		case 0x11:
			keyval=48; //0的ASCII码
			break;
		case 0x12:
			keyval=49;  //1
			break;
		case 0x14:
			keyval=50;   //2
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

	lcd1602_ClearLine(0);
	delay(50);
	lcd1602_Display("SET TIME:",0);
	// 初始显示占位 YYYYMMDDHHMMSS -> show underscores
	{
		uchar i;
		for(i=0; i<14; i++) tmp[i] = '_';
		tmp[14]=0;
		lcd1602_ClearLine(1);
		lcd1602_Display(tmp,1);
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
	lcd1602_Display(tmp,1);
}

void exit_time_set_mode_save()
{
	unsigned int y,mo,d;
	// 期望 time_buf 中有 14 位数字 YYYYMMDDHHMMSS
	if(time_buf_pos != 14)
	{
		lcd1602_ClearLine(0);
		lcd1602_Display("NEED 14 DIGITS",0);
		lcd1602_ClearLine(1);
		delay(1000);
		// 回到设置模式等待补全
		lcd1602_ClearLine(0);
		lcd1602_Display("SET TIME:",0);
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
		lcd1602_Display("WRONG TIME",0);
		lcd1602_ClearLine(1);
		delay(500);
		// 仍然在设置模式，等待重新输入
		lcd1602_Display("SET TIME:  ",0);
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
	lcd1602_Display("TIME SAVED",0);
	lcd1602_ClearLine(1);
	delay(300);
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

void Timer0_Init(void)
{
	TMOD &= 0xF0;     // 清除低四位（设置定时器0）
	TMOD |= 0x01;     // 模式1：16位定时器

	// 装载初值
	TH0 = (TIMER0_RELOAD >> 8) & 0xFF;
	TL0 = (TIMER0_RELOAD & 0xFF);

	ET0 = 1;   // 允许定时器0中断
	EA = 1;    // 允许总中断
	TR0 = 1;   // 启动定时器0
}

void Timer0_ISR(void) interrupt 1
{
	static unsigned int count = 0;
// 重装初值
	TH0 = (TIMER0_RELOAD >> 8) & 0xFF;
	TL0 = (TIMER0_RELOAD & 0xFF);
	count++;
	if(count >= 1000)
	{
		// 假设1ms中断一次
		count = 0;
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

void enter_clock_set_mode()
{
	time_buf_pos = 0;
	memset(time_buf,0,sizeof(time_buf));
	memset(tmp,0,sizeof(tmp));
	lcd1602_ClearLine(0);
	delay(50);
	lcd1602_Display("SET CLOCK:",0);
	sprintf(tmp, "%02d:%02d:%02d", hour1, min1, sec1);
	lcd1602_Display(tmp,1);
	delay(1000);
	memset(tmp,0,sizeof(tmp));
	// 初始显示占位 YYYYMMDDHHMMSS -> show underscores
	{
		uchar i;
		for(i=0; i<6; i++) tmp[i] = '_';
		tmp[6]=0;
		lcd1602_ClearLine(1);
		lcd1602_Display(tmp,1);
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
	lcd1602_Display(tmp,1);
}

void exit_clock_set_mode_save()
{
	// 期望 time_buf 中有 6位数字 HHMMSS
	if(time_buf_pos != 6)
	{
		lcd1602_ClearLine(0);
		lcd1602_Display("NEED 6 DIGITS",0);
		lcd1602_ClearLine(1);
		delay(1000);
		// 回到设置模式等待补全
		lcd1602_ClearLine(0);
		lcd1602_Display("SET CLOCK:",0);
		update_clock_set_display();
		return;
	}
	// 解析
	hh = (time_buf[0]-'0')*10 + (time_buf[1]-'0');
	mm = (time_buf[2]-'0')*10 + (time_buf[3]-'0');
	ss = (time_buf[4]-'0')*10 + (time_buf[5]-'0');

	if(!validate_date(2000,1,1,hh,mm,ss))
	{
		lcd1602_Display("WRONG CLOCK",0);
		lcd1602_ClearLine(1);
		delay(500);
		// 仍然在设置模式，等待重新输入
		lcd1602_Display("SET CLOCK: ",0);
		update_clock_set_display();
		return;
	}
	hour1 = hh;
	min1 = mm;
	sec1 = ss;
	mode = 2;
	flag3=1;
	lcd1602_Display("SAVE CLOCK",0);
	lcd1602_ClearLine(1);
	delay(1000);
}

void Display_Calendar()
{
	uchar data line0[10];
	uchar idata line1[8];
	// 格式化为 "YYYY-MM-DD"
	sprintf(line0, "%04d-%02d-%02d", year, month, day);
	// 格式化为 "HH:MM:SS"
	sprintf(line1, "%02d:%02d:%02d", hour, min, sec);

	lcd1602_Display(line0, 0);
	lcd1602_Display(line1, 1);
}

void main()
{
	uchar data buf[4];
	int i = 0;
	uchar num=0;
	uchar j;
	InitExtInt0();
	Timer0_Init();
	lcd1602_Init();
	BEEP = 0;

	while(1)
	{
		if(hour1==hour&&min1==min&&(sec1==sec||sec1==sec+1||sec1==sec-1))
		{
			for(j=0; j<100; j++)
			{	BEEP =1;
				delay(200);
				BEEP=0;
				delay(200);
				if(flag1||flag2||flag3||flag4)
					break;
			}
		}
		if (sec == 0 && min == 0)
		{
			BEEP = 1;
			delay(1000);
			BEEP = 0;
		}
		if(mode == 0)
		{
			if(flag1==1)
			{
				lcd1602_Display(str_mode0,0);
				lcd1602_ClearLine(1);
				flag1=0;
			}
			if(getkey())
			{
				if(keyval==35)
				{
					memset(keystr, 0, sizeof(keystr));
					num=0;
					lcd1602_ClearLine(1);
				}
				else
				{ 
					if(keyval>=48&&keyval<=57)
					{
						LED = LED_CODE[keyval-48];
					}
					keystr[num]=keyval;
					if(keyval=='=')
					{
						num++;
						sprintf(buf, "%d", calculate_expression(keystr));
						for(i=num; i<(num+strlen(buf)); i++)
							keystr[i]=buf[i-num];
					}
					lcd1602_Display(keystr,1);
					num++;
					num%=16;
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
				if(keyval==35)  // '#' 用作清除当前已输入（重新开始）
				{
					time_buf_pos = 0;
					memset(time_buf,0,sizeof(time_buf));
					update_time_set_display();
				}
				else if(keyval==61)    // '=' 保存并退出（如果合法）
				{
					exit_time_set_mode_save();
				}
				else if(keyval>='0' && keyval<='9')
				{
					if(time_buf_pos < 14)
					{
						time_buf[time_buf_pos++] = (uchar)keyval;
						update_time_set_display();
					}
					else
					{
						// 已满14位，短提示
						lcd1602_Display("FULL     ",0);
						lcd1602_ClearLine(1);
						delay(1000);
						lcd1602_Display("SET TIME:",0);
						update_time_set_display();
					}
				}
				// 忽略其他键
			}
		}
		else if(mode==2)
		{
			if(flag3==1)
			{
				lcd1602_ClearLine(1);
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
				if(keyval==35)  // '#' 用作清除当前已输入（重新开始）
				{
					time_buf_pos = 0;
					memset(time_buf,0,sizeof(time_buf));
					update_clock_set_display();
				}
				else if(keyval==61)    // '=' 保存并退出（如果合法）
				{
					exit_clock_set_mode_save();
				}
				else if(keyval>='0' && keyval<='9')
				{
					if(time_buf_pos < 6)
					{
						time_buf[time_buf_pos++] = (uchar)keyval;
						update_clock_set_display();
					}
					else
					{
						// 已满14位，短提示
						lcd1602_Display("FULL     ",0);
						lcd1602_ClearLine(1);
						delay(1000);
						lcd1602_Display("SET CLOCK:",0);
						update_clock_set_display();
					}
				}
			}
		}
	}
}
