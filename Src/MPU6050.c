#include "MPU6050.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx.h"
#include "stm32f1xx_it.h"
#include "main.h"
#include "math.h"


extern UART_HandleTypeDef huart1;
extern I2C_HandleTypeDef hi2c1;



void init_MPU6050(void)
{
		// 1. Resd who am i reg
	  uint8_t sizebuf=1;
		uint16_t timeout=1000;
		char str1[40]={0};	
		uint8_t size=0;
		uint8_t who_i_am_buffer=0x00;
		uint8_t who_i_am=0x68;
		
		HAL_I2C_Mem_Read(&hi2c1, (uint16_t)I2C_address_MPU6050<<1,(uint16_t)WHO_AM_I_MPU6050, (uint16_t) sizebuf, &who_i_am_buffer, (uint16_t) sizebuf,(uint32_t) timeout);
	  if(who_i_am_buffer!=who_i_am)
		{
				sprintf(str1,"CONNECTION_TO_ADXL345_ERROR!!! \r\n");                		  					// convert   in  str 
				size=sizeof(str1);
				HAL_UART_Transmit(&huart1 , (uint8_t *)str1, size, 0xFFFF);     // send  new  line  in  com  port
				HAL_Delay(4000);	
		}
		else
		{
//				sprintf(str1,"CONNECTION_TO_ADXL345_OK  ID:%x \r\n", who_i_am_buffer);                		  						// convert  in  str 
//				size=sizeof(str1);
//				HAL_UART_Transmit(&huart1 , (uint8_t *)str1, size, 0xFFFF);     // send  new  line  in  com  port
//				HAL_Delay(200);	
		}
		
		// 2. Out from sleep mode
		uint8_t out_from_sleep=0x00;
		HAL_I2C_Mem_Write(&hi2c1, (uint16_t) I2C_address_MPU6050<<1, (uint16_t) PWR_MGMT_1, (uint16_t) 1, &out_from_sleep, (uint16_t) 1, (uint32_t) 1000);
		
		// Set asseleration on 4G mode 
		uint8_t SET_ACC_IN_4G=0x08;
		uint8_t SET_ACC_IN_8G=0x10;
		uint8_t SET_ACC_IN_16G=0x18;
		HAL_I2C_Mem_Write(&hi2c1, (uint16_t) I2C_address_MPU6050<<1, (uint16_t) ACCEL_CONFIG, (uint16_t) 1, &SET_ACC_IN_4G, (uint16_t) 1, (uint32_t) 1000);
	
}

void read_acc_MPU6050(void)
{
		uint8_t acc_buffer[6]={0};
		uint8_t size=0;
		
		HAL_I2C_Mem_Read(&hi2c1, (uint16_t)I2C_address_MPU6050<<1,(uint16_t)ACCEL_XOUT_H, (uint16_t) 1, acc_buffer, (uint16_t) 6,(uint32_t) 1000);
	  
		int16_t X_acc_value=(((uint16_t)acc_buffer[0]<<8)|acc_buffer[1]);
		int16_t Y_acc_value=(((uint16_t)acc_buffer[2]<<8)|acc_buffer[3]);
		int16_t Z_acc_value=(((uint16_t)acc_buffer[4]<<8)|acc_buffer[5]);
		
		// Convert data in G/s
		float X=(float)X_acc_value/8192.0;
		float Y=(float)Y_acc_value/8192.0;
		float Z=(float)Z_acc_value/8192.0;
		
//		char str[40]={0};
//		sprintf(str,"X:%.2f  Y:%.2f  Z:%.2f \r\n",X, Y, Z);                		  					// convert   in  str 
//		size=sizeof(str);
//		HAL_UART_Transmit(&huart1 , (uint8_t *)str, size, 0xFF);     // send  new  line  in  com  port	
		
		//Convert accelerometr data in angle
		// FORMULA ||| X_ANHLE=
		int16_t ACCEL_XANGLE = 57.295*atan((float)-X_acc_value/ sqrt(pow((float)Y_acc_value,2)+pow((float)Z_acc_value,2)));
		int16_t ACCEL_YANGLE = 57.295*atan((float)-Y_acc_value/ sqrt(pow((float)X_acc_value,2)+pow((float)Z_acc_value,2)));
		int16_t ACCEL_ZANGLE = 57.295*atan((float)-Z_acc_value/ sqrt(pow((float)X_acc_value,2)+pow((float)Y_acc_value,2)));
		
		char str1[100]={0};
		sprintf(str1,"X_acc_value: %d, Y_acc_value: %d, Z_acc_value: %d,\r\n",ACCEL_XANGLE, ACCEL_YANGLE, ACCEL_ZANGLE);                		  						// convert  in  str 
		size=sizeof(str1);
		HAL_UART_Transmit(&huart1 , (uint8_t *)str1, size, 0xFFFF);     // send  new  line  in  com  port
		
		////////////////////////////////////////////////////////////////////////////////////////////////\

}