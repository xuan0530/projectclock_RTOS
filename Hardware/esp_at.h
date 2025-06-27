#ifndef __ESP_AT_H__
#define __ESP_AT_H__

#include <stdint.h>

/**
  * ��    ����ESP ATģ���ʼ��
  * ��    ������
  * �� �� ֵ��0-ʧ�ܣ�1-�ɹ�
  */
uint8_t esp_at_init(void);

/**
  * ��    ��������AT����ȴ���Ӧ
  * ��    ����cmd Ҫ���͵�AT�����ַ���
  * ��    ����rsp ��Ӧ���ݵ�ָ�룬ʹ�������������ʽ����
  * ��    ����length ��Ӧ���ݳ��ȣ�ʹ�������������ʽ����
  * ��    ����timeout �ȴ���ʱʱ�䣬��λ����
  * �� �� ֵ��0-ʧ�ܣ�1-�ɹ�
  */
uint8_t esp_at_send_command(const char *cmd, const char **rsp, uint32_t *length, uint32_t timeout);

/**
  * ��    ����ͨ��ESP ATģ�鷢������
  * ��    ����data Ҫ���͵���������
  * ��    ����length Ҫ�������ݵĳ���
  * �� �� ֵ��0-ʧ�ܣ�1-�ɹ�
  */
uint8_t esp_at_send_data(const uint8_t *data, uint32_t length);

/**
  * ��    ��������ESP ATģ��
  * ��    ������
  * �� �� ֵ��0-ʧ�ܣ�1-�ɹ�
  */
uint8_t esp_at_reset(void);

/**
  * ��    ������ʼ��WiFiģʽ
  * ��    ������
  * �� �� ֵ��0-ʧ�ܣ�1-�ɹ�
  */
uint8_t esp_at_wifi_init(void);

/**
  * ��    ��������WiFi����
  * ��    ����ssid WiFi����
  * ��    ����pwd WiFi����
  * �� �� ֵ��0-ʧ�ܣ�1-�ɹ�
  */
uint8_t esp_at_wifi_connect(const char *ssid, const char *pwd);

/**
  * ��    ����ִ��HTTP GET����
  * ��    ����url HTTP�����URL��ַ
  * ��    ����rsp ��Ӧ���ݵ�ָ�룬ʹ�������������ʽ����
  * ��    ����length ��Ӧ���ݳ��ȣ�ʹ�������������ʽ����
  * ��    ����timeout �ȴ���ʱʱ�䣬��λ����
  * �� �� ֵ��0-ʧ�ܣ�1-�ɹ�
  */
uint8_t esp_at_http_get(const char *url, const char **rsp, uint32_t *length, uint32_t timeout);

/**
  * ��    ������ʼ��SNTPʱ��ͬ��
  * ��    ������
  * �� �� ֵ��0-ʧ�ܣ�1-�ɹ�
  */
uint8_t esp_at_sntp_init(void);

/**
  * ��    ������ȡ��ǰʱ���
  * ��    ����timestamp ʱ���ָ�룬ʹ�������������ʽ����
  * �� �� ֵ��0-ʧ�ܣ�1-�ɹ�
  */
uint8_t esp_at_time_get(uint32_t *timestamp);
//ip
uint8_t esp_at_get_ip(char *ip_str, uint8_t buf_size);
uint8_t esp_at_wifi_status(void);

#endif /* __ESP_AT_H__ */
