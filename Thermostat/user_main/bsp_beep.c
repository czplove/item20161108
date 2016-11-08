/**
  ******************************************************************************
  * @file    bsp_beep.c
  * @author  Wireless Protocol Stack Develop-Dept.
  * @version --
  * @date    2016/2/16 ���ڶ�,Administrator
  * @brief  This file
  *
  * @verbatim
  *
  ******************************************************************************
  * @attention
  *
  *
  *
  * <h2><center>&copy; COPYRIGHT 2015 WuLianGroup</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
//-#include "stm32f10x.h"
//-#include "bsp_beep.h"
//-#include "bsp_timer.h"
#include "user_conf.h"
/* Private typedef -----------------------------------------------------------*/

//#define Beep(BitVal)  		    GPIO_WriteBit(GPIOB,GPIO_Pin_6,BitVal)
#define Motor(BitVal)  		    GPIO_WriteBit(GPIOC,GPIO_Pin_2,BitVal)

#define beep    0x01
#define motor   0x02

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
//��ʱ��ʱ��ԴFck_cnt = 72000000/(1+psc)=1000000Hz
//��ʱ��Ƶ��(freq) = 72000000/((1+psc)*(arr+1)) = 1000000/2000=500Hz
//ռ�ձ�(duty) = compare/(arr+1) = 1000/2000 = 50%
uint16_t beep_arr = 1999;//������pwm������ֵ
uint16_t beep_psc = 71;//������pwm��Ԥ��Ƶֵ��
uint16_t beep_compare = 1000;//������pwm�ıȽ�ֵ������ռ�ձȣ�

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void BEEP_Off(void);
void BEEP_SetFreq(u16 freq);
void BEEP_SetDuty(u8 duty);


/**
  * @fun    void bsp_InitBeep
  * @brief  ��ʼ��������
  * @author huangzibo
  * @param  None
  *
  * @retval
  */
void bsp_InitMotor(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE); //�����˿�ʱ��

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

}

/**
  * @fun    void  BeepMotorStatus
  * @brief  �������״̬
  * @author huangzibo
  * @param  uint8 MOTOR
  *
  * @retval
  */
void MotorStatus(u8 MOTOR)
{
  if((MOTOR == motor) && (Motor_onoff == 1))
    Motor((BitAction)1);
  else//- if(MOTOR == 0x00) 
    Motor((BitAction)0);
}

/**
  * @brief ��ʼ��������
  * @param none
  * @retval none
  * @note
  *    ��TIM4_CH3��Ϊ������PWM����
  *    Ƶ�ʷ�Χ��: 1MHz~15Hz����ʼ����Ƶ����500Hz
	*		 ��ʼ����ռ�ձ���: 0%����˷�����������
  */
void BEEP_Init(void)
{
	//-TIM4_PWM_Init(beep_arr,beep_psc);
	BEEP_Off();
}

/**
  * @brief �رշ�����
  * @param none
  * @retval none
  * @note
  *    ��PWMռ�ձ�����Ϊ0%
  */
void BEEP_Off(void)
{
  BEEP_SetDuty(0);
}

/**
  * @brief �򿪷�����
  * @param none
  * @retval none
  * @note
  *   ��PWMƵ������Ϊ500Hz
	*		��ռ�ձ�����Ϊ50%
  */
void BEEP_On(void)
{
  //-BeepTime = 0;
  if(beep_onoff == 1)
  {
    BEEP_SetFreq(5000);
    BEEP_SetDuty(80);
  }
}

/**
  * @brief ���÷�����PWMƵ��
  * @param freq : ���õ�Ƶ�ʣ���λHz����Χ(15 ~ 65535)
  * @retval none
  * @note
  *    ��ʱ��ʱ��ԴƵ�� Fck_cnt = PCLK/(psc+1) = 1MHz
  *    Ƶ�� Freq = PCLK/((arr+1)*(psc+1))
  */
void BEEP_SetFreq(u16 freq)
{
	if(freq<15)return;//�����õ����Ƶ�ʱ������15

	beep_arr = 24000000/((1+beep_psc)*freq)-1;//����Ƶ�ʼ�������ֵARR
	beep_compare = (beep_arr+1)/2;	//��ռ�ձ�����Ϊ50%

	TIM4->ARR = beep_arr;//��������ֵ
	TIM4->CCR3 = beep_compare;//���ñȽ�ֵ
//    TIM4->CCR1 = beep_compare;//���ñȽ�ֵ
}

/**
  * @brief ��ȡ��ǰ������PWMƵ��
  * @param none
  * @retval ��ǰPWM��Ƶ��
  * @note
  *    ��ʱ������Ƶ�� = PCLK / ( psc + 1 )
  *    ��ʱ���ж����� = ( arr + 1 )*( psc + 1) / PCLK
  */
uint16_t BEEP_GetFreq(void)
{
	uint16_t freq;
	freq = 24000000/((beep_arr+1)*(beep_psc+1));
	return freq;
}

/**
  * @brief ���÷�����PWMռ�ձ�
  * @param duty : ռ�ձȣ���50%�������Ϊ50
  * @retval none
  * @note
	* 		duty = 100*CCR3/(ARR+1)
  */
void BEEP_SetDuty(u8 duty)
{
	beep_compare = duty*(beep_arr+1)/100;//����Ƚ�ֵ
	TIM4->CCR3 = beep_compare;//���±Ƚ�ֵ
}

/**
  * @brief ��ȡ��ǰ������PWMռ�ձ�
  * @param none
  * @retval ��ǰPWM��ռ�ձ�
  * @note
	* 		duty = 100*CCR3/(ARR+1)
  */
uint8_t BEEP_GetDuty(void)
{
	uint16_t duty;
	duty = 100*beep_compare/(beep_arr+1);
	return duty;
}



/******************* (C) COPYRIGHT 2015 WuLianGroup ********END OF FIL*********/

