#ifndef _SW_I2C_H_
#define _SW_I2C_H_

/* includes */
#include "stm32g0xx_hal.h"

#define SW_I2C1		        1

#define SW_I2C1_SCL_PORT    GPIOD
#define SW_I2C1_SDA_PORT    GPIOD
#define SW_I2C1_SCL_PIN     GPIO_PIN_0
#define SW_I2C1_SDA_PIN     GPIO_PIN_1

#define SW_I2C_WAIT_TIME    25
#define I2C_READ            0x01
#define READ_CMD            1
#define WRITE_CMD           0

/* functions */
void SW_I2C_initial(void);
uint8_t SW_I2C_Read_8addr(uint8_t sel, uint8_t IICID, uint8_t regaddr, uint8_t *pdata, uint8_t rcnt);
uint8_t SW_I2C_Read_16addr(uint8_t sel, uint8_t IICID, uint16_t regaddr, uint8_t *pdata, uint8_t rcnt);
uint8_t SW_I2C_Write_8addr(uint8_t sel, uint8_t IICID, uint8_t regaddr, uint8_t *pdata, uint16_t rcnt);
uint8_t SW_I2C_Write_16addr(uint8_t sel, uint8_t IICID, uint16_t regaddr, uint8_t *pdata, uint8_t rcnt);
uint8_t SW_I2C_Check_SlaveAddr(uint8_t sel, uint8_t IICID);


#endif  /* __I2C_SW_H */
