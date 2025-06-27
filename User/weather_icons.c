#include "stm32f10x.h"
#include "weather_icons.h"
#include "GUI.h"
#include "Lcd_Driver.h"
#include <string.h>

// 绘制天气图标
void Weather_DrawIcon(u8 x, u8 y, const char* weather_text)
{
    const st_image_t *img = NULL;
    
    if (strcmp(weather_text, "Cloudy") == 0 || 
        strcmp(weather_text, "Partly Cloudy") == 0 || 
        strcmp(weather_text, "Mostly Cloudy") == 0) {
        img = &icon_weather_duoyun;
    } 
    else if (strcmp(weather_text, "Wind") == 0 || 
             strcmp(weather_text, "Dust") == 0 || 
             strcmp(weather_text, "Sand") == 0 || 
             strcmp(weather_text, "Windy") == 0) {
        img = &icon_weather_feng;
    } 
    else if (strcmp(weather_text, "Sunny") == 0 || 
             strcmp(weather_text, "Clear") == 0 || 
             strcmp(weather_text, "Fair") == 0) {
        img = &icon_weather_qing;
    } 
    else if (strcmp(weather_text, "Snow") == 0 || 
             strcmp(weather_text, "Light Snow") == 0 || 
             strcmp(weather_text, "Moderate Snow") == 0 || 
             strcmp(weather_text, "Heavy Snow") == 0) {
        img = &icon_weather_xue;
    } 
    else if (strcmp(weather_text, "Overcast") == 0 || 
             strcmp(weather_text, "Fog") == 0 || 
             strcmp(weather_text, "Haze") == 0) {
        img = &icon_weather_yin;
    } 
    else if (strcmp(weather_text, "Rain") == 0 || 
             strcmp(weather_text, "Shower") == 0 || 
             strcmp(weather_text, "Thundershower") == 0 || 
             strcmp(weather_text, "Light Rain") == 0 || 
             strcmp(weather_text, "Moderate Rain") == 0 || 
             strcmp(weather_text, "Heavy Rain") == 0) {
        img = &icon_weather_yu;
    }
    
    if (img != NULL) {
        // 使用LCD驱动中的函数绘制图像
        for(u16 i = 0; i < img->height; i++) {
            for(u16 j = 0; j < img->width; j++) {
                u16 index = (i * img->width + j) * 2;  // 每个像素2字节
                u16 color = (img->data[index] << 8) | img->data[index + 1]; //高字节左移8位        |  低字节     组合成16位颜色值
                Gui_DrawPoint(x + j, y + i, color);
            }
        }
    } else {
        // 如果没有匹配的图标，显示文本
        Gui_DrawFont_GBK16(x, y, WHITE, BLACK, (u8*)weather_text);
    }
}
