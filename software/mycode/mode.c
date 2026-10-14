#include "headfile.h"

int mode_flag = 1;		//1表示手动模式，2为自动模式，3为定时模式
int timer_start_hour,timer_start_minute,timer_end_hour,timer_end_minute;
enum mode_status servo_state = close_state;	//定义当前舵机的开关状态

void hand_mode()
{
	if (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_12)==0)
	{
		HAL_Delay(20);				
		while(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_12)==0);
		if(servo_state == close_state)
		{
			servo_state = open_state;
			open();
		}
		else 
		{
			servo_state = close_state;
			close();
		}
		
		HAL_Delay(20);
	}
		
}

void automatic_mode()
{
	if(weather_flag == 1 && light > threshold_light && humidity < threshold_humidity)
		{
			servo_state = open_state;
			open();
		}
		else
		{
			servo_state = close_state;
			close();
		}	
}

void timer_mode()
{
	int now = hour * 60 + minute;
	int start = timer_start_hour * 60 + timer_start_minute;
	int end = timer_end_hour * 60 + timer_end_minute;

	if (now >= start && now < end)
	{
		servo_state = open_state;
		open();
	}
	else
	{
		servo_state = close_state;			
		close();
	}		
}

void mode()
{
	if(mode_flag == 1)
		hand_mode();
	
	else if(mode_flag == 2)
		automatic_mode();
	
	else if(mode_flag == 3)
		timer_mode();
}



