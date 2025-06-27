#include "stm32f10x.h"                  // Device header
#include "MyI2C.h"
#include "MPU6050_Reg.h"
#include "MPU6050.h"  
#include "Lcd_Driver.h" 
#include <stdlib.h>  

#define MPU6050_ADDRESS 0xD0

void MPU6050_WriteReg(uint8_t RegAddress,uint8_t Data)
{
	MyI2C_Start();
	MyI2C_SendByte(MPU6050_ADDRESS);
	MyI2C_ReceiveAck();
	MyI2C_SendByte(RegAddress);
	MyI2C_ReceiveAck();
	MyI2C_SendByte(Data);
	MyI2C_ReceiveAck();
	MyI2C_Stop();
}

uint8_t MPU6050_ReadReg(uint8_t  RegAddress)
{
	uint8_t Data;
	
	MyI2C_Start();
	MyI2C_SendByte(MPU6050_ADDRESS);
	MyI2C_ReceiveAck();
	MyI2C_SendByte(RegAddress);//先写入要读的寄存器地址，再读，重新指定读写位
	MyI2C_ReceiveAck();					//接收应答
	
	MyI2C_Start();
	MyI2C_SendByte(MPU6050_ADDRESS | 0x01);//最低位为1
	MyI2C_ReceiveAck();
	Data = MyI2C_ReceiveByte();
	MyI2C_SendAck(1);//想要继续读，就发送应答，不给置1
	MyI2C_Stop();
	
	return Data;
}

void MPU6050_Init(void)
{
	MyI2C_Init();
	MPU6050_WriteReg(MPU6050_PWR_MGMT_1,0x01);//设备不复位0，接触睡眠模式0，不循环模式0，无关位0，温度传感器不使能0，陀螺仪时钟001
	MPU6050_WriteReg(MPU6050_PWR_MGMT_2,0x00);
	MPU6050_WriteReg(MPU6050_SMPLRT_DIV,0x09);//十分频
	MPU6050_WriteReg(MPU6050_CONFIG,0x06);//110最平滑的滤波
	MPU6050_WriteReg(MPU6050_GYRO_CONFIG,0x18);
	MPU6050_WriteReg(MPU6050_ACCEL_CONFIG,0x18);//最大量程
	
}

void MPU6050_GetData(int16_t *AccX,int16_t *AccY,int16_t *AccZ,
	                     int16_t *GyroX,int16_t *GyroY,int16_t *GyroZ)//指针传多返回值
{
	uint8_t DataH,DataL;
	DataH = MPU6050_ReadReg(MPU6050_ACCEL_XOUT_H );
	DataL = MPU6050_ReadReg(MPU6050_ACCEL_XOUT_L );
	*AccX = (DataH << 8 ) | DataL;
	
	DataH = MPU6050_ReadReg(MPU6050_ACCEL_YOUT_H );
	DataL = MPU6050_ReadReg(MPU6050_ACCEL_YOUT_L );
	*AccY = (DataH << 8 ) | DataL;
	
	DataH = MPU6050_ReadReg(MPU6050_ACCEL_ZOUT_H );
	DataL = MPU6050_ReadReg(MPU6050_ACCEL_ZOUT_L );
	*AccZ = (DataH << 8 ) | DataL;
	
	DataH = MPU6050_ReadReg(MPU6050_GYRO_XOUT_H );
	DataL = MPU6050_ReadReg(MPU6050_GYRO_XOUT_L );
	*GyroX = (DataH << 8 ) | DataL;
	
	DataH = MPU6050_ReadReg(MPU6050_GYRO_YOUT_H );
	DataL = MPU6050_ReadReg(MPU6050_GYRO_YOUT_L );
	*GyroY = (DataH << 8 ) | DataL;
	
	DataH = MPU6050_ReadReg(MPU6050_GYRO_ZOUT_H );
	DataL = MPU6050_ReadReg(MPU6050_GYRO_ZOUT_L );
	*GyroZ = (DataH << 8 ) | DataL;
	
	
}

/**
 * 根据加速度计数据判断屏幕方向
 */
ScreenOrientation_t MPU6050_GetScreenOrientation(void)
{
    int16_t AccX, AccY, AccZ;
    int16_t GyroX, GyroY, GyroZ;
    
    // 获取传感器数据
    MPU6050_GetData(&AccX, &AccY, &AccZ, &GyroX, &GyroY, &GyroZ);
    
    // 根据您的实际测量数据设置阈值
    const int16_t Y_THRESHOLD_HIGH = 1500;   // Y轴高阈值，判断横屏
    const int16_t Y_THRESHOLD_LOW = -1500;   // Y轴低阈值，判断横屏
    const int16_t X_THRESHOLD_PORTRAIT = 300; // X轴阈值，区分正立和倒立
    
    // 首先判断是横屏还是竖屏（根据Y轴绝对值）
    if (AccY > Y_THRESHOLD_HIGH) {
        // Y轴正向大值 - 左横屏
        return SCREEN_LANDSCAPE_LEFT;
    }
    else if (AccY < Y_THRESHOLD_LOW) {
        // Y轴负向大值 - 右横屏  
        return SCREEN_LANDSCAPE_RIGHT;
    }
    else {
        // Y轴数值较小，是竖屏模式，通过X轴判断正立还是倒立
        if (AccX > X_THRESHOLD_PORTRAIT) {
            // X轴正向 - 倒立竖屏 (根据您的数据AX=+530)
            return SCREEN_PORTRAIT_INVERTED;
        }
        else {
            // X轴负向或小正值 - 正常竖屏 (根据您的数据AX=-200)
            return SCREEN_PORTRAIT;
        }
    }
}

/**
 * 设置LCD屏幕方向
 * 基于ST7735R控制器
 */
void LCD_SetOrientation(ScreenOrientation_t orientation)
{
    uint8_t madctl_value = 0x08;  // 默认值，基于您的初始化代码
    
    switch(orientation) {
        case SCREEN_PORTRAIT:
            // 正常竖屏模式 (0度)
            madctl_value = 0x08;  // MX=0, MY=0, MV=0, ML=0, RGB=1
            break;
            
        case SCREEN_LANDSCAPE_LEFT:
            // 左横屏模式 (逆时针90度)
            madctl_value = 0x68;  // MX=0, MY=1, MV=1, ML=0, RGB=1
            break;
            
        case SCREEN_PORTRAIT_INVERTED:
            // 倒立竖屏模式 (180度)
            madctl_value = 0xC8;  // MX=1, MY=1, MV=0, ML=0, RGB=1
            break;
            
        case SCREEN_LANDSCAPE_RIGHT:
            // 右横屏模式 (顺时针90度)
            madctl_value = 0xA8;  // MX=1, MY=0, MV=1, ML=0, RGB=1
            break;
    }
    
    // 写入Memory Access Control寄存器
    Lcd_WriteIndex(0x36);
    Lcd_WriteData(madctl_value);
}


