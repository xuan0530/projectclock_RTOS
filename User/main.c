#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "Delay.h"
#include "Lcd_Driver.h"
#include "GUI.h"
#include "AD.h"
#include "MyDMA.h"
#include "MPU6050.h"
#include "W25Q32.h"
#include "esp_at.h"
#include "stdio.h"
#include "string.h"
#include "MyRTC.h"
#include "weatherapi.h" 
#include "weather_icons.h"


static SemaphoreHandle_t xLcdMutex = NULL;     // LCD��ʾ������
static SemaphoreHandle_t xEspMutex = NULL;     // ESP32����������

// =============================================================================
// ȫ�ֱ��� - ���ּ�
// =============================================================================
static WeatherData_t weather_data = {0};
static uint8_t last_weather_update_day = 0;
static float current_temp = 0.0f;
static uint8_t wifi_connected = 0;
static ScreenOrientation_t current_orientation = SCREEN_PORTRAIT;
static char ip_str[16];
static uint8_t screen_redraw_needed = 0;

// =============================================================================
// ��ȫ��LCD��������
// =============================================================================

/**
 * ��ȫ��������ʾ
 */
void SafeDrawFont(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t *s)
{
    if(xLcdMutex != NULL && xSemaphoreTake(xLcdMutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        Gui_DrawFont_GBK16(x, y, fc, bc, s);
        xSemaphoreGive(xLcdMutex);
    }
    else
    {
        // �����ȡ������������ֱ����ʾ�����⿨����
        Gui_DrawFont_GBK16(x, y, fc, bc, s);
    }
}

/**
 * ��ȫ���������
 */
void SafeClearArea(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color)
{
    if(xLcdMutex != NULL && xSemaphoreTake(xLcdMutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        Lcd_ClearArea(x, y, width, height, color);
        xSemaphoreGive(xLcdMutex);
    }
    else
    {
        Lcd_ClearArea(x, y, width, height, color);
    }
}

/**
 * ��ȫ�Ļ��ƿ�
 */
void SafeDrawBox(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t thickness)
{
    if(xLcdMutex != NULL && xSemaphoreTake(xLcdMutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        Gui_box2(x, y, width, height, thickness);
        xSemaphoreGive(xLcdMutex);
    }
    else
    {
        Gui_box2(x, y, width, height, thickness);
    }
}

/**
 * �ػ�����UI����
 */
void UI_RedrawAll(void)
{
    char temp_str[20];
    char ip_display[25];
    
    if(xLcdMutex != NULL && xSemaphoreTake(xLcdMutex, pdMS_TO_TICKS(200)) == pdTRUE)
    {
        // ����
        Lcd_Clear(BLACK);
        
        // ���»���UI���
        Gui_box2(0, 0, 128, 16, 2);   // ����
        Gui_box2(0, 16, 128, 32, 2);  // ʱ��
        Gui_box2(0, 48, 64, 48, 2);   // ����
        Gui_box2(64, 48, 64, 24, 2);  // �¶�
        Gui_box2(64, 72, 64, 24, 2);  // �����¶�
        Gui_box2(0, 96, 128, 32, 2);  // ������Ϣ
        
        // ��ʾ��ǩ
        //Gui_DrawFont_GBK16(68, 72, WHITE, BLACK, (uint8_t*)"inside:");
        
        // ������ʾ��ǰ����
        if (weather_data.is_valid) {
            Weather_DrawIcon(8, 48, weather_data.weather_text);
            sprintf(temp_str, "%s��C", weather_data.temperature);
            Gui_DrawFont_GBK16(68, 54, WHITE, BLACK, (uint8_t*)temp_str);
        }
        
        // ������ʾ������Ϣ
        if (wifi_connected) {
            Gui_DrawFont_GBK16(4, 98, GREEN, BLACK, (uint8_t*)"WiFi: OK");
            sprintf(ip_display, "IP:%s", ip_str);
            Gui_DrawFont_GBK16(4, 112, YELLOW, BLACK, (uint8_t*)ip_display);
        } else {
            Gui_DrawFont_GBK16(4, 98, RED, BLACK, (uint8_t*)"WiFi: Lost");
            Gui_DrawFont_GBK16(4, 112, RED, BLACK, (uint8_t*)"IP: None");
        }
        
        // ��ʾ��ǰ�����¶�
        sprintf(temp_str, "%.1f��C", current_temp);
        Gui_DrawFont_GBK16(70, 75, YELLOW, BLACK, (uint8_t*)temp_str);
        
        xSemaphoreGive(xLcdMutex);
    }
}


/**
 * ��ʾ���� - ʱ����ʾ����Ļ�ػ�
 */
void vTaskDisplay(void *pvParameters)
{
    char dateStr[20];
    char timeStr[20];
    uint8_t last_second = 61;
    
    for(;;)
    {
        // ����Ƿ���Ҫ�ػ���Ļ
        if(screen_redraw_needed)
        {
            screen_redraw_needed = 0;
            UI_RedrawAll();
            last_second = 61; // ǿ�Ƹ���ʱ����ʾ
        }
        
        // ��ȡRTCʱ��
        MyRTC_ReadTime();
        
        // ֻ�������仯ʱ�Ÿ�����ʾ
        if(MyRTC_Time[5] != last_second)
        {
            last_second = MyRTC_Time[5];
            
            sprintf(dateStr, "%04d-%02d-%02d", MyRTC_Time[0], MyRTC_Time[1], MyRTC_Time[2]);
            sprintf(timeStr, "%02d:%02d:%02d", MyRTC_Time[3], MyRTC_Time[4], MyRTC_Time[5]);
            
            // ʹ�ð�ȫ��ʾ����
            SafeDrawFont(16, 0, WHITE, BLACK, (uint8_t*)dateStr);
            SafeDrawFont(24, 24, WHITE, BLACK, (uint8_t*)timeStr);
        }
        
        // ÿ����һ��
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/**
 * ���������� - �¶ȶ�ȡ
 */
void vTaskSensor(void *pvParameters)
{
    char tempStr[20];
    
    for(;;)
    {
        // ��ȡ�¶�
        AD_GetValue();
        current_temp = ADC_ConvertToTemperature(AD_Value);
        
        // ����¶���ʾ������ʾ���¶�
        SafeClearArea(70, 82, 54, 14, BLACK);
        sprintf(tempStr, "%.1f��C", current_temp);
        SafeDrawFont(70, 75, YELLOW, BLACK, (uint8_t*)tempStr);
        
        // ÿ5�����һ��
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

/**
 * ��Ļ����������
 */
void vTaskOrientation(void *pvParameters)
{
    for(;;)
    {
        // �����Ļ����
        ScreenOrientation_t new_orientation = MPU6050_GetScreenOrientation();
        
        if (new_orientation != current_orientation) 
        {
            current_orientation = new_orientation;
            LCD_SetOrientation(current_orientation);
            
            // �����ػ��־
            screen_redraw_needed = 1;
        }
        
        // ÿ10����һ��
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

/**
 * �������� - ����״̬������������
 */
void vTaskNetwork(void *pvParameters)
{
    uint8_t check_count = 0;
    
    for(;;)
    {
        check_count++;
        
        // ÿ30����һ��WiFi״̬
        if(check_count >= 6) // 6 * 5�� = 30��
        {
            check_count = 0;
            
            // ʹ��ESP32�����������������
            uint8_t wifi_status = 0;
            char temp_ip[16];
            
            if(xEspMutex != NULL && xSemaphoreTake(xEspMutex, pdMS_TO_TICKS(1000)) == pdTRUE)
            {
                wifi_status = esp_at_wifi_status();
                if(wifi_status)
                {
                    esp_at_get_ip(temp_ip, sizeof(temp_ip));
                }
                xSemaphoreGive(xEspMutex);
            }
            else
            {
                // �����ȡ��������������ԭ���ķ���
                wifi_status = esp_at_wifi_status();
                if(wifi_status)
                {
                    esp_at_get_ip(temp_ip, sizeof(temp_ip));
                }
            }
            
            if(wifi_status)
            {
                if(!wifi_connected)
                {
                    wifi_connected = 1;
                    strcpy(ip_str, temp_ip);
                    SafeClearArea(1, 97, 126, 30, BLACK);
                    SafeDrawBox(0, 96, 128, 32, 2);
                    SafeDrawFont(4, 98, GREEN, BLACK, (uint8_t*)"WiFi: OK");
                    
                    char ip_display[25];
                    sprintf(ip_display, "IP:%s", ip_str);
                    SafeDrawFont(4, 112, YELLOW, BLACK, (uint8_t*)ip_display);
                }
            }
            else
            {
                if(wifi_connected)
                {
                    wifi_connected = 0;
                    SafeClearArea(1, 97, 126, 30, BLACK);
                    SafeDrawBox(0, 96, 128, 32, 2);
                    SafeDrawFont(4, 98, RED, BLACK, (uint8_t*)"WiFi: Lost");
                    SafeDrawFont(4, 112, RED, BLACK, (uint8_t*)"IP: None");
                }
            }
        }
        
        // �����������
        MyRTC_ReadTime();
        if(MyRTC_Time[2] != last_weather_update_day && wifi_connected)
        {
            SafeDrawFont(4, 98, YELLOW, BLACK, (uint8_t*)"Update...");
            
            // ʹ��ESP32������������������
            uint8_t weather_success = 0;
            if(xEspMutex != NULL && xSemaphoreTake(xEspMutex, pdMS_TO_TICKS(10000)) == pdTRUE)
            {
                weather_success = Weather_Update(&weather_data);
                xSemaphoreGive(xEspMutex);
            }
            else
            {
                // �����ȡ��������������ԭ���ķ���
                weather_success = Weather_Update(&weather_data);
            }
            
            if(weather_success)
            {
                // ������������ػ�
                SafeClearArea(1, 49, 62, 46, BLACK);
                SafeClearArea(65, 49, 62, 22, BLACK);
                
                if(xLcdMutex != NULL && xSemaphoreTake(xLcdMutex, pdMS_TO_TICKS(200)) == pdTRUE)
                {
                    Weather_DrawIcon(8, 48, weather_data.weather_text);
                    
                    char temp_str[16];
                    sprintf(temp_str, "%s��C", weather_data.temperature);
                    Gui_DrawFont_GBK16(68, 54, WHITE, BLACK, (uint8_t*)temp_str);
                    xSemaphoreGive(xLcdMutex);
                }
                
                last_weather_update_day = MyRTC_Time[2];
                
                // 2���ָ�������Ϣ��ʾ
                vTaskDelay(pdMS_TO_TICKS(2000));
                SafeClearArea(1, 97, 126, 30, BLACK);
                SafeDrawBox(0, 96, 128, 32, 2);
                SafeDrawFont(4, 98, GREEN, BLACK, (uint8_t*)"WiFi: OK");
                char ip_display[25];
                sprintf(ip_display, "IP:%s", ip_str);
                SafeDrawFont(4, 112, YELLOW, BLACK, (uint8_t*)ip_display);
            }
        }
        
        // ÿ5��ִ��һ��
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

// =============================================================================
// ϵͳ��ʼ����������
// =============================================================================

/**
 * ϵͳ��ʼ��
 */
void System_Init(void)
{
    // Ӳ����ʼ��
    MPU6050_Init();
    Lcd_Init();
    LCD_LED_SET;
    Lcd_Clear(BLACK);
    AD_Init();
    
    // ����UI���
    Gui_box2(0, 0, 128, 16, 2);   // ����
    Gui_box2(0, 16, 128, 32, 2);  // ʱ��
    Gui_box2(0, 48, 64, 48, 2);   // ����
    Gui_box2(64, 48, 64, 24, 2);  // �¶�
    Gui_box2(64, 72, 64, 24, 2);  // �����¶�
    Gui_box2(0, 96, 128, 32, 2);  // ������Ϣ
    
    // ��ʾ��ǩ
   // Gui_DrawFont_GBK16(68, 72, WHITE, BLACK, (uint8_t*)"inside:");
    
    // RTC��ʼ��
    MyRTC_Init();
    
    // �����ʼ��
    Gui_DrawFont_GBK16(4, 98, WHITE, BLACK, (uint8_t*)"Init...");
    
    esp_at_init();
    Delay_ms(100);
    esp_at_wifi_init();
    Delay_ms(100);
    esp_at_wifi_connect("AB", "123456789");
    Delay_ms(2000);
    
    // �������
    if(esp_at_wifi_status())
    {
        wifi_connected = 1;
        esp_at_get_ip(ip_str, sizeof(ip_str));
        esp_at_sntp_init();
        Delay_ms(500);
        MyRTC_SyncFromESP32();
        
        // ��ʼ��ȡ����
        Gui_DrawFont_GBK16(4, 98, YELLOW, BLACK, (uint8_t*)"Weather...");
        if(Weather_Update(&weather_data))
        {
            Weather_DrawIcon(8, 48, weather_data.weather_text);
            char temp_str[16];
            sprintf(temp_str, "%s��C", weather_data.temperature);
            Gui_DrawFont_GBK16(68, 54, WHITE, BLACK, (uint8_t*)temp_str);
        }
        
        MyRTC_ReadTime();
        last_weather_update_day = MyRTC_Time[2];
        
        Gui_DrawFont_GBK16(4, 98, GREEN, BLACK, (uint8_t*)"WiFi: OK");
        char ip_display[25];
        sprintf(ip_display, "IP:%s", ip_str);
        Gui_DrawFont_GBK16(4, 112, YELLOW, BLACK, (uint8_t*)ip_display);
    }
    else
    {
        Gui_DrawFont_GBK16(4, 98, RED, BLACK, (uint8_t*)"WiFi: Fail");
    }
}

/**
 * ������
 */
int main(void)
{
    // ����������
    xLcdMutex = xSemaphoreCreateMutex();
    xEspMutex = xSemaphoreCreateMutex();
    
    // ϵͳ��ʼ��
    System_Init();
    
    // ��������ʹ��֮ǰ���Գɹ��Ķ�ջ��С��
    xTaskCreate(vTaskDisplay,     "Display",     256, NULL, 4, NULL);   // ������ȼ�
    xTaskCreate(vTaskSensor,      "Sensor",      192, NULL, 3, NULL);   // �����ȼ�
    xTaskCreate(vTaskOrientation, "Orientation", 128, NULL, 2, NULL);   // �е����ȼ�
    xTaskCreate(vTaskNetwork,     "Network",     384, NULL, 1, NULL);   // ������ȼ�
    
    // ����������
    vTaskStartScheduler();
    
    // ��Զ���ᵽ������
    while(1)
    {
    }
}

/**
 * FreeRTOS������
 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    while(1);
}

void vApplicationMallocFailedHook(void)
{
    while(1);
}
