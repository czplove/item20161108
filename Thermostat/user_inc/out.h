/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : lcd.h
* Author             : David JIANG
* Date First Issued  : 2007.9.17
* Description        : LCD functions
*
********************************************************************************
* History:
* 2007.9.17 : Version 1.0
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __OUT_H
#define __OUT_H

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/


/* Exported constants --------------------------------------------------------*/
#define OUT_U4_M  		(3<<12)
#define OUT_U5_M  		(3<<10)
#define OUT_U7_M  		(3<<8)
#define OUT_U8_M  		(3<<6)
#define OUT_U9_M  		(3<<4)
#define OUT_U11_M  		(3<<2)
#define OUT_U12_M  		(3<<0)

#define OUT_U4_H  		(2<<12)
#define OUT_U5_H  		(2<<10)
#define OUT_U7_H  		(2<<8)
#define OUT_U8_H  		(2<<6)
#define OUT_U9_H  		(2<<4)
#define OUT_U11_H  		(2<<2)
#define OUT_U12_H  		(2<<0)

#define OUT_U4_D  		(1<<12)
#define OUT_U5_D  		(1<<10)
#define OUT_U7_D  		(1<<8)
#define OUT_U8_D  		(1<<6)
#define OUT_U9_D  		(1<<4)
#define OUT_U11_D  		(1<<2)
#define OUT_U12_D  		(1<<0)

#define OUT_HEAT_one(expr)  ((expr) ? OUT_U11_H : OUT_U11_D)  //-一级制热
#define OUT_HEAT_two(expr)  ((expr) ? OUT_U12_H : OUT_U12_D)  //-二级制热
#define OUT_NOP_one(expr)   ((expr) ? OUT_U9_H : OUT_U9_D)  //-预留端子
#define OUT_COOL_one(expr)  ((expr) ? OUT_U4_H : OUT_U4_D)  //-一级制冷
#define OUT_COOL_two(expr)  ((expr) ? OUT_U5_H : OUT_U5_D)  //-二级制冷
#define OUT_wind(expr)      ((expr) ? OUT_U7_H : OUT_U7_D)  //-风扇
#define OUT_NOP_two(expr)   ((expr) ? OUT_U8_H : OUT_U8_D)  //-预留端子

#define OUT_W1orAUX(expr)   ((expr) ? OUT_U11_H : OUT_U11_D)  //-一级制热
#define OUT_W2(expr)        ((expr) ? OUT_U12_H : OUT_U12_D)  //-二级制热
#define OUT_EorNOP(expr)    ((expr) ? OUT_U9_H : OUT_U9_D)  //-预留端子
#define OUT_Y1(expr)        ((expr) ? OUT_U4_H : OUT_U4_D)  //-一级制冷
#define OUT_Y2(expr)        ((expr) ? OUT_U5_H : OUT_U5_D)  //-二级制冷
#define OUT_G(expr)         ((expr) ? OUT_U7_H : OUT_U7_D)  //-风扇
#define OUT_OorB(expr)      ((expr) ? OUT_U8_H : OUT_U8_D)  //-预留端子

#define OUT_W1orAUX_M        OUT_U11_M
#define OUT_W2_M  		       OUT_U12_M
#define OUT_EorNOP_M  		   OUT_U9_M
#define OUT_Y1_M  		       OUT_U4_M
#define OUT_Y2_M  		       OUT_U5_M
#define OUT_G_M  		         OUT_U7_M
#define OUT_OorB_M  		     OUT_U8_M

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#endif

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/
