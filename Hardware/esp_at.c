#include "stm32f10x.h"                  // Device header
#include "esp_at.h"
#include "esp_uart.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "Delay.h"

// ������ջ�������С
#define RX_BUFFER_SIZE  4096

// ������ս��״̬��
#define RX_RESULT_OK    0
#define RX_RESULT_ERROR 1
#define RX_RESULT_FAIL  2

// ���ջ���������
static uint8_t rxdata[RX_BUFFER_SIZE];     // �������ݻ�����
static uint32_t rxlen;                     // �������ݳ���
static uint8_t rxready;                    // ���վ�����־
static uint8_t rxresult;                   // ���ս��

/**
  * ��    ����UART���ջص�����
  * ��    ����data ���յ��ĵ��ֽ�����
  * �� �� ֵ����
  */
static void on_uart_received(uint8_t data)
{
    // û���������󣬲���������
    if (!rxready)
    {
        return;
    }

    // �������ݣ���ֹ���������
    if (rxlen < RX_BUFFER_SIZE)
    {
        rxdata[rxlen++] = data;            // �洢���յ�������
    }
    else
    {
        rxresult = RX_RESULT_FAIL;         // ��������������Ϊʧ��
        rxready = 0;                       // �رս��ձ�־
        return;
    }

    // ���ݽ�������ж�
    if (data == '\n')                      // ������յ����з�
    {
        // �жϽ��ջ��з��Ƿ�Ϊ\r\n
        if (rxlen >= 2 && rxdata[rxlen - 2] == '\r')
        {
            // �յ�OK
            if (rxlen >= 4 && 
                rxdata[rxlen - 4] == 'O' && rxdata[rxlen - 3] == 'K')
            {
                rxresult = RX_RESULT_OK;   // ���յ�OK��Ӧ�����Ϊ�ɹ�
                rxready = 0;               // �رս��ձ�־
            }
            // �յ�ERROR
            else if (rxlen >= 7 && 
                     rxdata[rxlen - 7] == 'E' && rxdata[rxlen - 6] == 'R' && 
                     rxdata[rxlen - 5] == 'R' && rxdata[rxlen - 4] == 'O' && 
                     rxdata[rxlen - 3] == 'R')
            {
                rxresult = RX_RESULT_ERROR; // ���յ�ERROR��Ӧ�����Ϊ����
                rxready = 0;                // �رս��ձ�־
            }
        }
    }
}

/**
  * ��    ����ESP ATģ���ʼ��
  * ��    ������
  * �� �� ֵ��0-ʧ�ܣ�1-�ɹ�
  */
uint8_t esp_at_init(void)
{
    rxready = 0;                           // ��ս��ձ�־

    ESP_UART_Init();                       // ��ʼ��UART
    ESP_UART_ReceiveRegister(on_uart_received);  // ע����ջص�����

    return esp_at_reset();                 // ���ø�λ����
}

/**
  * ��    ��������AT����ȴ���Ӧ
  * ��    ����cmd Ҫ���͵�AT�����ַ���
  * ��    ����rsp ��Ӧ���ݵ�ָ�룬ʹ�������������ʽ����
  * ��    ����length ��Ӧ���ݳ��ȣ�ʹ�������������ʽ����
  * ��    ����timeout �ȴ���ʱʱ�䣬��λ����
  * �� �� ֵ��0-ʧ�ܣ�1-�ɹ�
  */
uint8_t esp_at_send_command(const char *cmd, const char **rsp, uint32_t *length, uint32_t timeout)
{
    uint32_t i;
    
    // ��ս��ջ�����
    rxlen = 0;                             // ��ս��ճ���
    rxready = 1;                           // ���ý��վ�����־
    rxresult = RX_RESULT_FAIL;             // Ĭ�Ͻ��Ϊʧ��

    // ����AT����
    for (i = 0; i < strlen(cmd); i++)
    {
        ESP_UART_SendByte(cmd[i]);         // ���ֽڷ�������
    }
    // ���ͻس�����
    ESP_UART_SendByte('\r');               // ���ͻس���
    ESP_UART_SendByte('\n');               // ���ͻ��з�

    // �ȴ���Ӧ��ʱ
    while (rxready && timeout--)
    {
        Delay_ms(1);                       // ��ʱ1����
    }
    rxready = 0;                           // ������ձ�־

    // ������Ӧ����
    if (rsp)
    {
        *rsp = (const char *)rxdata;       // ������Ӧ����ָ��
    }
    if (length)
    {
        *length = rxlen;                   // ������Ӧ����
    }

    return rxresult == RX_RESULT_OK;       // ������ΪOK�򷵻�1�����򷵻�0
}

/**
  * ��    ����ͨ��ESP ATģ�鷢������
  * ��    ����data Ҫ���͵���������
  * ��    ����length Ҫ�������ݵĳ���
  * �� �� ֵ��0-ʧ�ܣ�1-�ɹ�
  */
uint8_t esp_at_send_data(const uint8_t *data, uint32_t length)
{
    uint32_t i;
    
    // ֱ�ӷ�������
    for (i = 0; i < length; i++)
    {
        ESP_UART_SendByte(data[i]);        // ���ֽڷ�������
    }

    return 1;                              // ���سɹ�
}

/**
  * ��    ��������ESP ATģ��
  * ��    ������
  * �� �� ֵ��0-ʧ�ܣ�1-�ɹ�
  */
uint8_t esp_at_reset(void)
{
    // ��λESP32
    if (!esp_at_send_command("AT+RESTORE", NULL, NULL, 1000))
    {
        return 0;                          // ���͸�λָ��ʧ�ܣ�����0
    }
    // �ȴ��������
    Delay_ms(2000);                        // ��ʱ2000���룬�ȴ�ģ������
    
    // �رջ���
    if (!esp_at_send_command("ATE0", NULL, NULL, 1000))
    {
        return 0;                          // �رջ���ʧ�ܣ�����0
    }
    
    // �ر����ô洢
    if (!esp_at_send_command("AT+SYSSTORE=0", NULL, NULL, 1000))
    {
        return 0;                          // �ر����ô洢ʧ�ܣ�����0
    }

    return 1;                              // ȫ������ɹ�������1
}

/**
  * ��    ������ʼ��WiFiģʽ
  * ��    ������
  * �� �� ֵ��0-ʧ�ܣ�1-�ɹ�
  */
uint8_t esp_at_wifi_init(void)
{
    // ����Ϊstationģʽ
    if (!esp_at_send_command("AT+CWMODE=1", NULL, NULL, 1000))
    {
        return 0;                          // ����ģʽʧ�ܣ�����0
    }

    return 1;                              // ���óɹ�������1
}

/**
  * ��    ��������WiFi����
  * ��    ����ssid WiFi����
  * ��    ����pwd WiFi����
  * �� �� ֵ��0-ʧ�ܣ�1-�ɹ�
  */
uint8_t esp_at_wifi_connect(const char *ssid, const char *pwd)
{
    char cmd[64];                          // �����������

    // ����WiFi�����������ַ���
    sprintf(cmd, "AT+CWJAP=\"%s\",\"%s\"", ssid, pwd);  // ��ʽ�������ַ���
    if (!esp_at_send_command(cmd, NULL, NULL, 10000))
    {
        return 0;                          // ����ʧ�ܣ�����0
    }

    return 1;                              // ���ӳɹ�������1
}

/**
  * ��    ����ִ��HTTP GET����
  * ��    ����url HTTP�����URL��ַ
  * ��    ����rsp ��Ӧ���ݵ�ָ�룬ʹ�������������ʽ����
  * ��    ����length ��Ӧ���ݳ��ȣ�ʹ�������������ʽ����
  * ��    ����timeout �ȴ���ʱʱ�䣬��λ����
  * �� �� ֵ��0-ʧ�ܣ�1-�ɹ�
  */
uint8_t esp_at_http_get(const char *url, const char **rsp, uint32_t *length, uint32_t timeout)
{
    char cmd[128];                         // �����������

    // ����HTTP GET����
    sprintf(cmd, "AT+HTTPCGET=\"%s\"", url);  // ��ʽ�������ַ���
    if (!esp_at_send_command(cmd, rsp, length, timeout))
    {
        return 0;                          // ����ʧ�ܣ�����0
    }

    return 1;                              // ����ɹ�������1
}

/**
  * ��    ������ʼ��SNTPʱ��ͬ��
  * ��    ������
  * �� �� ֵ��0-ʧ�ܣ�1-�ɹ�
  */
uint8_t esp_at_sntp_init(void)
{
    // ����SNTP������
    if (!esp_at_send_command("AT+CIPSNTPCFG=1,8,\"cn.ntp.org.cn\",\"ntp.sjtu.edu.cn\"", NULL, NULL, 1000))
    {
        return 0;                          // ����SNTP������ʧ�ܣ�����0
    }

    // ��ѯSNTPʱ�䣬ȷ�����óɹ�
    if (!esp_at_send_command("AT+CIPSNTPTIME?", NULL, NULL, 1000))
    {
        return 0;                          // ��ѯʱ��ʧ�ܣ�����0
    }

    return 1;                              // ���óɹ�������1
}

/**
  * ��    ������ȡ��ǰʱ���
  * ��    ����timestamp ʱ���ָ�룬ʹ�������������ʽ����
  * �� �� ֵ��0-ʧ�ܣ�1-�ɹ�
  */
uint8_t esp_at_time_get(uint32_t *timestamp)
{
    const char *rsp;                       // ��Ӧ����ָ��
    uint32_t length;                       // ��Ӧ���ݳ���

    // ��ѯϵͳʱ���
    if (!esp_at_send_command("AT+SYSTIMESTAMP?", &rsp, &length, 1000))
    {
        return 0;                          // ��ѯʧ�ܣ�����0
    }

    // ����ʱ�����Ӧ
    char *sts = strstr(rsp, "+SYSTIMESTAMP:"); // ����ʱ�����ʶ
    if (sts != NULL)
    {
        sts += strlen("+SYSTIMESTAMP:");   // ָ���ƶ���ʱ�����ֵ
        *timestamp = atoi(sts);            // ת��Ϊ����
        return 1;                          // �����ɹ�������1
    }

    return 0;                              // ����ʧ�ܣ�����0
}


/**
  * ��    ������ȡESP32��IP��ַ
  * ��    ����ip_str IP��ַ�ַ���������
  * ��    ����buf_size ��������С
  * �� �� ֵ��0-ʧ�ܣ�1-�ɹ�
  */
uint8_t esp_at_get_ip(char *ip_str, uint8_t buf_size)
{
    const char *rsp;                       // ��Ӧ����ָ��
    uint32_t length;                       // ��Ӧ���ݳ���

    // ��ѯIP��ַ
    if (!esp_at_send_command("AT+CIPSTA?", &rsp, &length, 1000))
    {
        return 0;                          // ��ѯʧ�ܣ�����0
    }

    // ����IP��ַ��Ӧ
    // ��Ӧ��ʽ: +CIPSTA:ip:"192.168.1.100"
    char *ip_start = strstr(rsp, "+CIPSTA:ip:\""); 
    if (ip_start != NULL)
    {
        ip_start += strlen("+CIPSTA:ip:\"");   // ָ���ƶ���IP��ַ��ʼ
        char *ip_end = strchr(ip_start, '\"'); // ���ҽ�������
        if (ip_end != NULL)
        {
            uint8_t ip_len = ip_end - ip_start;
            if (ip_len < buf_size)
            {
                strncpy(ip_str, ip_start, ip_len);
                ip_str[ip_len] = '\0';      // ����ַ���������
                return 1;                   // �����ɹ�������1
            }
        }
    }

    return 0;                              // ����ʧ�ܣ�����0
}

/**
  * ��    ������ȡWiFi����״̬
  * ��    ������
  * �� �� ֵ��0-δ���ӣ�1-������
  */
uint8_t esp_at_wifi_status(void)
{
    const char *rsp;                       // ��Ӧ����ָ��
    uint32_t length;                       // ��Ӧ���ݳ���

    // ��ѯWiFi״̬
    if (!esp_at_send_command("AT+CWJAP?", &rsp, &length, 1000))
    {
        return 0;                          // ��ѯʧ�ܣ�����0
    }

    // ����Ƿ����WiFi������Ϣ
    if (strstr(rsp, "+CWJAP:") != NULL)
    {
        return 1;                          // ������
    }

    return 0;                              // δ����
}

