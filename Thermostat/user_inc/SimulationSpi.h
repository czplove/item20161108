#ifndef __SIMULATIONSPI_H
#define __SIMULATIONSPI_H

//-#include "stm32f10x.h"
//-#include "bsp_delay.h"

#define Red     0x01
#define Green   0x02
#define Blue    0x03

UINT8 SimulateSPISendByte(UINT8 dt); //����ģ��SPI�����ֽں��� 

//void SendByte(uint32_t data,uint8_t length);
//void SendBit(uint8_t data);
void DisplayLed(UINT32 data,UINT8 ss);
void TurnOnAllLed(UINT8 LedClour);
void TurnOffAllLed(void);


#endif
