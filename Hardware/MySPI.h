#ifndef __MYSPI_H
#define __MYSPI_H
//�ⲿ����Ҫ���ã��Ͳ�������
//void MySPI_W_SS(uint8_t  BitValue);
//void MySPI_W_SCK(uint8_t  BitValue);
//void MySPI_W_MOSI(uint8_t  BitValue);
//uint8_t MySPI_R_MISO(void);
void MySPI_Init(void);
void MySPI_Start(void);
void MySPI_Stop(void);
//ʹ��ģʽ0������״̬SCK�͵�ƽ����һ�������������ݣ��ڶ����Ƴ�
uint8_t MySPI_SwapByte(uint8_t  ByteSend);

#endif
