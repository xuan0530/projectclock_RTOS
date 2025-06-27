#include "weatherapi.h"
#include "esp_at.h"
#include <string.h>
#include <stdio.h>

// API链接，只需修改这个常量
const char* WEATHER_API_URL = "https://api.seniverse.com/v3/weather/now.json?key=SfesfR_4aLtHMcNwv&location=beijing&language=en&unit=c";

/**
  * 函    数：解析JSON天气数据
  * 参    数：data - JSON数据字符串
  *          weather - 输出的天气数据结构指针
  * 返 回 值：0-失败，1-成功
  */
static uint8_t parse_weather_json(const char* data, WeatherData_t* weather)
{
    // 查找天气文本
    const char* text_start = strstr(data, "\"text\":\"");
    if (!text_start) return 0;
    text_start += 8; // 跳过 "text":"
    
    const char* text_end = strchr(text_start, '\"');
    if (!text_end) return 0;
    
    // 复制天气文本
    size_t text_len = text_end - text_start;
    if (text_len >= sizeof(weather->weather_text)) text_len = sizeof(weather->weather_text) - 1;
    strncpy(weather->weather_text, text_start, text_len);
    weather->weather_text[text_len] = '\0';
    
    // 查找温度数据
    const char* temp_start = strstr(data, "\"temperature\":\"");
    if (!temp_start) return 0;
    temp_start += 15; // 跳过 "temperature":"
    
    const char* temp_end = strchr(temp_start, '\"');
    if (!temp_end) return 0;
    
    // 复制温度数据
    size_t temp_len = temp_end - temp_start;
    if (temp_len >= sizeof(weather->temperature)) temp_len = sizeof(weather->temperature) - 1;
    strncpy(weather->temperature, temp_start, temp_len);
    weather->temperature[temp_len] = '\0';
    
    // 标记数据有效
    weather->is_valid = 1;
    
    return 1;
}

/**
  * 函    数：更新天气数据
  * 参    数：weather - 输出的天气数据结构指针
  * 返 回 值：0-失败，1-成功
  */
uint8_t Weather_Update(WeatherData_t* weather)
{
    const char* response;
    uint32_t response_len;
    
    // 初始化weather结构
    memset(weather, 0, sizeof(WeatherData_t));
    weather->is_valid = 0;
    
    // 发送HTTP GET请求获取天气数据
    if (!esp_at_http_get(WEATHER_API_URL, &response, &response_len, 5000))
    {
        return 0;  // HTTP请求失败
    }
    
    // 解析JSON数据
    if (!parse_weather_json(response, weather))
    {
        return 0;  // 解析失败
    }
    
    return 1;  // 更新成功
}

