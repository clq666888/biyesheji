#include "headfile.h"

void Servo_Init()
{
	HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2);	//启动PWM2通道2
}

void SetAngle(float InputAngle)
{
	uint16_t  Ret=(uint16_t )(InputAngle/180 * 2000 +500 + 0.5f);
	__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2,Ret);
}

void open()
{
	SetAngle(0.0f);
}

void close()
{
	SetAngle(180.0f);
}
