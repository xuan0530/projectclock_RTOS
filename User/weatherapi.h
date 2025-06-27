#ifndef __WEATHERAPI_H
#define __WEATHERAPI_H

#include <stdint.h>

// 天气数据结构
typedef struct {
    char weather_text[32];   // 天气状态文本，如"Haze"、"Sunny"等
    char temperature[8];     // 温度值，带单位，如"31"
    uint8_t is_valid;        // 数据是否有效
} WeatherData_t;

// 函数声明
uint8_t Weather_Update(WeatherData_t* weather);

#endif // __MY_WEATHER_H

