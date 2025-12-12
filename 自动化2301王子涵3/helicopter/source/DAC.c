#include "DAC.h"

void DAC0_Init(void) {
    DAC0_Ctr_Set;  // 打开 DAC0，右对齐模式，输出更新发生在写 DAC0H 时
}


void DAC0_Output(unsigned int val) {
    DAC0L = (unsigned char)val;         //送DA值低2位
    DAC0H = (unsigned char)(val >> 8);  //送DA值高8位
}

