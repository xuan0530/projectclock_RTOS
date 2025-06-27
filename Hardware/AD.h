#ifndef __AD_H
#define __AD_H

extern uint16_t AD_Value;

void AD_Init(void);
void AD_GetValue(void);
float ADC_ConvertToTemperature(uint16_t adc_val);

#endif
