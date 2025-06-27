#include "stm32f10x.h"                  // Device header
#include "esp_at.h"
#include "esp_uart.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "Delay.h"

// 定义接收缓冲区大小
#define RX_BUFFER_SIZE  4096

// 定义接收结果状态码
#define RX_RESULT_OK    0
#define RX_RESULT_ERROR 1
#define RX_RESULT_FAIL  2

// 接收缓冲区变量
static uint8_t rxdata[RX_BUFFER_SIZE];     // 接收数据缓冲区
static uint32_t rxlen;                     // 接收数据长度
static uint8_t rxready;                    // 接收就绪标志
static uint8_t rxresult;                   // 接收结果

/**
  * 函    数：UART接收回调函数
  * 参    数：data 接收到的单字节数据
  * 返 回 值：无
  */
static void on_uart_received(uint8_t data)
{
    // 没有数据请求，不接受数据
    if (!rxready)
    {
        return;
    }

    // 接收数据，防止缓冲区溢出
    if (rxlen < RX_BUFFER_SIZE)
    {
        rxdata[rxlen++] = data;            // 存储接收到的数据
    }
    else
    {
        rxresult = RX_RESULT_FAIL;         // 缓冲区溢出，标记为失败
        rxready = 0;                       // 关闭接收标志
        return;
    }

    // 数据接收完毕判断
    if (data == '\n')                      // 如果接收到换行符
    {
        // 判断接收换行符是否为\r\n
        if (rxlen >= 2 && rxdata[rxlen - 2] == '\r')
        {
            // 收到OK
            if (rxlen >= 4 && 
                rxdata[rxlen - 4] == 'O' && rxdata[rxlen - 3] == 'K')
            {
                rxresult = RX_RESULT_OK;   // 接收到OK响应，标记为成功
                rxready = 0;               // 关闭接收标志
            }
            // 收到ERROR
            else if (rxlen >= 7 && 
                     rxdata[rxlen - 7] == 'E' && rxdata[rxlen - 6] == 'R' && 
                     rxdata[rxlen - 5] == 'R' && rxdata[rxlen - 4] == 'O' && 
                     rxdata[rxlen - 3] == 'R')
            {
                rxresult = RX_RESULT_ERROR; // 接收到ERROR响应，标记为错误
                rxready = 0;                // 关闭接收标志
            }
        }
    }
}

/**
  * 函    数：ESP AT模块初始化
  * 参    数：无
  * 返 回 值：0-失败，1-成功
  */
uint8_t esp_at_init(void)
{
    rxready = 0;                           // 清空接收标志

    ESP_UART_Init();                       // 初始化UART
    ESP_UART_ReceiveRegister(on_uart_received);  // 注册接收回调函数

    return esp_at_reset();                 // 调用复位函数
}

/**
  * 函    数：发送AT命令并等待响应
  * 参    数：cmd 要发送的AT命令字符串
  * 参    数：rsp 响应数据的指针，使用输出参数的形式返回
  * 参    数：length 响应数据长度，使用输出参数的形式返回
  * 参    数：timeout 等待超时时间，单位毫秒
  * 返 回 值：0-失败，1-成功
  */
uint8_t esp_at_send_command(const char *cmd, const char **rsp, uint32_t *length, uint32_t timeout)
{
    uint32_t i;
    
    // 清空接收缓冲区
    rxlen = 0;                             // 清空接收长度
    rxready = 1;                           // 设置接收就绪标志
    rxresult = RX_RESULT_FAIL;             // 默认结果为失败

    // 发送AT命令
    for (i = 0; i < strlen(cmd); i++)
    {
        ESP_UART_SendByte(cmd[i]);         // 逐字节发送命令
    }
    // 发送回车换行
    ESP_UART_SendByte('\r');               // 发送回车符
    ESP_UART_SendByte('\n');               // 发送换行符

    // 等待响应或超时
    while (rxready && timeout--)
    {
        Delay_ms(1);                       // 延时1毫秒
    }
    rxready = 0;                           // 清除接收标志

    // 返回响应数据
    if (rsp)
    {
        *rsp = (const char *)rxdata;       // 设置响应数据指针
    }
    if (length)
    {
        *length = rxlen;                   // 设置响应长度
    }

    return rxresult == RX_RESULT_OK;       // 如果结果为OK则返回1，否则返回0
}

/**
  * 函    数：通过ESP AT模块发送数据
  * 参    数：data 要发送的数据数组
  * 参    数：length 要发送数据的长度
  * 返 回 值：0-失败，1-成功
  */
uint8_t esp_at_send_data(const uint8_t *data, uint32_t length)
{
    uint32_t i;
    
    // 直接发送数据
    for (i = 0; i < length; i++)
    {
        ESP_UART_SendByte(data[i]);        // 逐字节发送数据
    }

    return 1;                              // 返回成功
}

/**
  * 函    数：重置ESP AT模块
  * 参    数：无
  * 返 回 值：0-失败，1-成功
  */
uint8_t esp_at_reset(void)
{
    // 复位ESP32
    if (!esp_at_send_command("AT+RESTORE", NULL, NULL, 1000))
    {
        return 0;                          // 发送复位指令失败，返回0
    }
    // 等待重启完成
    Delay_ms(2000);                        // 延时2000毫秒，等待模块重启
    
    // 关闭回显
    if (!esp_at_send_command("ATE0", NULL, NULL, 1000))
    {
        return 0;                          // 关闭回显失败，返回0
    }
    
    // 关闭配置存储
    if (!esp_at_send_command("AT+SYSSTORE=0", NULL, NULL, 1000))
    {
        return 0;                          // 关闭配置存储失败，返回0
    }

    return 1;                              // 全部步骤成功，返回1
}

/**
  * 函    数：初始化WiFi模式
  * 参    数：无
  * 返 回 值：0-失败，1-成功
  */
uint8_t esp_at_wifi_init(void)
{
    // 设置为station模式
    if (!esp_at_send_command("AT+CWMODE=1", NULL, NULL, 1000))
    {
        return 0;                          // 设置模式失败，返回0
    }

    return 1;                              // 设置成功，返回1
}

/**
  * 函    数：连接WiFi网络
  * 参    数：ssid WiFi名称
  * 参    数：pwd WiFi密码
  * 返 回 值：0-失败，1-成功
  */
uint8_t esp_at_wifi_connect(const char *ssid, const char *pwd)
{
    char cmd[64];                          // 定义命令缓冲区

    // 连接WiFi，构建命令字符串
    sprintf(cmd, "AT+CWJAP=\"%s\",\"%s\"", ssid, pwd);  // 格式化命令字符串
    if (!esp_at_send_command(cmd, NULL, NULL, 10000))
    {
        return 0;                          // 连接失败，返回0
    }

    return 1;                              // 连接成功，返回1
}

/**
  * 函    数：执行HTTP GET请求
  * 参    数：url HTTP请求的URL地址
  * 参    数：rsp 响应数据的指针，使用输出参数的形式返回
  * 参    数：length 响应数据长度，使用输出参数的形式返回
  * 参    数：timeout 等待超时时间，单位毫秒
  * 返 回 值：0-失败，1-成功
  */
uint8_t esp_at_http_get(const char *url, const char **rsp, uint32_t *length, uint32_t timeout)
{
    char cmd[128];                         // 定义命令缓冲区

    // 构建HTTP GET命令
    sprintf(cmd, "AT+HTTPCGET=\"%s\"", url);  // 格式化命令字符串
    if (!esp_at_send_command(cmd, rsp, length, timeout))
    {
        return 0;                          // 请求失败，返回0
    }

    return 1;                              // 请求成功，返回1
}

/**
  * 函    数：初始化SNTP时间同步
  * 参    数：无
  * 返 回 值：0-失败，1-成功
  */
uint8_t esp_at_sntp_init(void)
{
    // 配置SNTP服务器
    if (!esp_at_send_command("AT+CIPSNTPCFG=1,8,\"cn.ntp.org.cn\",\"ntp.sjtu.edu.cn\"", NULL, NULL, 1000))
    {
        return 0;                          // 配置SNTP服务器失败，返回0
    }

    // 查询SNTP时间，确认配置成功
    if (!esp_at_send_command("AT+CIPSNTPTIME?", NULL, NULL, 1000))
    {
        return 0;                          // 查询时间失败，返回0
    }

    return 1;                              // 配置成功，返回1
}

/**
  * 函    数：获取当前时间戳
  * 参    数：timestamp 时间戳指针，使用输出参数的形式返回
  * 返 回 值：0-失败，1-成功
  */
uint8_t esp_at_time_get(uint32_t *timestamp)
{
    const char *rsp;                       // 响应数据指针
    uint32_t length;                       // 响应数据长度

    // 查询系统时间戳
    if (!esp_at_send_command("AT+SYSTIMESTAMP?", &rsp, &length, 1000))
    {
        return 0;                          // 查询失败，返回0
    }

    // 解析时间戳响应
    char *sts = strstr(rsp, "+SYSTIMESTAMP:"); // 查找时间戳标识
    if (sts != NULL)
    {
        sts += strlen("+SYSTIMESTAMP:");   // 指针移动到时间戳数值
        *timestamp = atoi(sts);            // 转换为整数
        return 1;                          // 解析成功，返回1
    }

    return 0;                              // 解析失败，返回0
}


/**
  * 函    数：获取ESP32的IP地址
  * 参    数：ip_str IP地址字符串缓冲区
  * 参    数：buf_size 缓冲区大小
  * 返 回 值：0-失败，1-成功
  */
uint8_t esp_at_get_ip(char *ip_str, uint8_t buf_size)
{
    const char *rsp;                       // 响应数据指针
    uint32_t length;                       // 响应数据长度

    // 查询IP地址
    if (!esp_at_send_command("AT+CIPSTA?", &rsp, &length, 1000))
    {
        return 0;                          // 查询失败，返回0
    }

    // 解析IP地址响应
    // 响应格式: +CIPSTA:ip:"192.168.1.100"
    char *ip_start = strstr(rsp, "+CIPSTA:ip:\""); 
    if (ip_start != NULL)
    {
        ip_start += strlen("+CIPSTA:ip:\"");   // 指针移动到IP地址开始
        char *ip_end = strchr(ip_start, '\"'); // 查找结束引号
        if (ip_end != NULL)
        {
            uint8_t ip_len = ip_end - ip_start;
            if (ip_len < buf_size)
            {
                strncpy(ip_str, ip_start, ip_len);
                ip_str[ip_len] = '\0';      // 添加字符串结束符
                return 1;                   // 解析成功，返回1
            }
        }
    }

    return 0;                              // 解析失败，返回0
}

/**
  * 函    数：获取WiFi连接状态
  * 参    数：无
  * 返 回 值：0-未连接，1-已连接
  */
uint8_t esp_at_wifi_status(void)
{
    const char *rsp;                       // 响应数据指针
    uint32_t length;                       // 响应数据长度

    // 查询WiFi状态
    if (!esp_at_send_command("AT+CWJAP?", &rsp, &length, 1000))
    {
        return 0;                          // 查询失败，返回0
    }

    // 检查是否包含WiFi连接信息
    if (strstr(rsp, "+CWJAP:") != NULL)
    {
        return 1;                          // 已连接
    }

    return 0;                              // 未连接
}

