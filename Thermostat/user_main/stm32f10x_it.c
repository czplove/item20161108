/******************** (C) COPYRIGHT 2010 STMicroelectronics ********************
* File Name          : stm32f10x_it.c
* Author             : MCD Application Team
* Version            : V3.1.1
* Date               : 04/07/2010
* Description        : Main Interrupt Service Routines.
*                      This file provides template for all exceptions handler
*                      and peripherals interrupt service routine.
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "stm32f10x_exti.h"
//-#include "usb_lib.h"
//-#include "usb_istr.h"
//-#include "usb_pwr.h"
//-#include "escore.h"

#include "user_conf.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
u8 Count1 = 0;
//-u8 Count2 = 0;

u8 USART1ReceiveBuffer[10] = {0};
u8 USART1MemoryBuffer[10] = {0};
u8 USART2ReceiveBuffer[30] = {0};
u8 USART2MemoryBuffer[3] = {0};
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

//-extern  u8 RxBuf[RX_BUFF_SIZE];
extern  u8 RXCount;
extern __IO uint32_t irq_flag;


//-extern __IO uint32_t f_rec ;
extern __IO uint8_t rx1_cnt;
extern uint8_t uart1_rx_buff[100];

extern void it_deal(void);
extern void UART_Rx_Deal(void);
extern void UART_Tx_Deal(void);
extern void UART3_Rx_Deal(void);
extern void UART3_Tx_Deal(void);
extern void Noise_Process_it(void);
/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/*******************************************************************************
* Function Name  : NMI_Handler
* Description    : This function handles NMI exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void NMI_Handler(void)
{
}

/*******************************************************************************
* Function Name  : HardFault_Handler
* Description    : This function handles Hard Fault exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/*******************************************************************************
* Function Name  : MemManage_Handler
* Description    : This function handles Memory Manage exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/*******************************************************************************
* Function Name  : BusFault_Handler
* Description    : This function handles Bus Fault exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/*******************************************************************************
* Function Name  : UsageFault_Handler
* Description    : This function handles Usage Fault exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/*******************************************************************************
* Function Name  : SVC_Handler
* Description    : This function handles SVCall exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SVC_Handler(void)
{
}

/*******************************************************************************
* Function Name  : DebugMon_Handler
* Description    : This function handles Debug Monitor exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DebugMon_Handler(void)
{
}

/*******************************************************************************
* Function Name  : PendSV_Handler
* Description    : This function handles PendSVC exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PendSV_Handler(void)
{
}

/*******************************************************************************
* Function Name  : SysTick_Handler
* Description    : This function handles SysTick Handler.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SysTick_Handler(void)
{
   cticks_ms++;
   //-EEP_Data = cticks_ms;

   if((cticks_ms & 0x7ff)==0)
  	Time_2048ms_Counter++;

   //-it_deal();		//-需要中断内处理的内容放在这里进行

   //-UART_Rx_Deal();
   //-UART_Tx_Deal();

   //-UART3_Rx_Deal();
   //-UART3_Tx_Deal();
}

void USART1_IRQHandler(void)
{
  	//-u8 data;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{

		//RxBuf[RXCount++] = USART_ReceiveData(USART2);
		//-data = USART_ReceiveData(USART1);
		//-fifo_put(&data, 1);
	}
}

/**
* @fun    void USART2_IRQHandler
* @brief
*         2015/12/23 星期三,Administrator
* @param  None
*
* @retval
*/
void USART2_IRQHandler(void)  //-这里后期需要可靠处理
{
  //-int ushTemp;

  if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
  {
    USART2ReceiveBuffer[Touch_Count2] = USART_ReceiveData(USART2);
    Touch_Count2++;
    if(Touch_Count2 >= 30)
      Touch_Count2 = 0;
    STOP_status2 = 1; //-点阵屏显示
    STOP_wait_time2 = 0;
    Touch_wait_time = cticks_5ms;
    STOP_wait_time = cticks_5ms;
    //-下面的清除不需要,上面读数据已经做了
    //-USART_ClearFlag(USART2, USART_IT_RXNE);
    //-USART_ClearITPendingBit(USART2, USART_IT_RXNE);
  }
/*
  if(USART_GetFlagStatus(USART2, USART_FLAG_ORE) != RESET)
  {//同  @arg USART_IT_ORE_ER : OverRun Error interrupt if the EIE bit is set
    ushTemp = USART_ReceiveData(USART2); //取出来扔掉
    USART_ClearFlag(USART2, USART_FLAG_ORE);
  }

  if(USART_GetFlagStatus(USART2, USART_FLAG_NE) != RESET)
  {//同  @arg USART_IT_NE     : Noise Error interrupt
    USART_ClearFlag(USART2, USART_FLAG_NE);
  }

  if(USART_GetFlagStatus(USART2, USART_FLAG_FE) != RESET)
  {//同   @arg USART_IT_FE     : Framing Error interrupt
      USART_ClearFlag(USART2, USART_FLAG_FE);
  }

  if(USART_GetFlagStatus(USART2, USART_FLAG_PE) != RESET)
  {//同  @arg USART_IT_PE     : Parity Error interrupt
    USART_ClearFlag(USART2, USART_FLAG_PE);
  }
  */
}

void USART3_IRQHandler(void)
{

	//  unsigned int i;
  if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
  {

    uart1_rx_buff[rx1_cnt++] = USART_ReceiveData(USART3);

    if(uart1_rx_buff[rx1_cnt-2]==0x0d&&uart1_rx_buff[rx1_cnt-1]==0x0a)
    {
	  f_rec=1;
	  rx1_cnt=0;

    }
    if(rx1_cnt >= 100)
      rx1_cnt = 0;

  }

  if(USART_GetITStatus(USART3, USART_IT_TXE) != RESET)
  {
     USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
  }

}

/**
  * @fun    void EXTI0_IRQHandler
  * @brief  按键中断处理函数
  * @author huangzibo
  * @param  None
  *
  * @retval
  */
void EXTI3_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line3) != RESET)
  {//-这里应该是仅仅起到唤醒的目的实际代码还是在主程序中做,这样防止中断嵌套
    EXTI_to_uart2();
    EXTI_ClearITPendingBit(EXTI_Line3);//清除EXTI line3中断标志
    STOP_wait_time = cticks_5ms;
    Touch_wait_time = cticks_5ms;
    STOP_status2 = 1;
    STOP_wait_time2 = 0;
    //-测试用
    test_pt++;
    if(test_pt > 1)
    {
        test_pt = 0x55;
    }
  }
}

/**
  * @fun    void EXTI0_IRQHandler
  * @brief  按键中断处理函数
  * @author huangzibo
  * @param  None
  *
  * @retval
  */
void EXTI4_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line4) != RESET)
  {//-这里应该是仅仅起到唤醒的目的实际代码还是在主程序中做,这样防止中断嵌套

    EXTI_ClearITPendingBit(EXTI_Line4);//清除EXTI line3中断标志

  }
}

void EXTI15_10_IRQHandler(void)
{

    if(EXTI_GetITStatus(EXTI_Line12) != RESET)
    {

      STOP_status2 = 1;
      STOP_wait_time2 = 0;
      STOP_wait_time = cticks_5ms;
      Standby_status = 0; //-从待机模式唤醒
      
     	EXTI_ClearITPendingBit(EXTI_Line12);
    }

    if(EXTI_GetITStatus(EXTI_Line10) != RESET)
    {
      EXTI_to_uart1();
      EXTI_ClearITPendingBit(EXTI_Line10);
    }

    if(EXTI_GetITStatus(EXTI_Line11) != RESET)
    {
      EXTI_to_uart3();
      EXTI_ClearITPendingBit(EXTI_Line11);
    }
 }

void DMA1_Channel1_IRQHandler(void) //-这里最大的问题就是占用中断时间太长
{
	UINT16 x,y;

  if(DMA_GetITStatus(DMA1_IT_HT1))
  {//-每1mS的时间产生一次中断,那么每次中断处理时间必须小于1mS,这样才不会发生中断嵌套
        /* A相采样数据完成 */
        ADC_Conv_flag = TRUE;

        //-测试用
        //-if(voc_rd_flag == 0)
        {
          //-EEP_Data++;
          for(x = 0;x < ADC_Channel_num;x++)
          {
             for(y = 0;y < (maxbuffer/ADC_Channel_num);y++)
               samp_data_ram_base[x][y] = ADC_ConvertedValue[x + ADC_Channel_num * y];		//-数据在内部是连续排列的,这里按照频道分开
          }

          //-Noise_Process_it();
          get_ntc_adc_value();
        }
        DMA_ClearITPendingBit(DMA1_IT_HT1);
  }

}
extern u8 LineNumber; 
/*******************************************************************************
* Function Name  : TIM2_IRQHandler
* Description    : This function handles TIM2 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM2_IRQHandler(void)  //-2015/6/20 20:45:19目前每5mS刷一次全屏
{
  static UINT8 dis_cn;

	//-UINT32	*led_display_data_pt;
  //-2015/7/20 20:45:51 以前的方法对于每行点亮的时间是采用死等的办法,这样占用了大量的CPU资源
  //-现在采用点完一行之后立即释放CPU,提高CPU的工作效率
  if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
  {
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    cticks_5ms++;
    KEY_read();

    if(dis_cn&0x01)
      led_display_hang_it();
    else 
    {
      Display(LineNumber,0);
    //-  
    //-  GPIO_WriteBit(GPIOA,GPIO_Pin_7,1);
    //-  if(dis_cn==0x0a)
    //-    dis_cn = 0;
    }
 
    dis_cn++;

    UART1_Rx_Deal();
    UART1_Tx_Deal();

    UART2_Rx_Deal();
    //-UART2_Tx_Deal();

    //-UART3_Rx_Deal();
    //-UART3_Tx_Deal();
  }
}


void RTC_IRQHandler(void) //RTC的中断服务程序
{
    if (RTC_GetITStatus(RTC_IT_SEC) != RESET)
    {
        RTC_ClearITPendingBit(RTC_IT_SEC); //清除RTC的中断待处理位

    }

    if (RTC_GetITStatus(RTC_IT_OW) != RESET)
    {
        RTC_ClearITPendingBit(RTC_IT_OW); //清除RTC的中断待处理位

    }

    if (RTC_GetITStatus(RTC_IT_ALR) != RESET)
    {
        RTC_ClearITPendingBit(RTC_IT_ALR); //清除RTC的中断待处理位

    }
}

void RTCAlarm_IRQHandler(void)  //-RTC_IRQHandler是RTC中断,而这里是外部中断,一次闹钟同时使用了两个中断
{
  if(RTC_GetITStatus(RTC_IT_ALR) != RESET)
  {
    //-RTC_ClearFlag(RTC_FLAG_SEC);

    EXTI_ClearITPendingBit(EXTI_Line17);

    /* Check if the Wake-Up flag is set */
    if(PWR_GetFlagStatus(PWR_FLAG_WU) != RESET)
    {
      /* Clear Wake Up flag */
      PWR_ClearFlag(PWR_FLAG_WU);
    }

    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();
    /* Clear RTC Alarm interrupt pending bit */
    RTC_ClearITPendingBit(RTC_IT_ALR);
    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();
  }

  if (RTC_GetITStatus(RTC_IT_SEC) != RESET)
    {
        RTC_ClearITPendingBit(RTC_IT_SEC); //清除RTC的中断待处理位

    }

    if (RTC_GetITStatus(RTC_IT_OW) != RESET)
    {
        RTC_ClearITPendingBit(RTC_IT_OW); //清除RTC的中断待处理位

    }

}

/******************************************************************************/
/*            STM32F10x Peripherals Interrupt Handlers                        */
/******************************************************************************/

#ifndef STM32F10X_CL
/*******************************************************************************
* Function Name  : USB_HP_CAN1_TX_IRQHandler
* Description    : This function handles USB High Priority or CAN TX interrupts requests
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USB_HP_CAN1_TX_IRQHandler(void)
{
  //-CTR_HP();
}

/*******************************************************************************
* Function Name  : USB_LP_CAN1_RX0_IRQHandler
* Description    : This function handles USB Low Priority or CAN RX0 interrupts
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USB_LP_CAN1_RX0_IRQHandler(void)
{
  //-USB_Istr();
}
#endif /* STM32F10X_CL */

#ifdef STM32F10X_HD
/*******************************************************************************
* Function Name  : SDIO_IRQHandler
* Description    : This function handles SDIO global interrupt request.
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SDIO_IRQHandler(void)
{

}
#endif /* STM32F10X_HD */


/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/*******************************************************************************
* Function Name  : PPP_IRQHandler
* Description    : This function handles PPP interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
/*void PPP_IRQHandler(void)
{
}*/

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
