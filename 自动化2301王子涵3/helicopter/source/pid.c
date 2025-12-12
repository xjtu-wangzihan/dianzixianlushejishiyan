#include "pid.h"

//PID控制实现函数
float PIDcontroller(float error,float error_sum,float error_pre ,float kp,float ki,float kd)
{                  
	float output; 
  output = kp*error + ki * error_sum + kd * (error - error_pre);   //计算最终的输出
	return output;
}

int constrain(int value, int min, int max) 
{
    if (value < min) 
			return min;
    if (value > max) 
			return max;
    return value;
}
void PID_init(PID* ptr)
{
	//初始化结构体ptr的值
	ptr->setpoint = 3000;
	ptr->sumerror = 0; 
	ptr->lasterror = 0;
	ptr->preverror = 0;	
	ptr->P = 3.200;                
	ptr->I = 0.050;                
	ptr->D = 0.030;   
	ptr->result = 0;                                      			
}

// PID控制函数
void PID_contrl(PID* ptr,int nowpoint)
{
	int tmp_error;
	tmp_error = ptr->setpoint - nowpoint;	
	ptr->sumerror+=tmp_error;
	//ptr->sumerror = constrain(ptr->sumerror, -1000, 7000);
	ptr->result=PIDcontroller(tmp_error,ptr->sumerror,ptr->lasterror,ptr->P,ptr->I,ptr->D);
	ptr->preverror=ptr->lasterror;
	ptr->lasterror = tmp_error;		
}


