#include "weatherapi.h"
#include "esp_at.h"
#include <string.h>
#include <stdio.h>

// API���ӣ�ֻ���޸��������
const char* WEATHER_API_URL = "https://api.seniverse.com/v3/weather/now.json?key=SfesfR_4aLtHMcNwv&location=beijing&language=en&unit=c";

/**
  * ��    ��������JSON��������
  * ��    ����data - JSON�����ַ���
  *          weather - ������������ݽṹָ��
  * �� �� ֵ��0-ʧ�ܣ�1-�ɹ�
  */
static uint8_t parse_weather_json(const char* data, WeatherData_t* weather)
{
    // ���������ı�
    const char* text_start = strstr(data, "\"text\":\"");
    if (!text_start) return 0;
    text_start += 8; // ���� "text":"
    
    const char* text_end = strchr(text_start, '\"');
    if (!text_end) return 0;
    
    // ���������ı�
    size_t text_len = text_end - text_start;
    if (text_len >= sizeof(weather->weather_text)) text_len = sizeof(weather->weather_text) - 1;
    strncpy(weather->weather_text, text_start, text_len);
    weather->weather_text[text_len] = '\0';
    
    // �����¶�����
    const char* temp_start = strstr(data, "\"temperature\":\"");
    if (!temp_start) return 0;
    temp_start += 15; // ���� "temperature":"
    
    const char* temp_end = strchr(temp_start, '\"');
    if (!temp_end) return 0;
    
    // �����¶�����
    size_t temp_len = temp_end - temp_start;
    if (temp_len >= sizeof(weather->temperature)) temp_len = sizeof(weather->temperature) - 1;
    strncpy(weather->temperature, temp_start, temp_len);
    weather->temperature[temp_len] = '\0';
    
    // ���������Ч
    weather->is_valid = 1;
    
    return 1;
}

/**
  * ��    ����������������
  * ��    ����weather - ������������ݽṹָ��
  * �� �� ֵ��0-ʧ�ܣ�1-�ɹ�
  */
uint8_t Weather_Update(WeatherData_t* weather)
{
    const char* response;
    uint32_t response_len;
    
    // ��ʼ��weather�ṹ
    memset(weather, 0, sizeof(WeatherData_t));
    weather->is_valid = 0;
    
    // ����HTTP GET�����ȡ��������
    if (!esp_at_http_get(WEATHER_API_URL, &response, &response_len, 5000))
    {
        return 0;  // HTTP����ʧ��
    }
    
    // ����JSON����
    if (!parse_weather_json(response, weather))
    {
        return 0;  // ����ʧ��
    }
    
    return 1;  // ���³ɹ�
}

