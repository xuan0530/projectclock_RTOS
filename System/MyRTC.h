#ifndef __MYRTC_H
#define __MYRTC_H

extern uint16_t MyRTC_Time[];

void MyRTC_Init(void);
void MyRTC_SetTime(void);
void MyRTC_ReadTime(void);
/**
  * 函    数：使用ESP32获取的网络时间更新RTC
  * 参    数：无
  * 返 回 值：0-失败，1-成功
  */
uint8_t MyRTC_SyncFromESP32(void);

#endif
