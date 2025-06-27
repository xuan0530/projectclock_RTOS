#include "esp_uart.h"
#include "stm32f10x.h"                  // Device header
#include "stdio.h"
#include "string.h"

// 接收回调函数指针
static void (*ESP_UART_ReceiveCallback)(uint8_t) = NULL;

/**
  * 函    数：ESP通信UART初始化
  * 参    数：无
  * 返 回 值：无
  */
void ESP_UART_Init(void)
{
    // 使能USART2和GPIOA时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);  // 注意这里应该是APB1而不是APB2
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    // 配置TX引脚PA2为复用推挽输出模式
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 配置RX引脚PA3为上拉输入模式
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 配置USART2参数
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStructure.USART_Parity = USART_Parity_No;     // 无校验位
    USART_InitStructure.USART_StopBits = USART_StopBits_1;  // 1位停止位
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART2, &USART_InitStructure);
    
    // 使能USART2接收中断
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    
    // 配置USART2中断优先级
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // 使能USART2
    USART_Cmd(USART2, ENABLE);
}

/**
  * 函    数：ESP通信UART发送一个字节数据
  * 参    数：Byte 要发送的字节数据
  * 返 回 值：无
  */
void ESP_UART_SendByte(uint8_t Byte)
{
    USART_SendData(USART2, Byte);
    while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);  // 等待发送完成
}

/**
  * 函    数：ESP通信UART发送一个数组数据
  * 参    数：Array 要发送的数组
  * 参    数：Length 数组长度
  * 返 回 值：无
  */
void ESP_UART_SendArray(uint8_t *Array, uint16_t Length)
{
    uint16_t i;
    for(i = 0; i < Length; i++)
    {
        ESP_UART_SendByte(Array[i]);
    }
}

/**
  * 函    数：注册ESP通信UART接收回调函数
  * 参    数：CallBack 回调函数指针，函数形式为void function(uint8_t)
  * 返 回 值：无
  */
void ESP_UART_ReceiveRegister(void (*CallBack)(uint8_t))
{
    ESP_UART_ReceiveCallback = CallBack;
}

/**
  * 函    数：USART2中断服务函数
  * 参    数：无
  * 返 回 值：无
  */
void USART2_IRQHandler(void)
{
    // 检测是否为接收中断
    if(USART_GetITStatus(USART2, USART_IT_RXNE) == SET)
    {
        // 接收数据
        uint8_t RxData = USART_ReceiveData(USART2);
        
        // 如果注册了回调函数，则调用回调函数
        if(ESP_UART_ReceiveCallback != NULL)
        {
            ESP_UART_ReceiveCallback(RxData);
        }
        
        // 清除中断标志
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }
}
