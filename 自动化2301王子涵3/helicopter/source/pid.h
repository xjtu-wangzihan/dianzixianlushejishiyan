#ifndef PID_H
#define PID_H

#include "c8051f020.h"

typedef struct{
	int setpoint;	
	long sumerror;			
	float P;			
	float I;			
	float D;	
	int lasterror;	
	int preverror;
	int result;
}PID;

void PID_contrl(PID* ptr,int nowpoint);
void PID_init(PID* ptr);
float PIDcontroller(float error,float error_sum,float error_pre ,float kp,float ki,float kd);
int constrain(int value, int min, int max);

#endif