#include "headfile.h"


void demo()
{
	sprintf((char*)oled_buf,"AD0:%d",AD_Value[0]);
	OLED_ShowString(1,1,(char*)oled_buf);
	sprintf((char*)oled_buf,"AD1:%d",AD_Value[1]);
	OLED_ShowString(2,1,(char*)oled_buf);
	sprintf((char*)oled_buf,"%d:%d:%d",hour,minute,second);
	OLED_ShowString(3,1,(char*)oled_buf);
	
	    HAL_Delay(20);
	//	HAL_UART_Transmit(&huart1,"hello\n",7,1000);
	//  HAL_UART_Transmit_DMA(&huart1, "hello\n", 6);
//	if(second%2==0)open();
//	else  close();  

}
