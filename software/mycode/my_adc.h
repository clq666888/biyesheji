#ifndef __MY_ADC_H
#define __MY_ADC_H

typedef enum         //天气状态
{
    rain,
    sun,
}weather_status;

extern int light,temperature,humidity;
extern uint16_t AD_Value[2];
extern weather_status weather_flag;	//0雨天，1晴天
extern int16_t threshold_light;			//光照阈值
extern int16_t threshold_humidity;		//湿度阈值

void adc_init(void);
void Photosensitive_adc(void);


#endif
