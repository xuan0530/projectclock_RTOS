#include "stm32f10x.h"                  // Device header
#include "Delay.h"

void MyI2C_W_SCL(uint8_t BitValue)
{
	GPIO_WriteBit(GPIOC,GPIO_Pin_13,(BitAction)BitValue);
	Delay_us(10);
}
	
void MyI2C_W_SDA(uint8_t BitValue)
{
	GPIO_WriteBit(GPIOC,GPIO_Pin_14,(BitAction)BitValue);
	Delay_us(10);
}

uint8_t  MyI2C_R_SDA(void)
{
		uint8_t BitValue;
	BitValue = GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_14);
	Delay_us(10);
	return BitValue;
}

void MyI2C_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);  
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14; 
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOC,&GPIO_InitStructure);
	
	
	GPIO_SetBits(GPIOC,GPIO_Pin_13 | GPIO_Pin_14);//��Ҫ�øߵ�ƽ
}



void MyI2C_Start(void)
	//��ʼ������������SDA,������SCL
{
	MyI2C_W_SDA(1);
	MyI2C_W_SCL(1);
	
	MyI2C_W_SDA(0);
	MyI2C_W_SCL(0);
}

void MyI2C_Stop(void)
{
		MyI2C_W_SDA(0);
		MyI2C_W_SCL(1);
		MyI2C_W_SDA(1);
}
/*
���ͺͽ�������
*/

void MyI2C_SendByte(uint8_t Byte)
{
	 uint8_t  i;
	for(i = 0; i < 8; i++)
	{
		MyI2C_W_SDA(Byte & (0x80 >> i));//ȡ�����λ 1000 0000
		MyI2C_W_SCL(1);//�ͷ�SCL���ӻ�������
		MyI2C_W_SCL(0);
	}
	
}

uint8_t MyI2C_ReceiveByte(void)
{
	uint8_t  i,Byte = 0x00;
	MyI2C_W_SDA(1);//�����ͷ�SDA���ôӻ�д����
	for(i = 0; i < 8; i++)
	{
		MyI2C_W_SCL(1);
		if(MyI2C_R_SDA() == 1){Byte |= (0x80 >> i);}//ȡ�����λ 1000 0000}
		MyI2C_W_SCL(0);
	}
	return Byte;
}
/*
���ͺͽ���Ӧ��
*/
void MyI2C_SendAck(uint8_t AckBit)
{
		MyI2C_W_SDA(AckBit);//������Ӧ�����SDA
		MyI2C_W_SCL(1);//�ͷ�SCL���ӻ���ȡ
		MyI2C_W_SCL(0);//������һ��ʱ��Ԫ
	
	
}

uint8_t MyI2C_ReceiveAck(void)
{
	uint8_t AckBit;
	MyI2C_W_SDA(1);//�����ͷ�SDA���ôӻ�д����
	MyI2C_W_SCL(1);
	AckBit = MyI2C_R_SDA();//������ȡ
	MyI2C_W_SCL(0);
	return AckBit;
}
