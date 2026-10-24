#include "headfile.h"

uint8_t rx_buf[1024];
uint8_t tx_buf[1024];

/**
 * @brief 初始化蓝牙模块
 * @param huart UART句柄
 */
void Bluetooth_init(void)
{
    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, rx_buf, sizeof(rx_buf));
}

/**
 * @brief 处理接收数据
 * @param size 接收数据长度
 */
static void Bluetooth_Command(uint16_t size)
{

    if (size < 2 || rx_buf[0] != 0xE2)  //检查帧头和数据长度合理性
        return;

    switch (rx_buf[1])
    {
        case 0x01:                      //开窗
            if (size != 2) return;
            servo_state = open_state;
            open();
            break;

        case 0x02:                      //关窗
            if (size != 2) return;
            servo_state = close_state;
            close();
            break;

        case 0x11:                      //手动模式
            if (size != 2) return;
            mode_flag = 1;
            break;

        case 0x12:                      //自动模式
            if (size != 2) return;
            mode_flag = 2;
            break;

        case 0x13:                      //定时模式
            if (size != 2) return;
            mode_flag = 3;
            break;

        case 0x21:                      //设置光照阈值
            if (size != 4) return;
            threshold_light = (int16_t)(rx_buf[2] | (rx_buf[3] << 8));
            Flash_SaveThresholds();
            break;

        case 0x22:                      //设置湿度阈值
            if (size != 4) return;
            threshold_humidity = (int16_t)(rx_buf[2] | (rx_buf[3] << 8));
            Flash_SaveThresholds();
            break;

        case 0x30:                      //时间同步
            if (size != 9) return;
            year = (int16_t)(rx_buf[2] | (rx_buf[3] << 8));
            month = rx_buf[4];
            day = rx_buf[5];
            hour = rx_buf[6];
            minute = rx_buf[7];
            second = rx_buf[8];
            break;

        case 0x40:                      //定时时间设置
            if (size != 6) return;
            timer_start_hour = rx_buf[2];
            timer_start_minute = rx_buf[3];
            timer_end_hour = rx_buf[4];
            timer_end_minute = rx_buf[5];
            break;

        default:
            break;
    }
}

/**
 * @brief 发送设备状态
 */
void Bluetooth_SendDeviceStatus(void)
{
    tx_buf[0] = 0xE2;
    tx_buf[1] = 0x80;
    if(weather_flag == rain)
        tx_buf[2] = 0x00;
    else
        tx_buf[2] = 0x01;
    tx_buf[3] = (uint8_t)(temperature);
    tx_buf[4] = (uint8_t)(humidity);
    tx_buf[5] = (uint8_t)(light);
    tx_buf[6] = (uint8_t)(light >> 8);

    HAL_UART_Transmit_DMA(&huart1, tx_buf, 7);

}

/**
 * @brief 接收数据回调函数
 * @param huart UART句柄
 * @param Size 接收数据长度
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart == &huart1)
    {
        Bluetooth_Command(Size);
        HAL_UARTEx_ReceiveToIdle_DMA(&huart1, rx_buf, sizeof(rx_buf));
    }
}




