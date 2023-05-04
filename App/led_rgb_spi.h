#ifndef __LED_RGB_SPI_H__
#define __LED_RGB_SPI_H__

#include "gd32e10x.h"

#define LED_NUMS (1220)         // led总个数1216+4
#define LED_DATA_LEN (24)       // led长度，单个需要24个字节
#define TOTAL_DMABUFF_LEN (244) // dma双缓冲区总长度（led个数为单位)
#define HALF_DMABUFF_LEN (122)  // dma发送一次的灯珠个数

extern uint8_t g_targetlight_color[3];
extern uint8_t rgb_buff[LED_NUMS][3];

void ws2812_init(void);
void ws2812_write_buff(uint8_t *buff, uint8_t r, uint8_t g, uint8_t b);
void ws2812_set_rgb_node(uint16_t index, uint8_t r, uint8_t g, uint8_t b);
void ws2812_send(void);

#endif
