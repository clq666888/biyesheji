#ifndef __HEADFILE_H
#define __HEADFILE_H

#include "stdio.h"
#include "string.h"
#include "stdlib.h"

#include "main.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

#include "servo.h"
#include "OLED.h"
#include "my_adc.h"
#include "DHT11.h"
#include "Flash.h"
#include "key.h"
#include "menu.h"
#include "mode.h"

#include "demo.h"

extern int16_t year,month,day,second,minute,hour;
extern uint8_t oled_buf[1024];

#endif
