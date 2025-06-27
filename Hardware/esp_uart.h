
#ifndef __ESP_UART_H
#define __ESP_UART_H

#include "stm32f10x.h"                  // Device header

/* º¯ÊýÉùÃ÷ */
void ESP_UART_Init(void);
void ESP_UART_SendByte(uint8_t Byte);
void ESP_UART_SendArray(uint8_t *Array, uint16_t Length);
void ESP_UART_ReceiveRegister(void (*CallBack)(uint8_t));

#endif
