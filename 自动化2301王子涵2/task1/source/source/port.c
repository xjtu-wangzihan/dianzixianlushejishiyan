#include "sysclk.h"
#include "port.h"

void Port_IO_Init()
{
   P74OUT |= 0x01;//将p4低4位设置为推挽输出
   XBR2 |= 0x40;//使能交差开关
	 P4 &= 0xfd;//将p4.1为0
	 XBR1 |= 0x14; //使能外部中断1交叉开关
	 //XBR1 |= 0x10; //使能外部中断1交叉开关
	 XBR2 |= 0x44; //使能uart1
}
