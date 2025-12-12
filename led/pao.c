//#include <reg52.h>
//#define uchar unsigned char

//// 外部中断切换模式变量
//volatile uchar g_songSelect = 0;

//// 延时函数
//void delay()
//{
//    uchar a,b;
//    for(a=200;a;a--)       // 调整延时，避免太长
//        for(b=200;b;b--);
//}

//// 外部中断0服务函数（模式切换）
//void ExtInt0_ISR() interrupt 0
//{
//    delay();                // 简单防抖
//    g_songSelect ^= 1;      // 在 0 和 1 之间切换
//}

//// 外部中断初始化
//void InitExtInt0()
//{
//    IT0 = 1;   // 下降沿触发
//    EX0 = 1;   // 允许外部中断0
//    EA  = 1;   // 总中断允许
//    IP  = 0x01; // 设置 INT0 高优先级
//}

//void main()
//{
//    uchar tmp = 1;
//    uchar dir = 0;  // 流水灯方向：0=左移，1=右移

//    InitExtInt0();  // 初始化外部中断

//    while(1)
//    {
//        if(g_songSelect == 0)  // 模式0：单向流水灯
//        {
//            P0 = ~tmp;          // 输出 LED 灯
//            delay();
//            tmp <<= 1;
//            if(tmp == 0) tmp = 1;
//        }
//        else if(g_songSelect == 1)  // 模式1：双向流水灯
//        {
//            P0 = ~tmp;
//            delay();
//            if(dir == 0)  // 左移
//            {
//                tmp <<= 1;
//                if(tmp == 0x80) dir = 1;  // 到达左边界，反向
//            }
//            else          // 右移
//            {
//                tmp >>= 1;
//                if(tmp == 0x01) dir = 0;  // 到达右边界，反向
//            }
//        }
//    }
//}
#include <reg52.h>
#define uchar unsigned char

// 外部中断切换模式变量
volatile uchar g_songSelect = 0;

// 延时函数
void delay()
{
    uchar a,b;
    for(a=200;a;a--)
        for(b=200;b;b--);
}

// 外部中断0服务函数（模式切换）
void ExtInt0_ISR() interrupt 0
{
    delay();               // 简单防抖
    g_songSelect++;        // 模式切换
    if(g_songSelect > 2)   // 超过最大模式返回0
        g_songSelect = 0;
}

// 外部中断初始化
void InitExtInt0()
{
    IT0 = 1;   // 下降沿触发
    EX0 = 1;   // 允许外部中断0
    EA  = 1;   // 总中断允许
    IP  = 0x01; // 设置 INT0 高优先级
}

void main()
{
    uchar tmp = 1;
    uchar dir = 0;  // 流水灯方向：0=左移，1=右移
    uchar toggle = 0; // 全亮全灭切换标志

    InitExtInt0();  // 初始化外部中断

    while(1)
    {
        if(g_songSelect == 0)  // 模式0：单向流水灯
        {
            P0 = ~tmp;
            delay();
            tmp <<= 1;
            if(tmp == 0) tmp = 1;
        }
        else if(g_songSelect == 1)  // 模式1：双向流水灯
        {
            P0 = ~tmp;
            delay();
            if(dir == 0)  // 左移
            {
                tmp <<= 1;
                if(tmp == 0x80) dir = 1;  // 到达左边界，反向
            }
            else          // 右移
            {
                tmp >>= 1;
                if(tmp == 0x01) dir = 0;  // 到达右边界，反向
            }
        }
        else if(g_songSelect == 2)  // 模式2：全亮全灭
        {
            if(toggle == 0)
            {
                P0 = 0x00;   // 全亮（低电平点亮）
            }
            else
            {
                P0 = 0xFF;   // 全灭
            }
            toggle ^= 1;      // 切换状态
            delay();
        }
    }
}