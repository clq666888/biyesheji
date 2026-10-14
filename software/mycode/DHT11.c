#include "headfile.h"

/* DHT11 single-wire driver state. */
static uint8_t dht11_initialized = 0U;
static uint8_t dht11_has_read = 0U;
static uint32_t dht11_last_read_tick = 0U;

static void DHT11_DelayUs(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t cycles = (SystemCoreClock / 1000000U) * us;

    while ((uint32_t)(DWT->CYCCNT - start) < cycles)
    {
    }
}

static uint8_t DHT11_WaitPin(GPIO_PinState state, uint32_t timeout_us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t timeout_cycles = (SystemCoreClock / 1000000U) * timeout_us;

    while (HAL_GPIO_ReadPin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin) != state)
    {
        if ((uint32_t)(DWT->CYCCNT - start) >= timeout_cycles)
        {
            return 0U;
        }
    }
    return 1U;
}

void DHT11_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    /* In open-drain mode, SET releases the bus to the pull-up resistor. */
    HAL_GPIO_WritePin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin, GPIO_PIN_SET);
    HAL_Delay(1000U); /* Sensor power-up stabilization from the product manual. */
    dht11_has_read = 0U;
    dht11_initialized = 1U;
}

/* Returns 1 on a valid frame; updates humidity and temperature only on success. */
uint8_t DHT11_Read(void)
{
    uint8_t data[5] = {0U, 0U, 0U, 0U, 0U};
    uint8_t byte_index;
    uint8_t bit_index;
    uint8_t success = 0U;
    uint8_t checksum;
    uint32_t primask;

    if (dht11_initialized == 0U)
    {
        DHT11_Init();
    }

    /* The attached product manual specifies a minimum 1-second interval. */
    if ((dht11_has_read != 0U) &&
        ((uint32_t)(HAL_GetTick() - dht11_last_read_tick) < 1000U))
    {
        return 0U;
    }
    dht11_last_read_tick = HAL_GetTick();
    dht11_has_read = 1U;

    /* Host start signal: hold low for at least 18 ms, then release the bus. */
    HAL_GPIO_WritePin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin, GPIO_PIN_RESET);
    HAL_Delay(20U);
    primask = __get_PRIMASK();
    __disable_irq();
    HAL_GPIO_WritePin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin, GPIO_PIN_SET);
    DHT11_DelayUs(30U);

    /* Sensor response: low, high, then low for the first data-bit slot. */
    if (DHT11_WaitPin(GPIO_PIN_RESET, 100U) == 0U ||
        DHT11_WaitPin(GPIO_PIN_SET, 100U) == 0U ||
        DHT11_WaitPin(GPIO_PIN_RESET, 100U) == 0U)
    {
        goto dht11_done;
    }

    /* Each bit starts with about 50 us low; a 40 us sample distinguishes 0/1. */
    for (byte_index = 0U; byte_index < 5U; byte_index++)
    {
        for (bit_index = 0U; bit_index < 8U; bit_index++)
        {
            if (DHT11_WaitPin(GPIO_PIN_SET, 100U) == 0U)
            {
                goto dht11_done;
            }

            data[byte_index] <<= 1;
            DHT11_DelayUs(40U);
            if (HAL_GPIO_ReadPin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin) == GPIO_PIN_SET)
            {
                data[byte_index] |= 1U;
            }

            if (DHT11_WaitPin(GPIO_PIN_RESET, 100U) == 0U)
            {
                goto dht11_done;
            }
        }
    }
    success = 1U;

dht11_done:
    /* Release the bus and restore the prior interrupt state on every path. */
    HAL_GPIO_WritePin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin, GPIO_PIN_SET);
    if (primask == 0U)
    {
        __enable_irq();
    }

    if (success == 0U)
    {
        return 0U;
    }

    checksum = (uint8_t)(data[0] + data[1] + data[2] + data[3]);
    if (checksum != data[4])
    {
        return 0U;
    }

    /* The manual states the decimal bytes are currently zero. */
    humidity = data[0];
    temperature = data[2];
    return 1U;
}
