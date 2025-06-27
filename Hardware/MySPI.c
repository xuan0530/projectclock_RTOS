#include "stm32f10x.h"                  // Device header

void MySPI_W_SS(uint8_t  BitValue)//ѡ��ӻ�,дSS
	{
	GPIO_WriteBit(GPIOB,GPIO_Pin_9,(BitAction)BitValue);
}
	void MySPI_W_SCK(uint8_t  BitValue)//ѡ��ӻ�,дSS
	{
	GPIO_WriteBit(GPIOB,GPIO_Pin_7,(BitAction)BitValue);
}
	
void MySPI_W_MOSI(uint8_t  BitValue)//ѡ��ӻ�,дSS
	{
	GPIO_WriteBit(GPIOB,GPIO_Pin_6,(BitAction)BitValue);
}
uint8_t MySPI_R_MISO(void)//ѡ��ӻ�,дSS
	{
	return GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_8);
}
	
void MySPI_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);  
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_6| GPIO_Pin_7; 
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8; 
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	MySPI_W_SS(1);//Ĭ�ϲ�ѡ�дӻ�
	MySPI_W_SCK(0);//0ģʽ
	
}

void MySPI_Start(void)
{
	MySPI_W_SS(0);
}

void MySPI_Stop(void)
{
	MySPI_W_SS(1);
}

//ʹ��ģʽ0������״̬SCK�͵�ƽ����һ�������������ݣ��ڶ����Ƴ�
uint8_t MySPI_SwapByte(uint8_t  ByteSend)
{
	uint8_t i, ByteReceive = 0x00;
	for(i = 0; i < 8; i++)
	{
		MySPI_W_MOSI(ByteSend & (0x80 >> i));
		MySPI_W_SCK(1);
		if(MySPI_R_MISO() ==1) {ByteReceive |= (0x80 >> i);}
		MySPI_W_SCK(0);
		
	}
	return ByteReceive;

}
