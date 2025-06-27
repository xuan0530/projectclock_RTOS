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


static SemaphoreHandle_t xLcdMutex = NULL;     // LCD显示互斥锁
static SemaphoreHandle_t xEspMutex = NULL;     // ESP32操作互斥锁

// =============================================================================
// 全局变量 - 保持简单
// =============================================================================
static WeatherData_t weather_data = {0};
static uint8_t last_weather_update_day = 0;
static float current_temp = 0.0f;
static uint8_t wifi_connected = 0;
static ScreenOrientation_t current_orientation = SCREEN_PORTRAIT;
static char ip_str[16];
static uint8_t screen_redraw_needed = 0;

// =============================================================================
// 安全的LCD操作函数
// =============================================================================

/**
 * 安全的字体显示
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
        // 如果获取不到互斥锁，直接显示（避免卡死）
        Gui_DrawFont_GBK16(x, y, fc, bc, s);
    }
}

/**
 * 安全的清除区域
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
 * 安全的绘制框
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
 * 重绘整个UI界面
 */
void UI_RedrawAll(void)
{
    char temp_str[20];
    char ip_display[25];
    
    if(xLcdMutex != NULL && xSemaphoreTake(xLcdMutex, pdMS_TO_TICKS(200)) == pdTRUE)
    {
        // 清屏
        Lcd_Clear(BLACK);
        
        // 重新绘制UI框架
        Gui_box2(0, 0, 128, 16, 2);   // 日期
        Gui_box2(0, 16, 128, 32, 2);  // 时间
        Gui_box2(0, 48, 64, 48, 2);   // 天气
        Gui_box2(64, 48, 64, 24, 2);  // 温度
        Gui_box2(64, 72, 64, 24, 2);  // 环境温度
        Gui_box2(0, 96, 128, 32, 2);  // 网络信息
        
        // 显示标签
        //Gui_DrawFont_GBK16(68, 72, WHITE, BLACK, (uint8_t*)"inside:");
        
        // 重新显示当前数据
        if (weather_data.is_valid) {
            Weather_DrawIcon(8, 48, weather_data.weather_text);
            sprintf(temp_str, "%s°C", weather_data.temperature);
            Gui_DrawFont_GBK16(68, 54, WHITE, BLACK, (uint8_t*)temp_str);
        }
        
        // 重新显示网络信息
        if (wifi_connected) {
            Gui_DrawFont_GBK16(4, 98, GREEN, BLACK, (uint8_t*)"WiFi: OK");
            sprintf(ip_display, "IP:%s", ip_str);
            Gui_DrawFont_GBK16(4, 112, YELLOW, BLACK, (uint8_t*)ip_display);
        } else {
            Gui_DrawFont_GBK16(4, 98, RED, BLACK, (uint8_t*)"WiFi: Lost");
            Gui_DrawFont_GBK16(4, 112, RED, BLACK, (uint8_t*)"IP: None");
        }
        
        // 显示当前环境温度
        sprintf(temp_str, "%.1f°C", current_temp);
        Gui_DrawFont_GBK16(70, 75, YELLOW, BLACK, (uint8_t*)temp_str);
        
        xSemaphoreGive(xLcdMutex);
    }
}


/**
 * 显示任务 - 时间显示和屏幕重绘
 */
void vTaskDisplay(void *pvParameters)
{
    char dateStr[20];
    char timeStr[20];
    uint8_t last_second = 61;
    
    for(;;)
    {
        // 检查是否需要重绘屏幕
        if(screen_redraw_needed)
        {
            screen_redraw_needed = 0;
            UI_RedrawAll();
            last_second = 61; // 强制更新时间显示
        }
        
        // 读取RTC时间
        MyRTC_ReadTime();
        
        // 只有秒数变化时才更新显示
        if(MyRTC_Time[5] != last_second)
        {
            last_second = MyRTC_Time[5];
            
            sprintf(dateStr, "%04d-%02d-%02d", MyRTC_Time[0], MyRTC_Time[1], MyRTC_Time[2]);
            sprintf(timeStr, "%02d:%02d:%02d", MyRTC_Time[3], MyRTC_Time[4], MyRTC_Time[5]);
            
            // 使用安全显示函数
            SafeDrawFont(16, 0, WHITE, BLACK, (uint8_t*)dateStr);
            SafeDrawFont(24, 24, WHITE, BLACK, (uint8_t*)timeStr);
        }
        
        // 每秒检查一次
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/**
 * 传感器任务 - 温度读取
 */
void vTaskSensor(void *pvParameters)
{
    char tempStr[20];
    
    for(;;)
    {
        // 读取温度
        AD_GetValue();
        current_temp = ADC_ConvertToTemperature(AD_Value);
        
        // 清除温度显示区域并显示新温度
        SafeClearArea(70, 82, 54, 14, BLACK);
        sprintf(tempStr, "%.1f°C", current_temp);
        SafeDrawFont(70, 75, YELLOW, BLACK, (uint8_t*)tempStr);
        
        // 每5秒更新一次
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

/**
 * 屏幕方向检测任务
 */
void vTaskOrientation(void *pvParameters)
{
    for(;;)
    {
        // 检测屏幕方向
        ScreenOrientation_t new_orientation = MPU6050_GetScreenOrientation();
        
        if (new_orientation != current_orientation) 
        {
            current_orientation = new_orientation;
            LCD_SetOrientation(current_orientation);
            
            // 设置重绘标志
            screen_redraw_needed = 1;
        }
        
        // 每10秒检查一次
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

/**
 * 网络任务 - 网络状态检查和天气更新
 */
void vTaskNetwork(void *pvParameters)
{
    uint8_t check_count = 0;
    
    for(;;)
    {
        check_count++;
        
        // 每30秒检查一次WiFi状态
        if(check_count >= 6) // 6 * 5秒 = 30秒
        {
            check_count = 0;
            
            // 使用ESP32互斥锁保护网络操作
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
                // 如果获取不到互斥锁，用原来的方法
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
        
        // 检查天气更新
        MyRTC_ReadTime();
        if(MyRTC_Time[2] != last_weather_update_day && wifi_connected)
        {
            SafeDrawFont(4, 98, YELLOW, BLACK, (uint8_t*)"Update...");
            
            // 使用ESP32互斥锁保护天气更新
            uint8_t weather_success = 0;
            if(xEspMutex != NULL && xSemaphoreTake(xEspMutex, pdMS_TO_TICKS(10000)) == pdTRUE)
            {
                weather_success = Weather_Update(&weather_data);
                xSemaphoreGive(xEspMutex);
            }
            else
            {
                // 如果获取不到互斥锁，用原来的方法
                weather_success = Weather_Update(&weather_data);
            }
            
            if(weather_success)
            {
                // 清空天气区域并重绘
                SafeClearArea(1, 49, 62, 46, BLACK);
                SafeClearArea(65, 49, 62, 22, BLACK);
                
                if(xLcdMutex != NULL && xSemaphoreTake(xLcdMutex, pdMS_TO_TICKS(200)) == pdTRUE)
                {
                    Weather_DrawIcon(8, 48, weather_data.weather_text);
                    
                    char temp_str[16];
                    sprintf(temp_str, "%s°C", weather_data.temperature);
                    Gui_DrawFont_GBK16(68, 54, WHITE, BLACK, (uint8_t*)temp_str);
                    xSemaphoreGive(xLcdMutex);
                }
                
                last_weather_update_day = MyRTC_Time[2];
                
                // 2秒后恢复网络信息显示
                vTaskDelay(pdMS_TO_TICKS(2000));
                SafeClearArea(1, 97, 126, 30, BLACK);
                SafeDrawBox(0, 96, 128, 32, 2);
                SafeDrawFont(4, 98, GREEN, BLACK, (uint8_t*)"WiFi: OK");
                char ip_display[25];
                sprintf(ip_display, "IP:%s", ip_str);
                SafeDrawFont(4, 112, YELLOW, BLACK, (uint8_t*)ip_display);
            }
        }
        
        // 每5秒执行一次
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

// =============================================================================
// 系统初始化和主函数
// =============================================================================

/**
 * 系统初始化
 */
void System_Init(void)
{
    // 硬件初始化
    MPU6050_Init();
    Lcd_Init();
    LCD_LED_SET;
    Lcd_Clear(BLACK);
    AD_Init();
    
    // 绘制UI框架
    Gui_box2(0, 0, 128, 16, 2);   // 日期
    Gui_box2(0, 16, 128, 32, 2);  // 时间
    Gui_box2(0, 48, 64, 48, 2);   // 天气
    Gui_box2(64, 48, 64, 24, 2);  // 温度
    Gui_box2(64, 72, 64, 24, 2);  // 环境温度
    Gui_box2(0, 96, 128, 32, 2);  // 网络信息
    
    // 显示标签
   // Gui_DrawFont_GBK16(68, 72, WHITE, BLACK, (uint8_t*)"inside:");
    
    // RTC初始化
    MyRTC_Init();
    
    // 网络初始化
    Gui_DrawFont_GBK16(4, 98, WHITE, BLACK, (uint8_t*)"Init...");
    
    esp_at_init();
    Delay_ms(100);
    esp_at_wifi_init();
    Delay_ms(100);
    esp_at_wifi_connect("AB", "123456789");
    Delay_ms(2000);
    
    // 检查连接
    if(esp_at_wifi_status())
    {
        wifi_connected = 1;
        esp_at_get_ip(ip_str, sizeof(ip_str));
        esp_at_sntp_init();
        Delay_ms(500);
        MyRTC_SyncFromESP32();
        
        // 初始获取天气
        Gui_DrawFont_GBK16(4, 98, YELLOW, BLACK, (uint8_t*)"Weather...");
        if(Weather_Update(&weather_data))
        {
            Weather_DrawIcon(8, 48, weather_data.weather_text);
            char temp_str[16];
            sprintf(temp_str, "%s°C", weather_data.temperature);
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
 * 主函数
 */
int main(void)
{
    // 创建互斥锁
    xLcdMutex = xSemaphoreCreateMutex();
    xEspMutex = xSemaphoreCreateMutex();
    
    // 系统初始化
    System_Init();
    
    // 创建任务（使用之前测试成功的堆栈大小）
    xTaskCreate(vTaskDisplay,     "Display",     256, NULL, 4, NULL);   // 最高优先级
    xTaskCreate(vTaskSensor,      "Sensor",      192, NULL, 3, NULL);   // 高优先级
    xTaskCreate(vTaskOrientation, "Orientation", 128, NULL, 2, NULL);   // 中等优先级
    xTaskCreate(vTaskNetwork,     "Network",     384, NULL, 1, NULL);   // 最低优先级
    
    // 启动调度器
    vTaskStartScheduler();
    
    // 永远不会到达这里
    while(1)
    {
    }
}

/**
 * FreeRTOS错误处理
 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    while(1);
}

void vApplicationMallocFailedHook(void)
{
    while(1);
}
