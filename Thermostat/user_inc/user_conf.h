/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : stm32f10x_conf.h
* Author             : MCD Application Team
* Version            : V2.0.3
* Date               : 09/22/2008
* Description        : Library configuration file.
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USER_CONF_H
#define __USER_CONF_H

/* Includes ------------------------------------------------------------------*/
//-#include "stm32f10x_lib.h"
#include "stm32f10x.h"
#include "integer.h"
#include "COMMON.h"
#include "GloblDef.h"
#include "COMUSE.h"
#include "I2c.h"
#include "temp_hum.h"
#include <math.h>

#include "SimulationSpi.h"
#include "menu.h"
#include "bsp_led.h"
#include "out.h"


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Uncomment the line below to compile the library in DEBUG mode, this will expanse
   the "assert_param" macro in the firmware library code (see "Exported macro"
   section below) */
/* #define DEBUG    1*/

#endif /* __USER_CONF_H */

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/
