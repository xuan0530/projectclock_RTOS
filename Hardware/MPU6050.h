#ifndef __MPU6050_H
#define __MPU6050_H


typedef enum {
    SCREEN_PORTRAIT = 0,      // 竖屏正向
    SCREEN_LANDSCAPE_LEFT,    // 横屏左转
    SCREEN_PORTRAIT_INVERTED, // 竖屏倒立
    SCREEN_LANDSCAPE_RIGHT    // 横屏右转
} ScreenOrientation_t;

// 函数声明
ScreenOrientation_t MPU6050_GetScreenOrientation(void);
void LCD_SetOrientation(ScreenOrientation_t orientation);


void MPU6050_WriteReg(uint8_t RegAddress,uint8_t Data);
uint8_t MPU6050_ReadReg(uint8_t  RegAddress);
void MPU6050_GetData(int16_t *AccX,int16_t *AccY,int16_t *AccZ,
	                     int16_t *GyroX,int16_t *GyroY,int16_t *GyroZ);//指针传多返回值
void MPU6050_Init(void);

#endif
