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
	MyI2C_SendByte(RegAddress);//��д��Ҫ���ļĴ�����ַ���ٶ�������ָ����дλ
	MyI2C_ReceiveAck();					//����Ӧ��
	
	MyI2C_Start();
	MyI2C_SendByte(MPU6050_ADDRESS | 0x01);//���λΪ1
	MyI2C_ReceiveAck();
	Data = MyI2C_ReceiveByte();
	MyI2C_SendAck(1);//��Ҫ���������ͷ���Ӧ�𣬲�����1
	MyI2C_Stop();
	
	return Data;
}

void MPU6050_Init(void)
{
	MyI2C_Init();
	MPU6050_WriteReg(MPU6050_PWR_MGMT_1,0x01);//�豸����λ0���Ӵ�˯��ģʽ0����ѭ��ģʽ0���޹�λ0���¶ȴ�������ʹ��0��������ʱ��001
	MPU6050_WriteReg(MPU6050_PWR_MGMT_2,0x00);
	MPU6050_WriteReg(MPU6050_SMPLRT_DIV,0x09);//ʮ��Ƶ
	MPU6050_WriteReg(MPU6050_CONFIG,0x06);//110��ƽ�����˲�
	MPU6050_WriteReg(MPU6050_GYRO_CONFIG,0x18);
	MPU6050_WriteReg(MPU6050_ACCEL_CONFIG,0x18);//�������
	
}

void MPU6050_GetData(int16_t *AccX,int16_t *AccY,int16_t *AccZ,
	                     int16_t *GyroX,int16_t *GyroY,int16_t *GyroZ)//ָ�봫�෵��ֵ
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
 * ���ݼ��ٶȼ������ж���Ļ����
 */
ScreenOrientation_t MPU6050_GetScreenOrientation(void)
{
    int16_t AccX, AccY, AccZ;
    int16_t GyroX, GyroY, GyroZ;
    
    // ��ȡ����������
    MPU6050_GetData(&AccX, &AccY, &AccZ, &GyroX, &GyroY, &GyroZ);
    
    // ��������ʵ�ʲ�������������ֵ
    const int16_t Y_THRESHOLD_HIGH = 1500;   // Y�����ֵ���жϺ���
    const int16_t Y_THRESHOLD_LOW = -1500;   // Y�����ֵ���жϺ���
    const int16_t X_THRESHOLD_PORTRAIT = 300; // X����ֵ�����������͵���
    
    // �����ж��Ǻ�����������������Y�����ֵ��
    if (AccY > Y_THRESHOLD_HIGH) {
        // Y�������ֵ - �����
        return SCREEN_LANDSCAPE_LEFT;
    }
    else if (AccY < Y_THRESHOLD_LOW) {
        // Y�Ḻ���ֵ - �Һ���  
        return SCREEN_LANDSCAPE_RIGHT;
    }
    else {
        // Y����ֵ��С��������ģʽ��ͨ��X���ж��������ǵ���
        if (AccX > X_THRESHOLD_PORTRAIT) {
            // X������ - �������� (������������AX=+530)
            return SCREEN_PORTRAIT_INVERTED;
        }
        else {
            // X�Ḻ���С��ֵ - �������� (������������AX=-200)
            return SCREEN_PORTRAIT;
        }
    }
}

/**
 * ����LCD��Ļ����
 * ����ST7735R������
 */
void LCD_SetOrientation(ScreenOrientation_t orientation)
{
    uint8_t madctl_value = 0x08;  // Ĭ��ֵ���������ĳ�ʼ������
    
    switch(orientation) {
        case SCREEN_PORTRAIT:
            // ��������ģʽ (0��)
            madctl_value = 0x08;  // MX=0, MY=0, MV=0, ML=0, RGB=1
            break;
            
        case SCREEN_LANDSCAPE_LEFT:
            // �����ģʽ (��ʱ��90��)
            madctl_value = 0x68;  // MX=0, MY=1, MV=1, ML=0, RGB=1
            break;
            
        case SCREEN_PORTRAIT_INVERTED:
            // ��������ģʽ (180��)
            madctl_value = 0xC8;  // MX=1, MY=1, MV=0, ML=0, RGB=1
            break;
            
        case SCREEN_LANDSCAPE_RIGHT:
            // �Һ���ģʽ (˳ʱ��90��)
            madctl_value = 0xA8;  // MX=1, MY=0, MV=1, ML=0, RGB=1
            break;
    }
    
    // д��Memory Access Control�Ĵ���
    Lcd_WriteIndex(0x36);
    Lcd_WriteData(madctl_value);
}


