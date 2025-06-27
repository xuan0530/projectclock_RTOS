#ifndef __WEATHER_ICONS_H
#define __WEATHER_ICONS_H

#include <stdint.h>

typedef struct
{
    const uint16_t width;
    const uint16_t height;
    const uint8_t *data;
} st_image_t;

// 外部声明各个图标
extern const st_image_t icon_weather_duoyun;  // 多云图标
extern const st_image_t icon_weather_qing;    // 晴天图标
extern const st_image_t icon_weather_yin;     // 阴天图标
extern const st_image_t icon_weather_yu;      // 雨天图标
extern const st_image_t icon_weather_xue;     // 雪天图标
extern const st_image_t icon_weather_feng;    // 风天图标

// 天气图标绘制函数
void Weather_DrawIcon(uint8_t x, uint8_t y, const char* weather_text);

#endif /* __WEATHER_ICONS_H */
