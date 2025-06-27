#ifndef __MYSPI_H
#define __MYSPI_H
//外部不需要调用，就不用申明
//void MySPI_W_SS(uint8_t  BitValue);
//void MySPI_W_SCK(uint8_t  BitValue);
//void MySPI_W_MOSI(uint8_t  BitValue);
//uint8_t MySPI_R_MISO(void);
void MySPI_Init(void);
void MySPI_Start(void);
void MySPI_Stop(void);
//使用模式0，空闲状态SCK低电平，第一个边沿移入数据，第二个移出
uint8_t MySPI_SwapByte(uint8_t  ByteSend);

#endif
