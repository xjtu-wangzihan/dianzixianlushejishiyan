#ifndef TIME_H
#define TIME_H

extern unsigned int data sec,min,hour;
extern unsigned int data day, month;
extern unsigned int data year;
void Timer0_Init(void);
void Timer0_ISR (void);

#endif