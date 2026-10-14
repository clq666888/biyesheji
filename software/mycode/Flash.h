#ifndef __FLASH_H
#define __FLASH_H

#include "stm32f1xx_hal.h"

/* Reserved final 1 KB page for user parameters on the configured 64 KB part. */
#define FLASH_PARAM_PAGE_ADDR  0x0800FC00UL
#define FLASH_PARAM_PAGE_SIZE  1024U

/* Load/save the light and humidity thresholds without caller-supplied data. */
HAL_StatusTypeDef Flash_LoadThresholds(void);
HAL_StatusTypeDef Flash_SaveThresholds(void);

#endif
