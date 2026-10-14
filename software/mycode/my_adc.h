#ifndef __MY_ADC_H
#define __MY_ADC_H

extern int light,temperature,humidity;
extern uint16_t AD_Value[2];
extern int weather_flag;	//0为雨天，1为晴天
extern int16_t threshold_light;			//光照阈值（光越暗值越大）
extern int16_t threshold_humidity;		//湿度阈值

void adc_init(void);
void Photosensitive_adc(void);


#endif
