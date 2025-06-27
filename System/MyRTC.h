#ifndef __MYRTC_H
#define __MYRTC_H

extern uint16_t MyRTC_Time[];

void MyRTC_Init(void);
void MyRTC_SetTime(void);
void MyRTC_ReadTime(void);
/**
  * ��    ����ʹ��ESP32��ȡ������ʱ�����RTC
  * ��    ������
  * �� �� ֵ��0-ʧ�ܣ�1-�ɹ�
  */
uint8_t MyRTC_SyncFromESP32(void);

#endif
