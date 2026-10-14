#include "headfile.h"

int keynum;
int page = 1;

void main_page()
{
	if(page == 1)
	{
		sprintf((char*)oled_buf,"data %04d-%02d-%02d",year,month,day);				//日期
		OLED_ShowString(1,1,(char*)oled_buf);
		sprintf((char*)oled_buf,"time %02d:%02d:%02d",hour,minute,second);			//时间
		OLED_ShowString(2,1,(char*)oled_buf);
		if(weather_flag)															//天气	
			sprintf((char*)oled_buf,"weather:sun ");
		else 
			sprintf((char*)oled_buf,"weather:rain");
		OLED_ShowString(3,1,(char*)oled_buf);
		if(mode_flag == 1)															//当前模式							
			sprintf((char*)oled_buf,"hand mode");
		else if(mode_flag == 2)
			sprintf((char*)oled_buf,"automatic mode");
		else if(mode_flag == 3)
			sprintf((char*)oled_buf,"timer mode");
		OLED_ShowString(4,1,(char*)oled_buf);
	}
	else
	{
		sprintf((char*)oled_buf,"temperature %d ",temperature);							//温度
		OLED_ShowString(1,1,(char*)oled_buf);
		sprintf((char*)oled_buf,"humidity %d %%",humidity);								//湿度
		OLED_ShowString(2,1,(char*)oled_buf);
		sprintf((char*)oled_buf,"light %d ",light);						//光照
		OLED_ShowString(3,1,(char*)oled_buf);
		if(servo_state == open_state)
			sprintf((char*)oled_buf,"cur state:open ");
		else
			sprintf((char*)oled_buf,"cur state:close");
		OLED_ShowString(4,1,(char*)oled_buf);
	}
}

int menu1()
{	
	uint8_t flag = 1;
	uint8_t last_flag;
	
	sprintf((char*)oled_buf,"Mode Selection");
	OLED_ShowString(1,1,(char*)oled_buf);
	sprintf((char*)oled_buf,"Threshold Set");
	OLED_ShowString(2,1,(char*)oled_buf);
	sprintf((char*)oled_buf,"Date Setting");
	OLED_ShowString(3,1,(char*)oled_buf);
	sprintf((char*)oled_buf,"Timer Setting");
	OLED_ShowString(4,1,(char*)oled_buf);
	OLED_ReverseArea(1,0);
	
	while(1)
	{
		last_flag = flag;
		keynum = KeyGetnum();
		if(keynum == 1)			//返回主页面
		{
			OLED_Clear();
			return 0;
		}
		if(keynum == 2)			//确认按钮
		{
			OLED_Clear();
			return flag;
		}
		else if(keynum == 3)	//上一项按钮
		{
			flag--;
			if(flag < 1)
			{
				flag = 4;
			}
		}
		else if(keynum == 4)	//下一项按钮
		{
			flag++;
			if(flag >4)
			{
				flag = 1;
			}

		}
		
		if(last_flag != flag)
		{
			OLED_Blackback();
			switch(flag)
			{
				case 1:OLED_ReverseArea(1,0);break;
				case 2:OLED_ReverseArea(2,0);break;
				case 3:OLED_ReverseArea(3,0);break;
				case 4:OLED_ReverseArea(4,0);break;
				
			}
		}
	}
	
}

void menu2_mode()
{
	uint8_t flag = 1;
	uint8_t last_flag;
	
	sprintf((char*)oled_buf,"Hand Mode");
	OLED_ShowString(1,1,(char*)oled_buf);
	sprintf((char*)oled_buf,"Automatic Mode");
	OLED_ShowString(2,1,(char*)oled_buf);
	sprintf((char*)oled_buf,"Timer Mode");
	OLED_ShowString(3,1,(char*)oled_buf);

	OLED_ReverseArea(1,0);
	
	while(1)
	{
		last_flag = flag;
		keynum = KeyGetnum();
		if(keynum == 2)			//确认按钮
		{
			OLED_Clear();
			mode_flag = flag;
			break;
		}
		else if(keynum == 3)	//上一项按钮
		{
			flag--;
			if(flag < 1)
			{
				flag = 3;
			}
		}
		else if(keynum == 4)	//下一项按钮
		{
			flag++;
			if(flag >3)
			{
				flag = 1;
			}

		}
		
		if(last_flag != flag)
		{
			OLED_Blackback();
			switch(flag)
			{
				case 1:OLED_ReverseArea(1,0);break;
				case 2:OLED_ReverseArea(2,0);break;
				case 3:OLED_ReverseArea(3,0);break;			
			}
		}
	}
}

int menu2_threshold_setting()
{
	uint8_t flag = 1;
	uint8_t last_flag;
	
	sprintf((char*)oled_buf,"light");
	OLED_ShowString(1,1,(char*)oled_buf);
	sprintf((char*)oled_buf,"humidity");
	OLED_ShowString(2,1,(char*)oled_buf);
	sprintf((char*)oled_buf,"<--");
	OLED_ShowString(3,1,(char*)oled_buf);

	OLED_ReverseArea(1,0);
	
	while(1)
	{
		last_flag = flag;
		keynum = KeyGetnum();
		if(keynum == 2)			//确认按钮
		{
			OLED_Clear();
			return flag;
		}
		else if(keynum == 3)	//上一项按钮
		{
			flag--;
			if(flag < 1)
			{
				flag = 3;
			}
		}
		else if(keynum == 4)	//下一项按钮
		{
			flag++;
			if(flag >3)
			{
				flag = 1;
			}
		}
		
		if(last_flag != flag)
		{
			OLED_Blackback();
			switch(flag)
			{
				case 1:OLED_ReverseArea(1,0);break;
				case 2:OLED_ReverseArea(2,0);break;
				case 3:OLED_ReverseArea(3,0);break;			
			}
		}
	}
}

void menu3_threshold(int flag)	//flag为1表示更改照明阈值，为2更改湿度阈值,为3返回主页面
{
	uint8_t rate = 1;
	
	while(1)
	{
		if(flag == 3)
			break;

		sprintf((char*)oled_buf,"rate:*%03d",rate);
		OLED_ShowString(1,1,(char*)oled_buf);
		sprintf((char*)oled_buf,"current value:");
		OLED_ShowString(2,1,(char*)oled_buf);
		if(flag == 1)	  sprintf((char*)oled_buf,"      %04d       ",threshold_light);
		else if(flag == 2)sprintf((char*)oled_buf,"      %03d       ",threshold_humidity);
		OLED_ShowString(3,1,(char*)oled_buf);
		
		keynum = KeyGetnum();
		if(keynum == 2)			//确认按钮
		{
			OLED_Clear();
			Flash_SaveThresholds();
			break;
		}
		if(keynum == 1)			//切换倍率
		{
			if(rate == 1)rate=10;
			else if(rate == 10)rate=100;
			else if(rate == 100)rate=1;
		}
		else if(keynum == 3)	//减
		{
			if(flag == 1)		threshold_light 	 -= rate;
			else if(flag == 2)	threshold_humidity	 -= rate;
			if(threshold_light<0)	threshold_light   = 0;
			if(threshold_humidity<0)threshold_humidity= 0;
		}
		else if(keynum == 4)	//加
		{
			if(flag == 1)		threshold_light 	 += rate;
			else if(flag == 2)	threshold_humidity	 += rate;
			if(threshold_light>4095)	threshold_light = 4095;
			if(threshold_humidity>100)threshold_humidity= 100;
		}
	}
}

void menu2_date_setting()
{
	uint8_t reverse_pos = 1;
	
	sprintf((char*)oled_buf,"date");
	OLED_ShowString(1,1,(char*)oled_buf);

	sprintf((char*)oled_buf,"time");
	OLED_ShowString(3,1,(char*)oled_buf);

	OLED_ReverseArea(2,1);
	OLED_ReverseArea(2,2);
	OLED_ReverseArea(2,3);
	OLED_ReverseArea(2,4);
	while(1)
	{
		sprintf((char*)oled_buf,"%04d-%02d-%02d",year,month,day);
		OLED_ShowString(2,1,(char*)oled_buf);

		sprintf((char*)oled_buf,"%02d:%02d:%02d",hour,minute,second);
		OLED_ShowString(4,1,(char*)oled_buf);

		keynum = KeyGetnum();
		
		if(keynum == 1)			//切换按钮
		{
			OLED_Blackback();
			if(reverse_pos == 1)
			{
				reverse_pos = 2;
				OLED_ReverseArea(2,6);
				OLED_ReverseArea(2,7);
			}
			else if(reverse_pos == 2)
			{
				reverse_pos = 3;
				OLED_ReverseArea(2,9);
				OLED_ReverseArea(2,10);
			}
			else if(reverse_pos == 3)
			{
				reverse_pos = 4;
				OLED_ReverseArea(4,1);
				OLED_ReverseArea(4,2);
			}
			else if(reverse_pos == 4)
			{
				reverse_pos = 5;
				OLED_ReverseArea(4,4);
				OLED_ReverseArea(4,5);
			}
			else if(reverse_pos == 5)
			{
				reverse_pos = 6;
				OLED_ReverseArea(4,7);
				OLED_ReverseArea(4,8);
			}
			else if(reverse_pos == 6)
			{
				reverse_pos = 1;
				OLED_ReverseArea(2,1);
				OLED_ReverseArea(2,2);
				OLED_ReverseArea(2,3);
				OLED_ReverseArea(2,4);			
			}
		}

		if(keynum == 2)			//确认按钮
		{
			OLED_Clear();
			break;
		}
		else if(keynum == 3)	//减
		{
			switch (reverse_pos)
			{
				case 1:year--;break;	
				case 2:month--;break;
				case 3:day--;break;
				case 4:hour--;break;
				case 5:minute--;break;
				case 6:second--;break;
			}
			if(year < 2020)year = 2020;
			if(month < 1)month = 1;
			if(day < 1)day = 1;
			if(hour < 0)hour = 0;
			if(minute < 0)minute = 0;
			if(second < 0)second = 0;	
		}
		else if(keynum == 4)	//加
		{
			switch (reverse_pos)
			{
				case 1:year++;break;	
				case 2:month++;break;
				case 3:day++;break;
				case 4:hour++;break;
				case 5:minute++;break;
				case 6:second++;break;
			}
			if(month > 12)month = 12;
			if(day > 31)day = 31;
			if(hour > 23)hour = 23;
			if(minute > 59)minute = 59;
			if(second > 59)second = 59;	
		}	
		
	}
}

void menu2_timer_setting()
{
	uint8_t reverse_pos = 1;
	
	sprintf((char*)oled_buf,"start time");
	OLED_ShowString(1,1,(char*)oled_buf);

	sprintf((char*)oled_buf,"end time");
	OLED_ShowString(3,1,(char*)oled_buf);

	OLED_ReverseArea(2,1);
	OLED_ReverseArea(2,2);

	while(1)
	{
		sprintf((char*)oled_buf,"%02d:%02d",timer_start_hour, timer_start_minute);
		OLED_ShowString(2,1,(char*)oled_buf);
		sprintf((char*)oled_buf,"%02d:%02d",timer_end_hour, timer_end_minute);
		OLED_ShowString(4,1,(char*)oled_buf);	

		keynum = KeyGetnum();
		
		if(keynum == 1)			//切换按钮
		{
			OLED_Blackback();
			if(reverse_pos == 1)
			{
				reverse_pos = 2;
				OLED_ReverseArea(2,4);
				OLED_ReverseArea(2,5);
			}
			else if(reverse_pos == 2)
			{
				reverse_pos = 3;
				OLED_ReverseArea(4,1);
				OLED_ReverseArea(4,2);
			}
			else if(reverse_pos == 3)
			{
				reverse_pos = 4;
				OLED_ReverseArea(4,4);
				OLED_ReverseArea(4,5);
			}
			else if(reverse_pos == 4)
			{
				reverse_pos = 1;
				OLED_ReverseArea(2,1);
				OLED_ReverseArea(2,2);
			}
		}

		if(keynum == 2)			//确认按钮
		{
			OLED_Clear();
			break;
		}
		else if(keynum == 3)	//减
		{
			switch (reverse_pos)
			{
				case 1:timer_start_hour--;break;
				case 2:timer_start_minute--;break;
				case 3:timer_end_hour--;break;
				case 4:timer_end_minute--;break;
			}
			if(timer_start_hour < 0)timer_start_hour = 0;
			if(timer_start_minute < 0)timer_start_minute = 0;
			if(timer_end_hour < 0)timer_end_hour = 0;
			if(timer_end_minute < 0)timer_end_minute = 0;	
		}
		else if(keynum == 4)	//加
		{
			switch (reverse_pos)
			{
				case 1:timer_start_hour++;break;
				case 2:timer_start_minute++;break;
				case 3:timer_end_hour++;break;
				case 4:timer_end_minute++;break;
			}
			if(timer_start_hour > 23)timer_start_hour = 23;
			if(timer_start_minute > 59)timer_start_minute = 59;	
			if(timer_end_hour > 23)timer_end_hour = 23;
			if(timer_end_minute > 59)timer_end_minute = 59;	
		}		
	}
}

