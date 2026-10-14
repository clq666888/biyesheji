#include "Flash.h"
#include "my_adc.h"
#include "stm32f1xx_hal_flash.h"
#include "stm32f1xx_hal_flash_ex.h"
#include <stddef.h>
#include <stdint.h>

#define FLASH_THRESHOLD_MAGIC  0x54485231UL
#define LIGHT_THRESHOLD_MIN    0
#define LIGHT_THRESHOLD_MAX    4095
#define HUMIDITY_THRESHOLD_MIN 0
#define HUMIDITY_THRESHOLD_MAX 100

typedef struct
{
    uint32_t magic;
    int16_t threshold_light;
    int16_t threshold_humidity;
} FlashThresholdConfig_t;

static HAL_StatusTypeDef Flash_ReadBytes(uint8_t *buffer, uint16_t length)
{
    uint16_t i;
    const volatile uint8_t *flash_data;

    if ((buffer == NULL) || (length == 0U) || (length > FLASH_PARAM_PAGE_SIZE))
    {
        return HAL_ERROR;
    }

    flash_data = (const volatile uint8_t *)FLASH_PARAM_PAGE_ADDR;
    for (i = 0U; i < length; i++)
    {
        buffer[i] = flash_data[i];
    }

    return HAL_OK;
}

static HAL_StatusTypeDef Flash_WriteBytes(const uint8_t *buffer, uint16_t length)
{
    HAL_StatusTypeDef status;
    FLASH_EraseInitTypeDef erase;
    uint32_t page_error = 0xFFFFFFFFU;
    uint16_t offset;
    uint16_t half_word;

    if ((buffer == NULL) || (length == 0U) || (length > FLASH_PARAM_PAGE_SIZE))
    {
        return HAL_ERROR;
    }

    status = HAL_FLASH_Unlock();
    if (status != HAL_OK)
    {
        return status;
    }

    erase.TypeErase = FLASH_TYPEERASE_PAGES;
    erase.PageAddress = FLASH_PARAM_PAGE_ADDR;
    erase.NbPages = 1U;

    status = HAL_FLASHEx_Erase(&erase, &page_error);
    if (status == HAL_OK)
    {
        for (offset = 0U; offset < length; offset = (uint16_t)(offset + 2U))
        {
            half_word = buffer[offset];
            if ((uint16_t)(offset + 1U) < length)
            {
                half_word |= (uint16_t)((uint16_t)buffer[offset + 1U] << 8);
            }
            else
            {
                half_word |= 0xFF00U;
            }

            status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,
                                       FLASH_PARAM_PAGE_ADDR + offset,
                                       half_word);
            if (status != HAL_OK)
            {
                break;
            }
        }
    }

    HAL_FLASH_Lock();
    return status;
}

static uint8_t Flash_ThresholdsAreValid(const FlashThresholdConfig_t *config)
{
    return (config->magic == FLASH_THRESHOLD_MAGIC) &&
           (config->threshold_light >= LIGHT_THRESHOLD_MIN) &&
           (config->threshold_light <= LIGHT_THRESHOLD_MAX) &&
           (config->threshold_humidity >= HUMIDITY_THRESHOLD_MIN) &&
           (config->threshold_humidity <= HUMIDITY_THRESHOLD_MAX);
}

HAL_StatusTypeDef Flash_LoadThresholds(void)
{
    FlashThresholdConfig_t config;
    HAL_StatusTypeDef status;

    status = Flash_ReadBytes((uint8_t *)&config, (uint16_t)sizeof(config));
    if (status != HAL_OK)
    {
        return status;
    }

    /* Invalid or erased Flash leaves the existing default values unchanged. */
    if (Flash_ThresholdsAreValid(&config) == 0U)
    {
        return HAL_ERROR;
    }

    threshold_light = config.threshold_light;
    threshold_humidity = config.threshold_humidity;
    return HAL_OK;
}

HAL_StatusTypeDef Flash_SaveThresholds(void)
{
    FlashThresholdConfig_t config;

    if ((threshold_light < LIGHT_THRESHOLD_MIN) ||
        (threshold_light > LIGHT_THRESHOLD_MAX) ||
        (threshold_humidity < HUMIDITY_THRESHOLD_MIN) ||
        (threshold_humidity > HUMIDITY_THRESHOLD_MAX))
    {
        return HAL_ERROR;
    }

    config.magic = FLASH_THRESHOLD_MAGIC;
    config.threshold_light = threshold_light;
    config.threshold_humidity = threshold_humidity;

    return Flash_WriteBytes((const uint8_t *)&config, (uint16_t)sizeof(config));
}
