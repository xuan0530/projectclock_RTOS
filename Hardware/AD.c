#include "stm32f10x.h"                  // Device header
#include "stdio.h"
#include "string.h"
#include <math.h>

// ����ԭ��ͼ��������
#define R_FIXED     10000.0f    // ��ѹ����10k����ԭ��ͼR1��
#define R_NTC_25C   10000.0f    // 25��ʱNTC��ֵ
#define B_VALUE     3950.0f     // NTC��Bֵ�������ʵ��У׼��
#define V_REF       3.3f        // ADC�ο���ѹ��ʵ��У׼��

uint16_t AD_Value;

void AD_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 ,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA ,ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);
	
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);//��Ƶ 72M/6
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; 
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	//ѡ�����ͨ��
	ADC_RegularChannelConfig(ADC1,ADC_Channel_0,1,ADC_SampleTime_55Cycles5);//68��T
//	ADC_RegularChannelConfig(ADC1,ADC_Channel_2,2,ADC_SampleTime_55Cycles5);//68��T
	
	ADC_InitTypeDef ADC_InitStructure;
	ADC_InitStructure.ADC_ContinuousConvMode=DISABLE;//ɨ�赥�Ρ�����ENABLE
	ADC_InitStructure.ADC_DataAlign=ADC_DataAlign_Right;//����롢��
	ADC_InitStructure.ADC_ExternalTrigConv=ADC_ExternalTrigConv_None;//�������
	ADC_InitStructure.ADC_Mode=ADC_Mode_Independent;
	ADC_InitStructure.ADC_NbrOfChannel=1;//ɨ�輸��ͨ��
	ADC_InitStructure.ADC_ScanConvMode=DISABLE;//ɨ��ģʽENABLE
	ADC_Init(ADC1,&ADC_InitStructure);
	

	
	DMA_InitTypeDef DMA_InitStructure;
	
	DMA_InitStructure.DMA_PeripheralBaseAddr=(uint32_t)&ADC1->DR;//����վ�����ʼ��ַ�����ݿ�ȡ��Ƿ�����
	DMA_InitStructure.DMA_PeripheralDataSize=DMA_PeripheralDataSize_HalfWord ;
	DMA_InitStructure.DMA_PeripheralInc=DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryBaseAddr=(uint32_t)&AD_Value;//�Ĵ���վ�����ʼ��ַ�����ݿ�ȡ��Ƿ�����
	DMA_InitStructure.DMA_MemoryDataSize=DMA_MemoryDataSize_HalfWord ;
	DMA_InitStructure.DMA_MemoryInc=DMA_MemoryInc_Disable;
	DMA_InitStructure.DMA_DIR=DMA_DIR_PeripheralSRC;//���䷽��,���赽�洢��
	DMA_InitStructure.DMA_BufferSize=1;//��������С�����������,���伸��
	DMA_InitStructure.DMA_Mode=DMA_Mode_Normal;//�Ƿ�ʹ���Զ���װ
	DMA_InitStructure.DMA_M2M=DMA_M2M_Disable ;//Ӳ�����������(�洢�����洢����enable
	DMA_InitStructure.DMA_Priority=DMA_Priority_Medium ;
	
	DMA_Init(DMA1_Channel1,&DMA_InitStructure);
	
	DMA_Cmd(DMA1_Channel1,ENABLE);
	ADC_DMACmd(ADC1,ENABLE);
	ADC_Cmd(ADC1,ENABLE);
	ADC_ResetCalibration(ADC1);//��λУ׼
	while(ADC_GetResetCalibrationStatus(ADC1) == SET);//�ȵ���λУ׼=0,����ѭ��
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1) == SET);
	
	
}

void AD_GetValue(void)
{
	  DMA_Cmd(DMA1_Channel1,DISABLE);
		DMA_SetCurrDataCounter(DMA1_Channel1,1);
	  DMA_Cmd(DMA1_Channel1,ENABLE);
	
		ADC_SoftwareStartConvCmd(ADC1,ENABLE);//�������

		while(DMA_GetFlagStatus(DMA1_FLAG_TC1) == RESET);
		DMA_ClearFlag(DMA1_FLAG_TC1);
}

float ADC_ConvertToTemperature(uint16_t adc_val) {
    // ��ѹת����ԭ��ͼ��ѹ���㣩
    float V_out = adc_val * V_REF / 4095.0f;
    float R_ntc = R_FIXED * (V_REF / V_out - 1.0f);
    
    // Steinhart-Hart���̣�����Bֵ��NTC��
    float steinhart = logf(R_ntc / R_NTC_25C) / B_VALUE; // ��Ȼ����
    steinhart += 1.0f / 298.15f;  // 25��=298.15K
    return (1.0f / steinhart) - 273.15f; // ת���϶�
}


