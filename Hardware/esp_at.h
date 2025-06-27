#ifndef __ESP_AT_H__
#define __ESP_AT_H__

#include <stdint.h>

/**
  * 函    数：ESP AT模块初始化
  * 参    数：无
  * 返 回 值：0-失败，1-成功
  */
uint8_t esp_at_init(void);

/**
  * 函    数：发送AT命令并等待响应
  * 参    数：cmd 要发送的AT命令字符串
  * 参    数：rsp 响应数据的指针，使用输出参数的形式返回
  * 参    数：length 响应数据长度，使用输出参数的形式返回
  * 参    数：timeout 等待超时时间，单位毫秒
  * 返 回 值：0-失败，1-成功
  */
uint8_t esp_at_send_command(const char *cmd, const char **rsp, uint32_t *length, uint32_t timeout);

/**
  * 函    数：通过ESP AT模块发送数据
  * 参    数：data 要发送的数据数组
  * 参    数：length 要发送数据的长度
  * 返 回 值：0-失败，1-成功
  */
uint8_t esp_at_send_data(const uint8_t *data, uint32_t length);

/**
  * 函    数：重置ESP AT模块
  * 参    数：无
  * 返 回 值：0-失败，1-成功
  */
uint8_t esp_at_reset(void);

/**
  * 函    数：初始化WiFi模式
  * 参    数：无
  * 返 回 值：0-失败，1-成功
  */
uint8_t esp_at_wifi_init(void);

/**
  * 函    数：连接WiFi网络
  * 参    数：ssid WiFi名称
  * 参    数：pwd WiFi密码
  * 返 回 值：0-失败，1-成功
  */
uint8_t esp_at_wifi_connect(const char *ssid, const char *pwd);

/**
  * 函    数：执行HTTP GET请求
  * 参    数：url HTTP请求的URL地址
  * 参    数：rsp 响应数据的指针，使用输出参数的形式返回
  * 参    数：length 响应数据长度，使用输出参数的形式返回
  * 参    数：timeout 等待超时时间，单位毫秒
  * 返 回 值：0-失败，1-成功
  */
uint8_t esp_at_http_get(const char *url, const char **rsp, uint32_t *length, uint32_t timeout);

/**
  * 函    数：初始化SNTP时间同步
  * 参    数：无
  * 返 回 值：0-失败，1-成功
  */
uint8_t esp_at_sntp_init(void);

/**
  * 函    数：获取当前时间戳
  * 参    数：timestamp 时间戳指针，使用输出参数的形式返回
  * 返 回 值：0-失败，1-成功
  */
uint8_t esp_at_time_get(uint32_t *timestamp);
//ip
uint8_t esp_at_get_ip(char *ip_str, uint8_t buf_size);
uint8_t esp_at_wifi_status(void);

#endif /* __ESP_AT_H__ */
