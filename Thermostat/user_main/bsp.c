/*
	�Եײ��֧��
*/
#include "stm32f10x.h"
//-#include "demo.h"
//-#include "integer.h"
#include "user_conf.h"



void RCC_Configuration(void){

  SystemInit();   //-��Ƶ����56MHz


  //RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC
  						|RCC_APB2Periph_AFIO , ENABLE);

  //-������
  //-GPIO_InitTypeDef GPIO_InitStructure;
  //-GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7;
  //-GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  //-GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  //-//-RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
  //-GPIOC->BSRR = GPIO_Pin_6;
  //-GPIO_Init(GPIOC, &GPIO_InitStructure);
  //-end

  //-RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);
  //-RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
  /* Enable USART1, AFIO clocks */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_AFIO, ENABLE);
  /* Enable USART2, USART3 clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2 | RCC_APB1Periph_USART3, ENABLE);

  /* TIM2 clocks enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM3, ENABLE);

  //-RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);//ʹ��TIM1ʱ��

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);	//ʹ�ܶ�ʱ��4ʱ��

  /* Enable DMA clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

  /* Enable ADC1 clock */
  //-RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE );

  /* I2C1 clock enable */
  //-RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);   //-����ģ��I2C

#if SYS_iwatch_DOG
  /* Enable the LSI OSC */
  RCC_LSICmd(ENABLE);		//-the LSI OSC ���ڲ�����40KHz

  /* Wait till LSI is ready */
  while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
  {}

#endif

  /* Enable PWR and BKP clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

}

ErrorStatus HSEStartUpStatus;
/**
  * @brief  Configures system clock after wake-up from STOP: enable HSE, PLL
  *         and select PLL as system clock source.
  * @param  None
  * @retval None
  */
void SYSCLKConfig_STOP(void)
{
  /* Enable HSE */
  RCC_HSEConfig(RCC_HSE_ON);

  /* Wait till HSE is ready */
  HSEStartUpStatus = RCC_WaitForHSEStartUp();

  if(HSEStartUpStatus == SUCCESS)
  {

#ifdef STM32F10X_CL
    /* Enable PLL2 */
    RCC_PLL2Cmd(ENABLE);

    /* Wait till PLL2 is ready */
    while(RCC_GetFlagStatus(RCC_FLAG_PLL2RDY) == RESET)
    {
    }

#endif

    /* Enable PLL */
    RCC_PLLCmd(ENABLE);

    /* Wait till PLL is ready */
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
    {
    }

    /* Select PLL as system clock source */
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

    /* Wait till PLL is used as system clock source */
    while(RCC_GetSYSCLKSource() != 0x08)
    {
    }
  }
}

void iwdg_init(void)
{
//-�������Ź�
   /* IWDG timeout equal to 250 ms (the timeout may varies due to LSI frequency dispersion) */
  /* Enable write access to IWDG_PR and IWDG_RLR registers */
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
  /* IWDG counter clock: LSI/32 */
  IWDG_SetPrescaler(IWDG_Prescaler_256);
  /* Set counter reload value to obtain 250ms IWDG TimeOut.
     Counter Reload Value = 250ms/IWDG counter clock period
                          = 250ms / (LSI/32)
                          = 0.25s / (LsiFreq/32)
                          = LsiFreq/(32 * 4)
                          = LsiFreq/128
   */
  IWDG_SetReload(4063);	//-���������ֵ�����˿��Ź���λʱ��ÿ�ε�ʱ����6.4mS,�ܴ���������ʱ��:100*6.4=80mS,��ô�����ʱ���ڱ���ι��
  /* Reload IWDG counter */
  IWDG_ReloadCounter();
  /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
  IWDG_Enable();
}

/*
STM32F103Rx     ��64���ŵģ�����Ԥ�������£�
//-��λ�ڼ�͸ո�λ�󣬸��ù���δ������ I/O�˿ڱ����óɸ�������ģʽ
--��������
PA8
PA12
PA13    SWDIO
PA14    SWDCK
PA15
PD2
PB12
PB14
PC4
PC5
PC9
PC12


--����1   CC2530
PA9	  USART1_TXD
PA10	USART1_RXD
--����2 �봥������ͨѶ
PA2	  USART2_TXD
PA3	  USART2_RXD
--����3 WiFi
PB10  USART3_TXD
PB11	USART3_RXD

--��������
PC4   KeySignle

--������
PC2 	Motor_Controler

--LED���
PA4   POWER_LED
PA5   UP_LED
PB4   WIND_LED
PB5   DOWN_LED
PB9   MENU_LED

PB13  AIR
PC10  COOL
PC11  HEAT

--������
PA6	  LD_LAT
PB0	  LD_SCK
PB1	  LD_SDI
PA7	  LD_OE

--������Ƶ�Դ
PB15    ControlPower

--������
PB8   BEEP

--������Ӧ
//-PB3	    PULSE_OUT
PC13	  PULSE_IN

--�ⲿRTC
PA0	  RTC_SCL
PA1	  RTC_SDA

--��λ�Ĵ���HC595
PA11    595CS
PC6 	  595STCP
PC7     595SHCP
PC8     595DATAIN


--�¶ȴ�����I2C
PB6   TEHU_I2C_SCL
PB7   TEHU_I2C_SDA

--ADC
PC0 ��ص�ѹ
PC1 ntc�¶�/��鹩��
*/
void GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);      /*ʹ��SWD ����JTAG*/

//-����
  /* Configure USART1 Tx (PA.09) as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Configure USART2 Tx (PA.02) as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Configure USART3 Tx (PB.10) as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* Configure USART4 Tx (PC.10) as alternate function push-pull */
  //-GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  //-GPIO_Init(GPIOC, &GPIO_InitStructure);

  /* Configure USART1 Rx (PA.10) as input floating */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Configure USART2 Rx (PA.03) as input floating */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  //-Configure USART3 Rx (PB.11) as input floating
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  //-Configure USART4 Rx (PC.11) as input floating,����PM2.5ͨѶ����
  //-GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
  //-GPIO_Init(GPIOC, &GPIO_InitStructure);

//---��������
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

//---������
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

//---LED���
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4 | GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIOA->BRR = GPIO_Pin_4 | GPIO_Pin_5;   //-��ʼΪ�͵�ƽ���
  //-GPIOA->BSRR = GPIO_Pin_5;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_9 | GPIO_Pin_13;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIOB->BRR = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_9 | GPIO_Pin_13;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10 | GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIOC->BRR = GPIO_Pin_10 | GPIO_Pin_11;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
//---������
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0 | GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
//---������Ƶ�Դ
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
//---������
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8;  //PB8����ӦTIM4_CH3
  //-GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //�����������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

//---������Ӧ
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIOB->BSRR = GPIO_Pin_3;   //-��ʼΪ�ߵ�ƽ���,ʹ�ܽ�����Ӧ
  //-GPIOB->BRR = GPIO_Pin_3;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  //-�ⲿRTC  ,,ģ��I2C
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  | GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  //-GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
  //-I2C1_SCL_H;
  //-I2C1_SDA_H;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

//---��λ�Ĵ���HC595
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIOA->BSRR = GPIO_Pin_11;   //-��ʼΪ�ߵ�ƽ���
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIOC->BSRR = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8;   //-��ʼΪ�ߵ�ƽ���
  GPIO_Init(GPIOC, &GPIO_InitStructure);

//---�¶ȴ�����I2C
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6  | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  //-GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
  I2C2_SCL_H;
  I2C2_SDA_H;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

//-ADC
  //-Configure PC.0 (ADC12 Channel10) as analog input,��ص�ѹ�ɼ�
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  //-Configure PC.1 (ADC12 Channel11) as analog input,�ⲿNTC�¶Ȳɼ�
  //-GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
  //-GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  //-GPIO_Init(GPIOC, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

}

void GPIO_Configuration_out(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;


  //- Configure PD2 as output push-pull
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10 | GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  //- Configure PD2 as output push-pull
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_12 | GPIO_Pin_13;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

}

void GPIO_Configuration_in(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}

//-�����жϵ�����
void commonIT_Config(void)
{
	//-EXTI_InitTypeDef EXTI_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_OCInitTypeDef  TIM_OCInitStructure;

	//-�����ⲿ�ж�

  //-����2�ж� PB12	I2C2_SMBA	INT2
  //-EXTI_ClearITPendingBit(EXTI_Line12);
  //-GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource12);
  //-//- Configure EXTI Line12 to generate an interrupt on falling edge
  //-EXTI_InitStructure.EXTI_Line = EXTI_Line12;
  //-EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  //-//-EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;		//-������
  //-EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;		//-�½���
  //-EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  //-EXTI_Init(&EXTI_InitStructure);


  //-����T2��������
  //-TIM2 Configuration: Output Compare Timing Mode:
  //-                 TIM2CLK = 36 *2 =72MHz, Prescaler = 17, TIM2 counter clock = 4 MHz
  //-                 TIM2 update Period = ARR / TIM2 counter Period = 2 ms
  //-                 CC1 OC period = 1ms
  //-ϵͳʱ����56MHz,TIM2���ڵ���ʱ��(APB1)����,Ȼ��������2��Ƶ���Ա�Ϊ28MHz,(���¿�֪ʵ��Ӳ��������56MHz)Ȼ��ʱ�Ӽ���
  //-����28��Ƶ�趨��,TIM2�ļ���ʱ��Ƶ�ʾͱ�Ϊ��1MHz,��ô����2000�ξ���1mS
  //-APB1���������Ƶ����36MHz
  //-��ʱ��ʱ��Ƶ�ʷ�����Ӳ��������2������Զ��趨:
  //-1 �����Ӧ��APBԤ��Ƶϵ����1,��ʱ����ʱ��Ƶ�������ڵ�APB����Ƶ��һ��
  //-2.����,��ʱ����ʱ��Ƶ�ʱ��趨Ϊ����������APB����Ƶ�ʵ�2��.
  //- Time base configuration
  TIM_TimeBaseStructure.TIM_Period = 1000;		//-2000*5����һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ
  TIM_TimeBaseStructure.TIM_Prescaler = 55;		//-27������Ϊ TIMx ʱ��Ƶ�ʳ�����Ԥ��Ƶֵ,,��������ʱ��Ƶ��CK_CNT����fCK_PSC/(PSC[15:0]+1)��
  TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;		//-������ʱ�ӷָ�,,00�� tDTS = tCK_INT
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;		//-ѡ���˼�����ģʽ,,00�����ض���ģʽ�����������ݷ���λ(DIR)���ϻ����¼�����
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

  TIM_ARRPreloadConfig(TIM2,ENABLE); //-ÿ�������к����ɵ�����,������������,,ʹ�ܻ���ʧ�� TIMx �� ARR(�Զ�װ�ؼĴ���) �ϵ�Ԥװ�ؼĴ���
  // only counter overflow/underflow generate U interrupt
  TIM_UpdateRequestConfig(TIM2,TIM_UpdateSource_Global);	//-���� TIMx ��������Դ,1�����ʹ���˸����жϻ�DMA������ֻ�м��������/����Ų��������жϻ�DMA����

  // TIM IT enable
  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE); //-���ж�ʹ��λ��������,���£��������������/�����������������ʼ��(ͨ����������ڲ�/�ⲿ����)

  // TIM2 enable counter
  TIM_Cmd(TIM2, ENABLE);  //-��ʼ����
/*
  //-��ʱ��4����
  TIM_TimeBaseStructure.TIM_Period = 80;		//-����һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ130mS�����ж�һ��
  TIM_TimeBaseStructure.TIM_Prescaler = 55999;		//-������Ϊ TIMx ʱ��Ƶ�ʳ�����Ԥ��Ƶֵ,,��������ʱ��Ƶ��CK_CNT����fCK_PSC/(PSC[15:0]+1)��
  TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;		//-������ʱ�ӷָ�,,00�� tDTS = tCK_INT
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;		//-ѡ���˼�����ģʽ,,00�����ض���ģʽ�����������ݷ���λ(DIR)���ϻ����¼�����
  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

  TIM_ARRPreloadConfig(TIM4,ENABLE); //-ÿ�������к����ɵ�����,������������,,ʹ�ܻ���ʧ�� TIMx �� ARR(�Զ�װ�ؼĴ���) �ϵ�Ԥװ�ؼĴ���
  // only counter overflow/underflow generate U interrupt
  TIM_UpdateRequestConfig(TIM4,TIM_UpdateSource_Global);	//-���� TIMx ��������Դ,1�����ʹ���˸����жϻ�DMA������ֻ�м��������/����Ų��������жϻ�DMA����

  // TIM IT enable
  TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE); //-���ж�ʹ��λ��������,���£��������������/�����������������ʼ��(ͨ����������ڲ�/�ⲿ����)

  // TIM4 enable counter
  TIM_Cmd(TIM4, ENABLE);  //-��ʼ����
  */
  //��ʼ��TIM4
  TIM_TimeBaseStructure.TIM_Period = 1999; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ
  TIM_TimeBaseStructure.TIM_Prescaler =71; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
  TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ

  TIM_ARRPreloadConfig(TIM4,ENABLE);//ʹ��ARRԤװ�أ���ֹ���ϼ���ʱ�����¼��쳣�ӳ�

  //��ʼ��TIM4_CH3 PWM���
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ2
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; //�������:TIM����Ƚϼ��Ե�
  TIM_OCInitStructure.TIM_Pulse = 0;
  TIM_OC3Init(TIM4, &TIM_OCInitStructure);  //�����ò�����ʼ������TIM4 OC3

  TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);  //ʹ��TIM4��CCR3�ϵ�Ԥװ�ؼĴ���

  TIM_Cmd(TIM4, ENABLE);  //ʹ��TIM4
}


void EXTI_Configuration(void)
{
  EXTI_InitTypeDef  EXTI_InitStructure;


  EXTI_ClearITPendingBit(EXTI_Line12);
  /* Configure EXTI Line3 to generate an interrupt on falling edge */
  EXTI_InitStructure.EXTI_Line = EXTI_Line12;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource12);

 //-����keysignle�ж� PC4
  EXTI_ClearITPendingBit(EXTI_Line4);
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource4);
  //- Configure EXTI Line12 to generate an interrupt on falling edge
  EXTI_InitStructure.EXTI_Line = EXTI_Line4;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  //-EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;		//-������
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;		//-�½���
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

}

/*
u8 NVIC_IRQChannel;                    // �����жϺ�
u8 NVIC_IRQChannelPreemptionPriority;	 // ��ռ���ȼ�
u8 NVIC_IRQChannelSubPriority;         // �����ȼ�
FunctionalState NVIC_IRQChannelCmd;    // ʹ�ܺ�ʧ���ж�

��Cortex-M3�ж�����8������λ���������ж�Դ�����ȼ�����8������λ������8�ַ��䷽ʽ�����£�
1.     ����8λ����ָ����Ӧ���ȼ�
2.     ���1λ����ָ����ռʽ���ȼ������7λ����ָ����Ӧ���ȼ�
3.     ���2λ����ָ����ռʽ���ȼ������6λ����ָ����Ӧ���ȼ�
4.     ���3λ����ָ����ռʽ���ȼ������5λ����ָ����Ӧ���ȼ�
5.     ���4λ����ָ����ռʽ���ȼ������4λ����ָ����Ӧ���ȼ�
6.     ���5λ����ָ����ռʽ���ȼ������3λ����ָ����Ӧ���ȼ�
7.     ���6λ����ָ����ռʽ���ȼ������2λ����ָ����Ӧ���ȼ�
8.     ���7λ����ָ����ռʽ���ȼ������1λ����ָ����Ӧ���ȼ�
 ���ϱ������ȼ�����ĸ������Cortex-M3������н����ж�Դʱʹ�ý��ٵļĴ���λָ���ж�Դ�����ȼ������STM32��ָ���ж����ȼ��ļĴ���λ���ٵ�4λ����4���Ĵ���λ�ķ��鷽ʽ���£�
   ��0�飺����4λ����ָ����Ӧ���ȼ�
   ��1�飺���1λ����ָ����ռʽ���ȼ������3λ����ָ����Ӧ���ȼ�
   ��2�飺���2λ����ָ����ռʽ���ȼ������2λ����ָ����Ӧ���ȼ�
   ��3�飺���3λ����ָ����ռʽ���ȼ������1λ����ָ����Ӧ���ȼ�
   ��4�飺����4λ����ָ����ռʽ���ȼ�

ע��:
�����жϵ����ȼ���������жϸ�
 �����жϺ����жϼ���ͬʱ���� ���жϵĴ����� ��RTC_IRQHandler()
 �������������� ��ôҪ���RTC_IRQHandler()�˳�  �������������жϱ�־
    (���������ж�)�� ���� �����жϱ�־����� ��RTCAlarm_IRQHandler()�����϶��ǽ���ȥ��
 �������������жϱ�־ ��ô���������RTC_IRQHandler()���
 �������� �����жϱ����ܴ�����жϵ�ִ�� �����������ִ�е�RTCAlarm_IRQHandler()���
*/
void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Configure the NVIC Preemption Priority Bits */
  /* Configure one bit for preemption priority */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

  //-NVIC_SetPriority(SysTick_IRQn,...);
  /* enabling interrupt */
  //-NVIC_InitStructure.NVIC_IRQChannel=SysTick_IRQn;
  //-NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  //-NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  //-NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  //-NVIC_Init(&NVIC_InitStructure);

  /* enabling interrupt */
  NVIC_InitStructure.NVIC_IRQChannel=TIM2_IRQn;   //-������ȼ�����,������ʾ������
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;			//-by zj 2015/9/7 11:04:20
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;			//-by zj 2015/9/7 11:04:20
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;			//-by zj 2015/9/7 11:04:20
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

  NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;   //ָ����RTCȫ���ж�
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
  NVIC_Init(&NVIC_InitStructure);
  //-RTC���ǳ���,���ڰ����ĵȼ��ᵽ���,���ñ��˴��
  //?����жϿ��Ի��ѵ�Ƭ��,�����Ƭ���Ѿ�����,�����ʲôӰ����
  NVIC_InitStructure.NVIC_IRQChannel = RTCAlarm_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

void init_system_tick(void)
{

   if (SysTick_Config(SystemCoreClock / 4))  //-������������Ѿ��Եδ�ʱ���������ж����ȼ�������,���ڵ�һ����
   {
     /* Capture error */
     while (1);
   }

}

///////////////////////////////////////////////////////////////////////////////


void uart2_config(void)
{
  USART_InitTypeDef USART_InitStructure;

  //-���ÿ��ܵ�DMA
	DMA_InitTypeDef DMA_InitStructure;

  /*
     ����:
          ��׼���õ�����һ������DMA���ͳ�ȥ,ÿ�ζ���Ҫ�޸ĳ���.������Է�����ѭ���и�,�Ƿ��������Ϳ������ж��в�ѯ
     ����:
          ����һ��512�Ľ��ջ�����,���DMA���͵�����,Ȼ���ȡ����.����DMA�����ڴ����,���ú�ָ��Ĺ�ϵ,�ҾͿ���ģ���
          һ��512��FIFO.
  */
  // USARTy TX DMA1 Channel (triggered by USARTy Tx event) Config
  DMA_DeInit(DMA1_Channel7);  //-���ǰ����еĳ�ʼ��ΪĬ��ֵ
  DMA_InitStructure.DMA_PeripheralBaseAddr = USART2_DR_Address;		//-�����ַ(USART1):0x4001 3800 - 0x4001 3BFF + 0x04(�Ĵ���ƫ�Ƶ�ַ) = ������Ե�ַ
  DMA_InitStructure.DMA_MemoryBaseAddr = (UINT32)&port_send[1][0];   //-�洢����ַ
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;            //-���ݴ��䷽��,�������
  DMA_InitStructure.DMA_BufferSize = 1;             //-���ͻ������ߴ�,���ݴ�������
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //-�����ַ����ģʽ,��ִ�������ַ��������
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;				//-�洢����ģʽ
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;   //-�������ݿ��,8λ
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;    //-�洢�����ݿ��,8λ
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;     //-ѭ��ģʽ,ִ��ѭ������,���ݴ������Ŀ��Ϊ0ʱ�������Զ��ر��ָ�������ͨ��ʱ���õĳ�ֵ
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;   //-ͨ�����ȼ�,��?�������ȼ���ô��:�ܹ���4�����ȼ�ͬһ��ģ����
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;   //-�洢�����洢��ģʽ,�Ǵ洢�����洢��ģʽ
  DMA_Init(DMA1_Channel7, &DMA_InitStructure);

  // USARTy RX DMA1 Channel (triggered by USARTy Rx event) Config
  DMA_DeInit(DMA1_Channel6);
  DMA_InitStructure.DMA_PeripheralBaseAddr = USART2_DR_Address;
  DMA_InitStructure.DMA_MemoryBaseAddr = (UINT32)&port_recv[1][0];
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = 256;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;		//-��Ҫ����ѭ��,��DMA�Զ�ȫ������
  DMA_Init(DMA1_Channel6, &DMA_InitStructure);

	//-END

//-#ifdef  BSP_USART2

  USART_InitStructure.USART_BaudRate = 9600;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

  USART_Init(USART2, &USART_InitStructure);
  //-USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//�������ڽ����ж�
  USART_Cmd(USART2, ENABLE);
  while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
	{ //-����0˵�����ͻ�������������û�з��ͳ�ȥ,������Ҫ�ȴ�ֱ�����Ϳ�
	}

  //-Ϊ�˽��������������жϵ�bug,���������޸�
  //-USART_ITConfig(USART2, USART_IT_PE, ENABLE);    //����PE��������ж�Bit 8PEIE: PE interrupt enable
  //-USART_ITConfig(USART2, USART_IT_ERR, ENABLE);   //CR2 ����ERR�ж�

  //- Enable USARTy DMA TX request
  USART_DMACmd(USART2, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE);

  //-ͨ��ʹ�� ����
  DMA_Cmd(DMA1_Channel6, ENABLE);
  //- Enable USARTy DMA TX Channel
  DMA_Cmd(DMA1_Channel7, ENABLE);

  USART_ClearFlag(USART2, USART_FLAG_TC);

//-#endif

}

void TIM_config(void)
{
	 /*TIM_TimeBaseInitTypeDef        TIM1_TimeBaseStructure;
   TIM_OCInitTypeDef                  TIM_OCInitStructure;
   TIM_BDTRInitTypeDef TIM_BDTRInitStructure;
   //-----------------------------------��ʱ��TIM1��������------------------------------------
   TIM_DeInit(TIM1);//���½�TIM1��Ϊȱʡֵ
   TIM_InternalClockConfig(TIM1);//�����ڲ�ʱ�Ӹ�TIM1�ṩʱ��Դ ������8M
   TIM1_TimeBaseStructure.TIM_Prescaler=8;//7��Ƶ,TIM1Ƶ��Ϊ56MHz/7=8M
   TIM1_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;//���ϼ���ģʽ
   TIM1_TimeBaseStructure.TIM_Period=PWM_Period_Value-1;//���������С,ÿ��5000��������һ�������¼�,��PWM���Ƶ��1KHz
   TIM1_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;//ʱ�ӷָ�TDTS=Tck_tim
   TIM1_TimeBaseStructure.TIM_RepetitionCounter=0x0;
   TIM_TimeBaseInit(TIM1,&TIM1_TimeBaseStructure);
   //-���ϼ���,û��ֵǰ�ο���ѹΪ��,����֮��Ϊ��
   //-�����źŵ�����ǲο���ѹ�Ƚϵ�,�����������Ǻ�,��ô������Ͳο���ѹһ��,��һ���෴,ֻ�Ƕ���һ����ʱ
   //- PWM1 Mode configuration: Channel1 		//-GREEN
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable;		//-����û��������������Խ�ֹ���
  TIM_OCInitStructure.TIM_Pulse = PWM_Period_Value;			//-�����ȸ�����ֵ
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;		//-��������ļ���:0�� OC1�ߵ�ƽ��Ч��1�� OC1�͵�ƽ��Ч��
  //���漸�������Ǹ߼���ʱ���Ż��õ���ͨ�ö�ʱ����������
  TIM_OCInitStructure.TIM_OCNPolarity=TIM_OCNPolarity_High;  //���û������������
	TIM_OCInitStructure.TIM_OutputNState=TIM_OutputNState_Enable;//ʹ�ܻ��������
	TIM_OCInitStructure.TIM_OCIdleState=TIM_OCIdleState_Set;  //���������״̬
	TIM_OCInitStructure.TIM_OCNIdleState=TIM_OCIdleState_Set;//�����󻥲������״̬

  TIM_OC1Init(TIM1, &TIM_OCInitStructure);
  //���岽��������ɲ���������ã��߼���ʱ�����еģ�ͨ�ö�ʱ����������
	TIM_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Disable;//����ģʽ�����ѡ��
	TIM_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Disable;//����ģʽ�����ѡ��
	TIM_BDTRInitStructure.TIM_LOCKLevel = TIM_LOCKLevel_OFF; //��������
	TIM_BDTRInitStructure.TIM_DeadTime = 0; //����ʱ������
	TIM_BDTRInitStructure.TIM_Break = TIM_Break_Disable; //ɲ������ʹ��
	TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_High;//ɲ�����뼫��
	TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Disable;//�Զ����ʹ��
	TIM_BDTRConfig(TIM1,&TIM_BDTRInitStructure);

  TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);

  //- PWM1 Mode configuration: Channel2  //-BLUE
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable;
  TIM_OCInitStructure.TIM_Pulse = PWM_Period_Value;

  TIM_OC2Init(TIM1, &TIM_OCInitStructure);
  TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);

  //- PWM1 Mode configuration: Channel3  //-RED
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable;
  //-��������ΪPWM_Period_Valueʱ,PWM����ֱ����� 1
  //-��������Ϊ0ʱ,PWM����ֱ����� 0
  TIM_OCInitStructure.TIM_Pulse = PWM_Period_Value;		//-���1 ����;���0 ���

  TIM_OC3Init(TIM1, &TIM_OCInitStructure);
  TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);

  TIM_ARRPreloadConfig(TIM1, ENABLE);
  //- TIM1 enable counter
  TIM_Cmd(TIM1, ENABLE);
  TIM_CtrlPWMOutputs(TIM1, ENABLE);		//-TIMx->BDTR
  */

   //-�������Ź�
   /* IWDG timeout equal to 250 ms (the timeout may varies due to LSI frequency dispersion) */
  /* Enable write access to IWDG_PR and IWDG_RLR registers */
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
  /* IWDG counter clock: LSI/32 */
  IWDG_SetPrescaler(IWDG_Prescaler_32);
  /* Set counter reload value to obtain 250ms IWDG TimeOut.
     Counter Reload Value = 250ms/IWDG counter clock period
                          = 250ms / (LSI/32)
                          = 0.25s / (LsiFreq/32)
                          = LsiFreq/(32 * 4)
                          = LsiFreq/128
   */
  IWDG_SetReload(1000);	//-���������ֵ�����˿��Ź���λʱ��ÿ�ε�ʱ����0.8mS,�ܴ���������ʱ��:100*0.8=80mS,��ô�����ʱ���ڱ���ι��
  /* Reload IWDG counter */
  IWDG_ReloadCounter();
  /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
  //-IWDG_Enable();

}

void adc_config(void)
{
  ADC_InitTypeDef ADC_InitStructure;
  DMA_InitTypeDef DMA_InitStructure;


	DMA_DeInit(DMA1_Channel1);  //-���ǰ�������Ҫ�Ľ��г�ʼ����ֵ
  DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address; //-�����ַ
  DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&ADC_ConvertedValue[0];  //-�洢����ַ
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //-���ݴ��䷽��,�������
  //-DMA_InitStructure.DMA_BufferSize = 1; //-���ݴ�������Ϊ1
  DMA_InitStructure.DMA_BufferSize = maxbuffer;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//-�����ַ����ģʽ,��ִ�������ַ��������
  //-DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Enable;
  //-DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;//-�洢����ַ����ģʽ,��ִ�д洢����ַ��������
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//-�������ݿ��,16λ
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;//-�洢�����ݿ��,16λ
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; //-ѭ��ģʽ,ִ��ѭ������
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;//-ͨ�����ȼ�,��
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;//-�洢�����洢��ģʽ,�Ǵ洢�����洢��ģʽ
  DMA_Init(DMA1_Channel1, &DMA_InitStructure);

  /* Enable DMA channel1 */
//-ͨ������,,����ͨ������֮�����Դ���и��¾��Զ����Ƶ�Ŀ�Ĵ�
  /*ADC1��ADC2��������ܾ����������ñ����л�*/
  /* ADC1 configuration ------------------------------------------------------*/
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;  //-����ģʽ
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;  //-ʹ��ɨ��ģʽ
  //-ADC_InitStructure.ADC_ScanConvMode = DISABLE;  //
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;  //-ת������������ֱ����λ�����
 // ADC_InitStructure.ADC_ContinuousConvMode = DISABLE; //-����ת��ģʽ
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; //-���������λ����ת��
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;  //-�����Ҷ���
  //-ADC_InitStructure.ADC_NbrOfChannel = 1; //-�����ڹ���ͨ��ת�������е�ͨ����ĿΪ1
  ADC_InitStructure.ADC_NbrOfChannel = 3;
  //ADC_InitStructure.ADC_NbrOfChannel = 7;
  ADC_Init(ADC1, &ADC_InitStructure);

  /* ADC1 regular channel10 configuration PIN15*///-ͨ������ĺ�������ȷ��ת��˳��͸���
  ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_55Cycles5); //-��ת��ʱ��TCONV= 1.5 + 12.5 = 14 ���� = 1 �� s

  /* ADC1 regular channel13 configuration PIN18*/
  ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 2, ADC_SampleTime_55Cycles5);

  /* ADC1 regular channel13 configuration PIN18*/
  ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 3, ADC_SampleTime_55Cycles5);


  /* Enable ADC1 DMA */
  ADC_DMACmd(ADC1, ENABLE); //-ʹ��DMAģʽ,���ܵ����֮��ͻ��Զ������������ź�
// ADC_DMACmd(ADC1, DISABLE);

  ADC_TempSensorVrefintCmd(ENABLE); //ʹ���¶ȴ��������ڲ��ο���ѹͨ��

  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);  //-����ADC

  /* Enable ADC1 reset calibaration register */
  ADC_ResetCalibration(ADC1); //-��ʼ��У׼�Ĵ���
  /* Check the end of ADC1 reset calibration register */
  while(ADC_GetResetCalibrationStatus(ADC1));
  /* Start ADC1 calibaration */
  ADC_StartCalibration(ADC1); //-��������Կ�ʼУ׼������У׼����ʱ��Ӳ�����
  /* Check the end of ADC1 calibration */
  while(ADC_GetCalibrationStatus(ADC1));

  DMA_ITConfig(DMA1_Channel1, DMA_IT_HT, ENABLE);
  DMA_Cmd(DMA1_Channel1, ENABLE);

  /* Start ADC1 Software Conversion */
  //-ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

#if 0
void I2C1_Init(I2C_InitTypeDef* I2C_InitStruct)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    I2C_InitTypeDef I2C_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);



    GPIO_InitStruct.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    I2C_Init(I2C1, I2C_InitStruct);
    I2C_Cmd(I2C1, ENABLE);




}

void I2C_init(void)
{
   I2C_InitTypeDef I2C_InitStructure;
    I2C_InitStructure.I2C_ClockSpeed    =  400000;
    I2C_InitStructure.I2C_Mode          =  I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle     =  I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1   =  0x7C;
    I2C_InitStructure.I2C_Ack           =  I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;

   I2C1_Init(&I2C_InitStructure);

}
#endif





void Usart_es704_Init(void)
{
  //-GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;

  //-RCC_APB2PeriphClockCmd( RCC_APB2Periph_USART1 , ENABLE);

  //-GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  //-GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  //-GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  //-GPIO_Init(GPIOA, &GPIO_InitStructure);

  //-GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  //-GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  //-GPIO_Init(GPIOA, &GPIO_InitStructure);

  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

  /* Configure USART1 */
  USART_Init(USART1, &USART_InitStructure);
   /* Enable the USART1 */
  USART_Cmd(USART1, ENABLE);

  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

}

/**
  * @brief  Configures RTC clock source and prescaler.
  * @param  None
  * @retval None
  */
void RTC_Configuration(void)
{
  /* RTC clock source configuration ------------------------------------------*/
  /* Allow access to BKP Domain */
  PWR_BackupAccessCmd(ENABLE);  //-ʹ�ܺ󱸼Ĵ�������

  /* Reset Backup Domain */
  BKP_DeInit(); //-��BKP��ȫ���Ĵ�������Ϊȱʡֵ

  /* Enable the LSE OSC */
  RCC_LSEConfig(RCC_LSE_ON);  //-�����ⲿ���پ���(LSE)32.768K  ����ָ��LSE��״̬�������ǣ�RCC_LSE_ON��LSE����ON
  /* Wait till LSE is ready */
  while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
  {
  }

  /* Select the RTC Clock Source */
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE); //-����RTCʱ�ӣ�����ָ��RTCʱ�ӣ������ǣ�RCC_RTCCLKSource_LSE��ѡ��LSE��Ϊ�ңԣ�ʱ��

  /* Enable the RTC Clock */
  RCC_RTCCLKCmd(ENABLE);  //-RTC����

  /* RTC configuration -------------------------------------------------------*/
  /* Wait for RTC APB registers synchronisation */
  RTC_WaitForSynchro(); //-��������Ҫ�ȴ�APB1ʱ����RTCʱ��ͬ�������ܶ�д�Ĵ���

  //-ÿһ�ζ�д�Ĵ���ǰ��Ҫȷ����һ�������Ѿ�����
  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();

  /* Set the RTC time base to 1s */
  RTC_SetPrescaler(32767);  //-����RTC��Ƶ����ʹRTCʱ��Ϊ1Hz
  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();

  //-/* Enable the RTC Alarm interrupt */
  RTC_ITConfig(RTC_IT_ALR, ENABLE);   //-ʹ�������ж�
  //-RTC_ITConfig(RTC_IT_SEC, ENABLE); //-ʹ�����ж�
  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();
}

