#ifndef __DELAY_H
#define __DELAY_H

#include "stm32f10x.h"

void Delay_us(uint32_t xus);
void Delay_ms(uint32_t xms);
void Delay_s(uint32_t xs);

// 新增的FreeRTOS专用延时函数
void Delay_ms_RTOS(uint32_t xms);

#endif


