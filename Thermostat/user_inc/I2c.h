
#ifndef __TH23S_DRIVER_EEPROM_H
#define __TH23S_DRIVER_EEPROM_H

//-#include "All.h"

/*
I2Cʱ�ӵļ���:
100 kHz(��׼�ٶ�) = 10uSһ�α仯
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


//IO��������
/*
�����ģʽ�£�ODR����������Ĵ�����
����������ģʽ�£�ͬʱҲ�������������������á�
�ڽ��˿�����Ϊ����/���������״���£����ɽ�GPIOx->ODR��Ӧλ����Ϊ0��1������Ϊ�������뻹����������ġ�
Ϊʲô�˿�����Ϊ���룬Ȼ��Ҫ������Ĵ�������д����,��ԭ�����Ϊ��������������
*/
#define I2C1_SDA_IN()  {GPIOA->CRH&=0x0FFFF0FFF;GPIOA->CRH|=(UINT32)8<<12;}	//-����������������������,������������ҪODR����
#define I2C1_SDA_OUT() {GPIOA->CRH&=0x0FFFF0FFF;GPIOA->CRH|=3<<12;}

#define I2C2_SDA_IN()  {GPIOB->CRL&=0x00FFFFFFF;GPIOB->CRL|=(UINT32)8<<28;}	//-����������������������,������������ҪODR����
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
//-i2c���ͻ��߽���һ�����ݶ������ݰ� struct i2c_msg ��װ
struct i2c_msg 
{
     UINT16 addr;     // �ӻ���ַ 
     UINT16 flags;    // ��־ 
 #define I2C_M_TEN   0x0010  // ʮλ��ַ��־ 
 #define I2C_M_RD    0x0001  // �������ݱ�־ 
     UINT16 len;      // ���ݳ��� 
     UINT8 *buf;      // ����ָ�� 
}; 

//-i2c�ӻ���Ϣ
struct i2c_client 
{
     UINT16 addr;     // �ӻ���ַ 
     UINT16 flags;    // ��־ 
     UINT8  num;			//-��ʾӲ���˿ں�
};


int i2c_read_reg_Buffer(struct i2c_client *client,u8 numberOfBytes,u8 *data);
int i2c_write_reg_Buffer(struct i2c_client *client,u8 *writeBuffer,u8 numberOfBytes);
int i2c_write_data(struct i2c_client *client,u8 reg,u8 value);
int i2c_read_data_Buffer(struct i2c_client *client,u8 numberOfBytes,u8 *data);
int i2c_read_reg(struct i2c_client *client,u8 reg,u8 *data);

#endif
