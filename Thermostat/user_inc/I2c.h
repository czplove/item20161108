
#ifndef __TH23S_DRIVER_EEPROM_H
#define __TH23S_DRIVER_EEPROM_H

//-#include "All.h"

/*
I2C时钟的计算:
100 kHz(标准速度) = 10uS一次变化
*/

#define EEPROM_Block0_ADDRESS 0xA0

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void I2C_EE_Init(void);
// void I2C_EE_ByteWrite(u8* pBuffer, u8 WriteAddr);
// void I2C_EE_PageWrite(u8* pBuffer, u8 WriteAddr, u8 NumByteToWrite);
// void I2C_EE_BufferWrite(u8* pBuffer, u8 WriteAddr, u16 NumByteToWrite);
// void I2C_EE_BufferRead(u8* pBuffer, u8 ReadAddr, u16 NumByteToRead);
// void I2C_EE_WaitEepromStandbyState(void);

//-BOOL EEPROM_Write(WORD WriteAddr, BYTE *pBuffer, WORD NumByteToWrite);
//-BOOL EEPROM_Read(WORD ReadAddr, BYTE *pBuffer, WORD NumByteToRead);
// BOOL EEPROM_Write(WORD nAddr, BYTE *pData, WORD nLen);
// BOOL EEPROM_Read(WORD nAddr, BYTE *pData, WORD nLen);


//IO方向设置
/*
在输出模式下：ODR是数据输出寄存器，
但是在输入模式下，同时也用来配置上拉下拉设置。
在将端口设置为上拉/下拉输入的状况下，是由将GPIOx->ODR相应位设置为0或1来设置为下拉输入还是上拉输入的。
为什么端口设置为输入，然后还要对输出寄存器进行写操作,的原因就是为了配置上拉下拉
*/
#define I2C1_SDA_IN()  {GPIOA->CRH&=0x0FFFF0FFF;GPIOA->CRH|=(UINT32)8<<12;}	//-这里仅仅配置输入输出类型,上拉下拉还需要ODR配置
#define I2C1_SDA_OUT() {GPIOA->CRH&=0x0FFFF0FFF;GPIOA->CRH|=3<<12;}

#define I2C2_SDA_IN()  {GPIOB->CRL&=0x00FFFFFFF;GPIOB->CRL|=(UINT32)8<<28;}	//-这里仅仅配置输入输出类型,上拉下拉还需要ODR配置
#define I2C2_SDA_OUT() {GPIOB->CRL&=0x00FFFFFFF;GPIOB->CRL|=3<<28;}


#define I2C1_SCL_H         GPIOA->BSRR = GPIO_Pin_12
#define I2C1_SCL_L         GPIOA->BRR  = GPIO_Pin_12    
#define I2C1_SDA_H         GPIOA->BSRR = GPIO_Pin_11
#define I2C1_SDA_L         GPIOA->BRR  = GPIO_Pin_11
#define I2C1_SCL_read      (GPIOA->IDR  & GPIO_Pin_12)
#define I2C1_SDA_read      (GPIOA->IDR  & GPIO_Pin_11)

#define I2C2_SCL_H         GPIOB->BSRR = GPIO_Pin_6
#define I2C2_SCL_L         GPIOB->BRR  = GPIO_Pin_6
#define I2C2_SDA_H         GPIOB->BSRR = GPIO_Pin_7
#define I2C2_SDA_L         GPIOB->BRR  = GPIO_Pin_7
#define I2C2_SCL_read      (GPIOB->IDR  & GPIO_Pin_6)
#define I2C2_SDA_read      (GPIOB->IDR  & GPIO_Pin_7)
/*
#define I2C3_SCL_H         GPIOA->BSRR = GPIO_Pin_8
#define I2C3_SCL_L         GPIOA->BRR  = GPIO_Pin_8
#define I2C3_SDA_H         GPIOC->BSRR = GPIO_Pin_9
#define I2C3_SDA_L         GPIOC->BRR  = GPIO_Pin_9
#define I2C3_SCL_read      (GPIOA->IDR  & GPIO_Pin_8)
#define I2C3_SDA_read      (GPIOC->IDR  & GPIO_Pin_9)

#define I2C4_SCL_H         GPIOC->BSRR = GPIO_Pin_11
#define I2C4_SCL_L         GPIOC->BRR  = GPIO_Pin_11
#define I2C4_SDA_H         GPIOC->BSRR = GPIO_Pin_12
#define I2C4_SDA_L         GPIOC->BRR  = GPIO_Pin_12
#define I2C4_SCL_read      (GPIOC->IDR  & GPIO_Pin_11)
#define I2C4_SDA_read      (GPIOC->IDR  & GPIO_Pin_12)

#define I2C5_SCL_H         GPIOB->BSRR = GPIO_Pin_15
#define I2C5_SCL_L         GPIOB->BRR  = GPIO_Pin_15
#define I2C5_SDA_H         GPIOA->BSRR = GPIO_Pin_12
#define I2C5_SDA_L         GPIOA->BRR  = GPIO_Pin_12
#define I2C5_SCL_read      (GPIOB->IDR  & GPIO_Pin_15)
#define I2C5_SDA_read      (GPIOA->IDR  & GPIO_Pin_12)
*/
//-i2c发送或者接收一次数据都以数据包 struct i2c_msg 封装
struct i2c_msg 
{
     UINT16 addr;     // 从机地址 
     UINT16 flags;    // 标志 
 #define I2C_M_TEN   0x0010  // 十位地址标志 
 #define I2C_M_RD    0x0001  // 接收数据标志 
     UINT16 len;      // 数据长度 
     UINT8 *buf;      // 数据指针 
}; 

//-i2c从机信息
struct i2c_client 
{
     UINT16 addr;     // 从机地址 
     UINT16 flags;    // 标志 
     UINT8  num;			//-表示硬件端口号
};


int i2c_read_reg_Buffer(struct i2c_client *client,u8 numberOfBytes,u8 *data);
int i2c_write_reg_Buffer(struct i2c_client *client,u8 *writeBuffer,u8 numberOfBytes);
int i2c_write_data(struct i2c_client *client,u8 reg,u8 value);
int i2c_read_data_Buffer(struct i2c_client *client,u8 numberOfBytes,u8 *data);
int i2c_read_reg(struct i2c_client *client,u8 reg,u8 *data);

#endif
