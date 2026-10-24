#include "headfile.h"

extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

uint16_t AD_Value[2];
weather_status weather_flag;	//0为雨天，1为晴天
int light,temperature,humidity;         //光照强度，温度，湿度
int16_t threshold_light = 350;			//光照阈值（光越暗值越大）
int16_t threshold_humidity = 60;		//湿度阈值
int16_t threshold_rain = 3000;			//雨滴检测阈值

void adc_init()
{
	HAL_ADCEx_Calibration_Start(&hadc1);
}

void Photosensitive_adc()
{
	HAL_StatusTypeDef hhStatue=HAL_ADC_Start_DMA(&hadc1,(uint32_t*) AD_Value, 2);
	if(hhStatue!=HAL_OK)
	{
		AD_Value[0]=0;//光照强度
		AD_Value[1]=0;//雨滴检测
	}
	while(hdma_adc1.State!=HAL_DMA_STATE_READY);
	
	if(AD_Value[1]>threshold_rain)		//判断是否下雨
	{
		weather_flag = sun;
	}
	else
	{
		weather_flag = rain;
	}
	light = AD_Value[0];				//更新光照强度
	
	HAL_ADC_Stop_DMA(&hadc1);
}
