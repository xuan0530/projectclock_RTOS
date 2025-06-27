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
	
	
	GPIO_SetBits(GPIOC,GPIO_Pin_13 | GPIO_Pin_14);//需要置高电平
}



void MyI2C_Start(void)
	//起始条件，先拉低SDA,再拉低SCL
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
发送和接收数据
*/

void MyI2C_SendByte(uint8_t Byte)
{
	 uint8_t  i;
	for(i = 0; i < 8; i++)
	{
		MyI2C_W_SDA(Byte & (0x80 >> i));//取出最高位 1000 0000
		MyI2C_W_SCL(1);//释放SCL，从机读数据
		MyI2C_W_SCL(0);
	}
	
}

uint8_t MyI2C_ReceiveByte(void)
{
	uint8_t  i,Byte = 0x00;
	MyI2C_W_SDA(1);//主机释放SDA，让从机写数据
	for(i = 0; i < 8; i++)
	{
		MyI2C_W_SCL(1);
		if(MyI2C_R_SDA() == 1){Byte |= (0x80 >> i);}//取出最高位 1000 0000}
		MyI2C_W_SCL(0);
	}
	return Byte;
}
/*
发送和接收应答
*/
void MyI2C_SendAck(uint8_t AckBit)
{
		MyI2C_W_SDA(AckBit);//主机把应答放入SDA
		MyI2C_W_SCL(1);//释放SCL，从机读取
		MyI2C_W_SCL(0);//进入下一个时序单元
	
	
}

uint8_t MyI2C_ReceiveAck(void)
{
	uint8_t AckBit;
	MyI2C_W_SDA(1);//主机释放SDA，让从机写数据
	MyI2C_W_SCL(1);
	AckBit = MyI2C_R_SDA();//主机读取
	MyI2C_W_SCL(0);
	return AckBit;
}
