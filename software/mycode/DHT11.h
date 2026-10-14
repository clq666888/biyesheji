#ifndef __DHT11_H
#define __DHT11_H

#include <stdint.h>

void DHT11_Init(void);
uint8_t DHT11_Read(void);

#endif
