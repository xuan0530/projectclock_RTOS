#include "stm32f10x.h"                  // Device header


void PWM_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; 
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	TIM_InternalClockConfig(TIM2);//内部时钟,选择时钟源
	
	
	
	TIM_OCInitTypeDef TIM_OCInitStructure;//带N、idle->高级
	TIM_OCStructInit(&TIM_OCInitStructure);//赋初始值
	TIM_OCInitStructure.TIM_OCMode=TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OCPolarity=TIM_OCPolarity_High;//极性
	TIM_OCInitStructure.TIM_OutputState=TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse=0;// CCR 16位范围
	
	TIM_OC1Init(TIM2,&TIM_OCInitStructure);
	
//	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);//更新中断到NVIC
//	
//	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
//	
//	NVIC_InitTypeDef NVIC_InitStructure;
//	NVIC_InitStructure.NVIC_IRQChannel=TIM2_IRQn;
//	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE ;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority=1;
//	NVIC_Init(&NVIC_InitStructure);
	
	TIM_Cmd(TIM2,ENABLE);
	
	
	
	
}

void PWM_SetCompare1(uint16_t Compare)//设置CCR的值
{
	TIM_SetCompare1(TIM2,Compare);
}

