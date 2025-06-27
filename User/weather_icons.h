#ifndef __WEATHER_ICONS_H
#define __WEATHER_ICONS_H

#include <stdint.h>

typedef struct
{
    const uint16_t width;
    const uint16_t height;
    const uint8_t *data;
} st_image_t;

// �ⲿ��������ͼ��
extern const st_image_t icon_weather_duoyun;  // ����ͼ��
extern const st_image_t icon_weather_qing;    // ����ͼ��
extern const st_image_t icon_weather_yin;     // ����ͼ��
extern const st_image_t icon_weather_yu;      // ����ͼ��
extern const st_image_t icon_weather_xue;     // ѩ��ͼ��
extern const st_image_t icon_weather_feng;    // ����ͼ��

// ����ͼ����ƺ���
void Weather_DrawIcon(uint8_t x, uint8_t y, const char* weather_text);

#endif /* __WEATHER_ICONS_H */
