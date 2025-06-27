#ifndef __WEATHERAPI_H
#define __WEATHERAPI_H

#include <stdint.h>

// �������ݽṹ
typedef struct {
    char weather_text[32];   // ����״̬�ı�����"Haze"��"Sunny"��
    char temperature[8];     // �¶�ֵ������λ����"31"
    uint8_t is_valid;        // �����Ƿ���Ч
} WeatherData_t;

// ��������
uint8_t Weather_Update(WeatherData_t* weather);

#endif // __MY_WEATHER_H

