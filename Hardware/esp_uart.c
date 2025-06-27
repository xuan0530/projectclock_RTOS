#include "esp_uart.h"
#include "stm32f10x.h"                  // Device header
#include "stdio.h"
#include "string.h"

// ���ջص�����ָ��
static void (*ESP_UART_ReceiveCallback)(uint8_t) = NULL;

/**
  * ��    ����ESPͨ��UART��ʼ��
  * ��    ������
  * �� �� ֵ����
  */
void ESP_UART_Init(void)
{
    // ʹ��USART2��GPIOAʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);  // ע������Ӧ����APB1������APB2
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    // ����TX����PA2Ϊ�����������ģʽ
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // ����RX����PA3Ϊ��������ģʽ
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // ����USART2����
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStructure.USART_Parity = USART_Parity_No;     // ��У��λ
    USART_InitStructure.USART_StopBits = USART_StopBits_1;  // 1λֹͣλ
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART2, &USART_InitStructure);
    
    // ʹ��USART2�����ж�
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    
    // ����USART2�ж����ȼ�
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // ʹ��USART2
    USART_Cmd(USART2, ENABLE);
}

/**
  * ��    ����ESPͨ��UART����һ���ֽ�����
  * ��    ����Byte Ҫ���͵��ֽ�����
  * �� �� ֵ����
  */
void ESP_UART_SendByte(uint8_t Byte)
{
    USART_SendData(USART2, Byte);
    while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);  // �ȴ��������
}

/**
  * ��    ����ESPͨ��UART����һ����������
  * ��    ����Array Ҫ���͵�����
  * ��    ����Length ���鳤��
  * �� �� ֵ����
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
  * ��    ����ע��ESPͨ��UART���ջص�����
  * ��    ����CallBack �ص�����ָ�룬������ʽΪvoid function(uint8_t)
  * �� �� ֵ����
  */
void ESP_UART_ReceiveRegister(void (*CallBack)(uint8_t))
{
    ESP_UART_ReceiveCallback = CallBack;
}

/**
  * ��    ����USART2�жϷ�����
  * ��    ������
  * �� �� ֵ����
  */
void USART2_IRQHandler(void)
{
    // ����Ƿ�Ϊ�����ж�
    if(USART_GetITStatus(USART2, USART_IT_RXNE) == SET)
    {
        // ��������
        uint8_t RxData = USART_ReceiveData(USART2);
        
        // ���ע���˻ص�����������ûص�����
        if(ESP_UART_ReceiveCallback != NULL)
        {
            ESP_UART_ReceiveCallback(RxData);
        }
        
        // ����жϱ�־
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }
}
