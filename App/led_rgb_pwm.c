#include "led_rgb_pwm.h"
#include "main.h"

extern TIM_HandleTypeDef htim1;
extern DMA_HandleTypeDef hdma_tim1_ch1;

// dma发送标志位
uint8_t ws2812_dma_busy_flag = 0;
// dma缓冲区
static uint8_t rgb_dma_buff[LED_DATA_LEN * TOTAL_DMABUFF_LEN] = {0};
// 每个节点使用3个uint8_t类型保存RGB数值
uint8_t rgb_buff[LED_NUMS][3] = {0};

void ws2812_init(void)
{
    // spi_dma_enable(SPI1, SPI_DMA_TRANSMIT);
    // targetlight_color_refresh();

    __HAL_DMA_ENABLE_IT(&hdma_tim1_ch1, DMA_IT_HT);
    __HAL_DMA_ENABLE_IT(&hdma_tim1_ch1, DMA_IT_TC);

    for (int i = 0; i < LED_NUMS; i++)
    {
        ws2812_set_rgb_node(i, 100, 100, 100);
    }

    ws2812_send();
}

void ws2812_write_buff(uint8_t *buff, uint8_t r, uint8_t g, uint8_t b)
{
    uint32_t grb = 0;
    uint8_t i = 0;

    grb = g << 24 | r << 16 | b << 8;
    i = 24;

    while (i--)
    {
        // bit 1:1111 1000
        if (grb & 0x80000000)
        {
            *buff++ = ONE_PULSE; // f8 fc fe
        }
        // bit 0:1100 0000
        else
        {
            *buff++ = ZERO_PULSE; // 80 c0 e0
        }
        grb <<= 1;
    }
}

void ws2812_set_rgb_node(uint16_t index, uint8_t r, uint8_t g, uint8_t b)
{
    rgb_buff[index][0] = r;
    rgb_buff[index][1] = g;
    rgb_buff[index][2] = b;
}

void ws2812_send(void)
{
    // if (ws2812_dma_busy_flag == 1)
    //     return;

    // // 停止dma发送
    // dma_channel_disable(DMA0, DMA_CH4);
    // // 填充复位码（持续低电平)
    // memset(rgb_dma_buff, 0x00, sizeof(rgb_dma_buff));
    // // 设置DMA发送开始位置指针
    // dma_memory_address_config(DMA0, DMA_CH4, (uint32_t)&rgb_dma_buff);
    // dma_transfer_number_config(DMA0, DMA_CH4, (LED_DATA_LEN * TOTAL_DMABUFF_LEN));
    // // dma发送中标志位
    // ws2812_dma_busy_flag = 1; // 使能dma发送
    // dma_channel_enable(DMA0, DMA_CH4);

    // HAL_TIM_PWM_Stop_DMA(&htim1, TIM_CHANNEL_1);
    // memset(rgb_dma_buff, 0x00, sizeof(rgb_dma_buff));
    HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_1, (uint32_t *)rgb_dma_buff, (LED_DATA_LEN * TOTAL_DMABUFF_LEN));
}

void led_rgb_irqhandler(void)
{
    volatile static uint16_t send_nums = 0;

    if (__HAL_DMA_GET_FLAG(&hdma_tim1_ch1, DMA_FLAG_HTIF1_5)) // BANK0
    {
        __HAL_DMA_CLEAR_FLAG(&hdma_tim1_ch1, DMA_FLAG_HTIF1_5);

        for (int i = 0; i < HALF_DMABUFF_LEN; i++)
        {
            ws2812_write_buff(&rgb_dma_buff[LED_DATA_LEN * i], rgb_buff[send_nums + i][0], rgb_buff[send_nums + i][1], rgb_buff[send_nums + i][2]);
        }
    }
    else if (__HAL_DMA_GET_FLAG(&hdma_tim1_ch1, DMA_FLAG_TCIF1_5)) // BANK1
    {
        __HAL_DMA_CLEAR_FLAG(&hdma_tim1_ch1, DMA_FLAG_TCIF1_5);

        for (int i = 0; i < HALF_DMABUFF_LEN; i++)
        {
            ws2812_write_buff(&rgb_dma_buff[LED_DATA_LEN * (i + HALF_DMABUFF_LEN)], rgb_buff[send_nums + i][0], rgb_buff[send_nums + i][1], rgb_buff[send_nums + i][2]);
        }
    }

    if (send_nums > (LED_NUMS))
    {
        send_nums = 0;
        ws2812_dma_busy_flag = 0;
        // dma_channel_disable(&hdma_tim1_ch1, DMA_CH4);
        HAL_TIM_PWM_Stop_DMA(&htim1, TIM_CHANNEL_1);
        return;
    }

    send_nums += HALF_DMABUFF_LEN;
}
