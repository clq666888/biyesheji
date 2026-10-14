#include "headfile.h"

int KeyGetnum()
{
	uint8_t num=0;
	if (HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_4)==0)//端口默认高电平，按键按下变低电平
	{
		HAL_Delay(20);				//消抖
		while(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_4)==0);//如果一直按，则会卡在这
		HAL_Delay(20);
		num=1;
	}

	if (HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_5)==0)//端口默认高电平，按键按下变低电平
	{
		HAL_Delay(20);				//消抖
		while(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_5)==0);//如果一直按，则会卡在这
		HAL_Delay(20);
		num=2;
	}	
	
    if (HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_6)==0)//端口默认高电平，按键按下变低电平
	{
		HAL_Delay(20);				//消抖
		while(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_6)==0);//如果一直按，则会卡在这
		HAL_Delay(20);
		num=3;
	}	

    if (HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_7)==0)//端口默认高电平，按键按下变低电平
	{
		HAL_Delay(20);				//消抖
		while(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_7)==0);//如果一直按，则会卡在这
		HAL_Delay(20);
		num=4;
	}
		
	return num;

}



