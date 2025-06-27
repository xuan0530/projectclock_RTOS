#include "stm32f10x.h"                  // Device header

uint16_t MyDMA_Size;


void MyDMA_Init(uint32_t AddrA,uint32_t AddrB,uint16_t Size)
{
	MyDMA_Size=Size; 
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);
	
	DMA_InitTypeDef DMA_InitStructure;
	
	DMA_InitStructure.DMA_PeripheralBaseAddr=AddrA;//外设站点的起始地址、数据宽度、是否自增
	DMA_InitStructure.DMA_PeripheralDataSize=DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_PeripheralInc=DMA_PeripheralInc_Enable;
	DMA_InitStructure.DMA_MemoryBaseAddr=AddrB;//寄存器站点的起始地址、数据宽度、是否自增
	DMA_InitStructure.DMA_MemoryDataSize=DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_MemoryInc=DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_DIR=DMA_DIR_PeripheralSRC;//传输方向,外设到存储器
	DMA_InitStructure.DMA_BufferSize=Size;//缓存区大小，传输计数器,传输几次
	DMA_InitStructure.DMA_Mode=DMA_Mode_Normal;//是否使用自动重装
	DMA_InitStructure.DMA_M2M=DMA_M2M_Enable ;//硬件触发、软件(存储器到存储器，enable
	DMA_InitStructure.DMA_Priority=DMA_Priority_Medium ;
	
	DMA_Init(DMA1_Channel1,&DMA_InitStructure);
	
	DMA_Cmd(DMA1_Channel1,ENABLE);
}

void MyDMA_Transfer(void)
{
		DMA_Cmd(DMA1_Channel1,DISABLE);
		DMA_SetCurrDataCounter(DMA1_Channel1,MyDMA_Size);
	  DMA_Cmd(DMA1_Channel1,ENABLE);
	
	  while(DMA_GetFlagStatus(DMA1_FLAG_TC1) == RESET);
		DMA_ClearFlag(DMA1_FLAG_TC1);
}
