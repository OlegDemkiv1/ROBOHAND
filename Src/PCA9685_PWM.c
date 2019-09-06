#include "PCA9685_PWM.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx.h"
#include "stm32f1xx_it.h"
#include "main.h"
#include <string.h>

extern UART_HandleTypeDef huart1;
extern I2C_HandleTypeDef hi2c2;


//pca9586_drv.c
//#include "pca9685_drv.h"
 
HAL_StatusTypeDef pca_init(PCA9685_DRV* self, I2C_HandleTypeDef* bus, uint16_t dev_addr)
{
    self->hi2c2 = hi2c2;
    self->address = dev_addr << 1;
    return HAL_OK;
}
 
HAL_StatusTypeDef pca_write_block(PCA9685_DRV* self, uint16_t start, uint16_t count, uint8_t * buff, uint32_t timeout)
{
    return HAL_I2C_Mem_Write(
            &hi2c2, self->address,
            start, I2C_MEMADD_SIZE_8BIT,
            buff, count,
            timeout);
}
 
HAL_StatusTypeDef pca_read_block(PCA9685_DRV* self, uint16_t start, uint16_t count, uint8_t* buff, uint32_t timeout) {
    return HAL_I2C_Mem_Read(
            &hi2c2, self->address,
            start, I2C_MEMADD_SIZE_8BIT,
            buff, count,
            timeout);
}
 
HAL_StatusTypeDef pca_get_reg(PCA9685_DRV* self, uint16_t reg_no, uint8_t* value)
{
    return pca_read_block(self, reg_no, 1, value, HAL_MAX_DELAY);
}
 
HAL_StatusTypeDef pca_set_reg(PCA9685_DRV* self, uint16_t reg_no, uint8_t  value)
{
    return pca_write_block(self, reg_no, 1, &value, HAL_MAX_DELAY);
}
 
HAL_StatusTypeDef pca_set_pwm(PCA9685_DRV* self, uint8_t pwm_no, uint16_t pwm_on, uint16_t pwm_off)
{
    uint8_t mode1_save;
 
    // ????????? ????? ?????????????? ????????, ???????? MODE1
    pca_get_reg(self, PCA9685_REG_MODE1, &mode1_save);
    pca_set_reg(self, PCA9685_REG_MODE1, mode1_save | PCA9685_MODE1_AI);
    {
        // ???????? ?????? ??? ????????? ???
        uint16_t pwm_addr;
        uint8_t pwm_cfg[PWM_REG_SIZE];
 
        pwm_addr = PWM_REG_BASE + PWM_REG_SIZE * pwm_no;
        pwm_cfg[PWM_ON_L ] = (pwm_on  >> 0) & 0xFF;
        pwm_cfg[PWM_OFF_L] = (pwm_off >> 0) & 0xFF;
        // !!!! ??????? 12 ???
        pwm_cfg[PWM_ON_H ] = (pwm_on  >> 8) & 0x0F;
        pwm_cfg[PWM_OFF_H] = (pwm_off >> 8) & 0x0F;
 
        // ???????? ?????????
        pca_write_block(self, pwm_addr, PWM_REG_SIZE, pwm_cfg, HAL_MAX_DELAY);
    }
    // ??????????? MODE1
    pca_set_reg(self, PCA9685_REG_MODE1, mode1_save);
 
    //FIXME: ????? ????????? ??????
    return HAL_OK;
}




void init_PCA9685(PCA9685_DRV* dev) 
{
	  uint8_t PRE_SCALE_50_HZ=0x79;
    #define DEVICE_FOUND  0
    uint16_t STATUS = 1; // Status connect to device (if STATUS==0 - device if found, if STATUS==1 - device if not found)
    uint8_t size = 0;
    char str3[35] = { 0 };
    uint8_t size_mas = 0;
    uint8_t buff = 0;                                     // data for receive
 
    pca_init(dev, &hi2c2, PCA9685_ADDR_DEFAULT);
 
    // Read MODE1 register
    STATUS = pca_get_reg(dev, PCA9685_REG_MODE1, &buff);
 
    if (STATUS == DEVICE_FOUND)                           // if devsice is found
    {
        // Print mesage
        sprintf(str3, "CONNECT TO PCA9685 MODE1: %x\r\n", buff); // convert   in  str
        size = sizeof(str3);
        HAL_UART_Transmit(&huart1, (uint8_t *) str3, size, 0xFFFF);
 
        // Set frequency in 50Hz
        pca_set_reg(dev, PCA9685_REG_PRE_SCALE, PRE_SCALE_50_HZ);
 
        // Go out from SLEEP mode
        pca_set_reg(dev, PCA9685_REG_MODE1, buff & (~PCA9685_MODE1_SLEEP));
 
        HAL_Delay(10);
 
        // Read reg MODE1 after change
        pca_get_reg(dev, PCA9685_REG_MODE1, &buff);
 
        memset(str3, 0, sizeof(str3));
        sprintf(str3, "MODE1 AFTER: %x\r\n", buff);        // convert   in  str
        size = sizeof(str3);
        HAL_UART_Transmit(&huart1, (uint8_t *) str3, size, 0xFFFF);
 
        // Set test frequency
        uint16_t OnTime=205;              // 0,001 sec
        uint16_t OffTime=3890;
 
        pca_set_pwm(dev, 0, OnTime, OffTime);
 
        HAL_Delay(10);
 
    }
 
    else 
		{
        sprintf(str3, "ERROR CONNECT TO PCA9685 !!! \r\n"); // convert   in  str
        size = sizeof(str3);
        HAL_UART_Transmit(&huart1, (uint8_t *) str3, size, 0xFFFF);
 
    }
}


