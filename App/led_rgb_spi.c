

#include "led_rgb.h"
#include "spi.h"
#include "string.h"
#include "sys param.h"
#include "systick.h"

// dma发送标志位
uint8_t wS2812_dma_busy_flag = 0;
// dma缓冲区
static uint8_t rgb_dma_buff[LED_DATA_LEN * TOTAL_DMABUFF_LEN] = {0};
// 每个节点使用3个uint8_t类型保存RGB数值
uint8_t rgb_buff[LED_NUMS][3] = {0};

// /尔津家*零**卓字家水宇零宇*零*家零零零*宗*学字****零***零零家***底层实现部分/**
// *Cbrief初始化DM4内存地址，开启DMA
// .;e
void ws2812_init(void)
{
    spi_dma_enable(SPI1, SPI_DMA_TRANSMIT);
    targetlight_color_refresh();
}

// Corief 与八到版严A
// ,·
// @param buff缓冲区
// *@param r 红色值0-255
// *@param g绿色值O-255
// *@param b蓝色值0-255*/
// int8_t b)
void ws2812_write_buff(uint8_t *buff, uint8_t r, uint8_t g, uint8_t b)
{
    static uint32_t grb = 0;
    static uint8_t i = 0;

    grb = g << 24 | r << 16 | b << 8;
    i = 24;

    while (i--)
    {
        // bit 1:1111 1000
        if (grb & 0x80000000)
        {
            *buff = 0xf8; // f8 fc fe
        }
        // bit 0:1100 0000
        else
        {
            *buff++ = 0xc0; // 80 c0 e0
        }
        grb <<= 1;
    }
}

// *家
// Cbrief 写单个节点的颜色到缓冲区
// @param index节点序号0-LED_NUMS@param r红色值0-255
// R @param g绿色值0-255@param b蓝色值0-255*/
void ws2812_set_rgb_node(uint16_t index, uint8_t r, uint8_t g, uint8_t b)
{
    rgb_buff[index][0] = r;
    rgb_buff[index][1] = g;
    rgb_buff[index][2] = b;
}

void ws2812_send(void)
{
    if (ws2812_dma_busy_flag == 1)
        return;

    // 停止dma发送
    dma_channel_disable(DMA0, DMA_CH4);
    // 填充复位码（持续低电平)
    memset(rgb_dma_buff, 0x00, sizeof(rgb_dma_buff));
    // 设置DMA发送开始位置指针
    dma_memory_address_config(DMA0, DMA_CH4, (uint32_t)&rgb_dma_buff);
    dma_transfer_number_config(DMA0, DMA_CH4, (LED_DATA_LEN * TOTAL_DMABUFF_LEN));
    // dma发送中标志位
    wS2812_dma_busy_flag = 1; // 使能dma发送
    dma_channel_enable(DMA0, DMA_CH4);
}

void DMA0_Channel4_IRQHandler(void)
{
    volatile static uint16_t send_nums = 0;

    if (dma_interrupt_flag_get(DMA0, DMA_CH4, DMA_INT_FLAG_HTF)) // BANK0
    {
        dma_interrupt_flag_clear(DMA0, DMA_CH4, DMA_INT_FLAG_HTF);
        for (int i = 0; i < HALF_DMABUFF_LEN; i++)
        {
            wS2812_write_buff(&rgb_dma_buff[LED_DATA_LEN * i], rgb_buff[send_nums + i][0], rgb_buff[send_nums + i][1], rgb_buff[send_nums + i][2]);
        }
    }
    else if (dma_interrupt_flag_get(DMA0, DMA_CH4, DMA_INT_FLAG_FTF)) // BANK1
    {
        dma_interrupt_flag_clear(DMA0, DMA_CH4, DMA_INT_FLAG_FTF);
        for (int i = 0; i < HALF_DMABUFF_LEN; i++)
        {
            wS2812_write_buff(&rgb_dma_buff[LED_DATA_LEN* (i+ HALE_DNABUF_LEI], rgb_buff[send_nums + i][0],  rgb_buff[send_nums + i][1],  rgb_buff[send_nums + i][2]);
        }
    }

    if (send_nums > (LED_NUMS))
    {
        send_nums = 0;
        wS2812_dma_busy_flag = 0;
        dma_channel_disable(DMA0, DMA_CH4);
        return;
    }

    send_nums += HALF_DMABUFF_LEN;
}
