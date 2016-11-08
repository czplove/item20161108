/*
	对底层的支持
*/
#include "stm32f10x.h"
//-#include "demo.h"
//-#include "integer.h"
#include "user_conf.h"



void RCC_Configuration(void){

  SystemInit();   //-主频采用56MHz


  //RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC
  						|RCC_APB2Periph_AFIO , ENABLE);

  //-测试用
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

  //-RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);//使能TIM1时钟

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);	//使能定时器4时钟

  /* Enable DMA clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

  /* Enable ADC1 clock */
  //-RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE );

  /* I2C1 clock enable */
  //-RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);   //-采用模拟I2C

#if SYS_iwatch_DOG
  /* Enable the LSI OSC */
  RCC_LSICmd(ENABLE);		//-the LSI OSC 是内部低速40KHz

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
//-独立看门狗
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
  IWDG_SetReload(4063);	//-这里的设置值决定了看门狗复位时间每次的时间是6.4mS,总次数将决定时长:100*6.4=80mS,那么在这个时间内必须喂狗
  /* Reload IWDG counter */
  IWDG_ReloadCounter();
  /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
  IWDG_Enable();
}

/*
STM32F103Rx     是64引脚的，引脚预定义如下：
//-复位期间和刚复位后，复用功能未开启， I/O端口被配置成浮空输入模式
--悬空引脚
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


--串口1   CC2530
PA9	  USART1_TXD
PA10	USART1_RXD
--串口2 与触摸按键通讯
PA2	  USART2_TXD
PA3	  USART2_RXD
--串口3 WiFi
PB10  USART3_TXD
PB11	USART3_RXD

--按键输入
PC4   KeySignle

--马达控制
PC2 	Motor_Controler

--LED输出
PA4   POWER_LED
PA5   UP_LED
PB4   WIND_LED
PB5   DOWN_LED
PB9   MENU_LED

PB13  AIR
PC10  COOL
PC11  HEAT

--点阵屏
PA6	  LD_LAT
PB0	  LD_SCK
PB1	  LD_SDI
PA7	  LD_OE

--输出控制电源
PB15    ControlPower

--蜂鸣器
PB8   BEEP

--近场感应
//-PB3	    PULSE_OUT
PC13	  PULSE_IN

--外部RTC
PA0	  RTC_SCL
PA1	  RTC_SDA

--移位寄存器HC595
PA11    595CS
PC6 	  595STCP
PC7     595SHCP
PC8     595DATAIN


--温度传感器I2C
PB6   TEHU_I2C_SCL
PB7   TEHU_I2C_SDA

--ADC
PC0 电池电压
PC1 ntc温度/检查供电
*/
void GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);      /*使能SWD 禁用JTAG*/

//-串口
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

  //-Configure USART4 Rx (PC.11) as input floating,进行PM2.5通讯接收
  //-GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
  //-GPIO_Init(GPIOC, &GPIO_InitStructure);

//---按键输入
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

//---马达控制
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

//---LED输出
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4 | GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIOA->BRR = GPIO_Pin_4 | GPIO_Pin_5;   //-初始为低电平输出
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
//---点阵屏
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0 | GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
//---输出控制电源
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
//---蜂鸣器
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8;  //PB8，对应TIM4_CH3
  //-GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

//---近场感应
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIOB->BSRR = GPIO_Pin_3;   //-初始为高电平输出,使能近场感应
  //-GPIOB->BRR = GPIO_Pin_3;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  //-外部RTC  ,,模拟I2C
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  | GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  //-GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
  //-I2C1_SCL_H;
  //-I2C1_SDA_H;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

//---移位寄存器HC595
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIOA->BSRR = GPIO_Pin_11;   //-初始为高电平输出
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIOC->BSRR = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8;   //-初始为高电平输出
  GPIO_Init(GPIOC, &GPIO_InitStructure);

//---温度传感器I2C
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6  | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  //-GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
  I2C2_SCL_H;
  I2C2_SDA_H;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

//-ADC
  //-Configure PC.0 (ADC12 Channel10) as analog input,电池电压采集
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  //-Configure PC.1 (ADC12 Channel11) as analog input,外部NTC温度采集
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

//-开放中断的配置
void commonIT_Config(void)
{
	//-EXTI_InitTypeDef EXTI_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_OCInitTypeDef  TIM_OCInitStructure;

	//-设置外部中断

  //-手势2中断 PB12	I2C2_SMBA	INT2
  //-EXTI_ClearITPendingBit(EXTI_Line12);
  //-GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource12);
  //-//- Configure EXTI Line12 to generate an interrupt on falling edge
  //-EXTI_InitStructure.EXTI_Line = EXTI_Line12;
  //-EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  //-//-EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;		//-上升沿
  //-EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;		//-下降沿
  //-EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  //-EXTI_Init(&EXTI_InitStructure);


  //-开放T2参数配置
  //-TIM2 Configuration: Output Compare Timing Mode:
  //-                 TIM2CLK = 36 *2 =72MHz, Prescaler = 17, TIM2 counter clock = 4 MHz
  //-                 TIM2 update Period = ARR / TIM2 counter Period = 2 ms
  //-                 CC1 OC period = 1ms
  //-系统时钟是56MHz,TIM2挂在低速时钟(APB1)线上,然后设置了2分频所以变为28MHz,(由下可知实际硬件分配了56MHz)然后时钟计数
  //-经过28分频设定后,TIM2的计算时钟频率就变为了1MHz,那么计数2000次就是1mS
  //-APB1的最大允许频率是36MHz
  //-定时器时钟频率分配由硬件按以下2钟情况自动设定:
  //-1 如果相应的APB预分频系数是1,定时器的时钟频率与所在的APB总线频率一致
  //-2.否则,定时器的时钟频率被设定为与其相连的APB总线频率的2倍.
  //- Time base configuration
  TIM_TimeBaseStructure.TIM_Period = 1000;		//-2000*5在下一个更新事件装入活动的自动重装载寄存器周期的值
  TIM_TimeBaseStructure.TIM_Prescaler = 55;		//-27用来作为 TIMx 时钟频率除数的预分频值,,计数器的时钟频率CK_CNT等于fCK_PSC/(PSC[15:0]+1)。
  TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;		//-设置了时钟分割,,00： tDTS = tCK_INT
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;		//-选择了计数器模式,,00：边沿对齐模式。计数器依据方向位(DIR)向上或向下计数。
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

  TIM_ARRPreloadConfig(TIM2,ENABLE); //-每个都具有很自由的配置,而不是做死的,,使能或者失能 TIMx 在 ARR(自动装载寄存器) 上的预装载寄存器
  // only counter overflow/underflow generate U interrupt
  TIM_UpdateRequestConfig(TIM2,TIM_UpdateSource_Global);	//-设置 TIMx 更新请求源,1：如果使能了更新中断或DMA请求，则只有计数器溢出/下溢才产生更新中断或DMA请求

  // TIM IT enable
  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE); //-对中断使能位进行设置,更新：计数器向上溢出/向下溢出，计数器初始化(通过软件或者内部/外部触发)

  // TIM2 enable counter
  TIM_Cmd(TIM2, ENABLE);  //-开始计数
/*
  //-定时器4设置
  TIM_TimeBaseStructure.TIM_Period = 80;		//-在下一个更新事件装入活动的自动重装载寄存器周期的值130mS进入中断一次
  TIM_TimeBaseStructure.TIM_Prescaler = 55999;		//-用来作为 TIMx 时钟频率除数的预分频值,,计数器的时钟频率CK_CNT等于fCK_PSC/(PSC[15:0]+1)。
  TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;		//-设置了时钟分割,,00： tDTS = tCK_INT
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;		//-选择了计数器模式,,00：边沿对齐模式。计数器依据方向位(DIR)向上或向下计数。
  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

  TIM_ARRPreloadConfig(TIM4,ENABLE); //-每个都具有很自由的配置,而不是做死的,,使能或者失能 TIMx 在 ARR(自动装载寄存器) 上的预装载寄存器
  // only counter overflow/underflow generate U interrupt
  TIM_UpdateRequestConfig(TIM4,TIM_UpdateSource_Global);	//-设置 TIMx 更新请求源,1：如果使能了更新中断或DMA请求，则只有计数器溢出/下溢才产生更新中断或DMA请求

  // TIM IT enable
  TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE); //-对中断使能位进行设置,更新：计数器向上溢出/向下溢出，计数器初始化(通过软件或者内部/外部触发)

  // TIM4 enable counter
  TIM_Cmd(TIM4, ENABLE);  //-开始计数
  */
  //初始化TIM4
  TIM_TimeBaseStructure.TIM_Period = 1999; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值
  TIM_TimeBaseStructure.TIM_Prescaler =71; //设置用来作为TIMx时钟频率除数的预分频值
  TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位

  TIM_ARRPreloadConfig(TIM4,ENABLE);//使能ARR预装载，防止向上计数时更新事件异常延迟

  //初始化TIM4_CH3 PWM输出
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //选择定时器模式:TIM脉冲宽度调制模式2
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; //输出极性:TIM输出比较极性低
  TIM_OCInitStructure.TIM_Pulse = 0;
  TIM_OC3Init(TIM4, &TIM_OCInitStructure);  //将配置参数初始化外设TIM4 OC3

  TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);  //使能TIM4在CCR3上的预装载寄存器

  TIM_Cmd(TIM4, ENABLE);  //使能TIM4
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

 //-按键keysignle中断 PC4
  EXTI_ClearITPendingBit(EXTI_Line4);
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource4);
  //- Configure EXTI Line12 to generate an interrupt on falling edge
  EXTI_InitStructure.EXTI_Line = EXTI_Line4;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  //-EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;		//-上升沿
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;		//-下降沿
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

}

/*
u8 NVIC_IRQChannel;                    // 外设中断号
u8 NVIC_IRQChannelPreemptionPriority;	 // 抢占优先级
u8 NVIC_IRQChannelSubPriority;         // 亚优先级
FunctionalState NVIC_IRQChannelCmd;    // 使能和失能中断

在Cortex-M3中定义了8个比特位用于设置中断源的优先级，这8个比特位可以有8种分配方式，如下：
1.     所有8位用于指定响应优先级
2.     最高1位用于指定抢占式优先级，最低7位用于指定响应优先级
3.     最高2位用于指定抢占式优先级，最低6位用于指定响应优先级
4.     最高3位用于指定抢占式优先级，最低5位用于指定响应优先级
5.     最高4位用于指定抢占式优先级，最低4位用于指定响应优先级
6.     最高5位用于指定抢占式优先级，最低3位用于指定响应优先级
7.     最高6位用于指定抢占式优先级，最低2位用于指定响应优先级
8.     最高7位用于指定抢占式优先级，最低1位用于指定响应优先级
 以上便是优先级分组的概念，但是Cortex-M3允许具有较少中断源时使用较少的寄存器位指定中断源的优先级，因此STM32把指定中断优先级的寄存器位减少到4位，这4个寄存器位的分组方式如下：
   第0组：所有4位用于指定响应优先级
   第1组：最高1位用于指定抢占式优先级，最低3位用于指定响应优先级
   第2组：最高2位用于指定抢占式优先级，最低2位用于指定响应优先级
   第3组：最高3位用于指定抢占式优先级，最低1位用于指定响应优先级
   第4组：所有4位用于指定抢占式优先级

注意:
闹钟中断的优先级必须比秒中断高
 闹钟中断和秒中断几乎同时到来 秒中断的处理函数 是RTC_IRQHandler()
 如果进入这个函数 那么要想从RTC_IRQHandler()退出  则必须清除所有中断标志
    (包括闹钟中断)， 这样 闹钟中断标志被清除 则RTCAlarm_IRQHandler()函数肯定是进不去了
 如果不清楚闹钟中断标志 那么程序会死在RTC_IRQHandler()里边
 综上所述 那种中断必须能打断秒中断的执行 这样程序才能执行到RTCAlarm_IRQHandler()里边
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
  NVIC_InitStructure.NVIC_IRQChannel=TIM2_IRQn;   //-这个优先级低了,可能显示屏会闪
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

  NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;   //指定是RTC全局中断
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
  NVIC_Init(&NVIC_InitStructure);
  //-RTC老是出错,现在把它的等级提到最高,不让别人打断
  //?这个中断可以唤醒单片机,如果单片机已经醒了,这个有什么影响呢
  NVIC_InitStructure.NVIC_IRQChannel = RTCAlarm_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

void init_system_tick(void)
{

   if (SysTick_Config(SystemCoreClock / 4))  //-这个函数里面已经对滴答定时器进行了中断优先级的设置,属于第一级内
   {
     /* Capture error */
     while (1);
   }

}

///////////////////////////////////////////////////////////////////////////////


void uart2_config(void)
{
  USART_InitTypeDef USART_InitStructure;

  //-配置可能的DMA
	DMA_InitTypeDef DMA_InitStructure;

  /*
     发送:
          把准备好的内容一次性让DMA发送出去,每次都需要修改长度.处理可以放在主循环中干,是否启动发送可以在中断中查询
     接收:
          开辟一个512的接收缓冲区,检查DMA传送的数量,然后读取数据.接收DMA是周期处理的,运用好指针的关系,我就可以模拟出
          一个512的FIFO.
  */
  // USARTy TX DMA1 Channel (triggered by USARTy Tx event) Config
  DMA_DeInit(DMA1_Channel7);  //-就是把所有的初始化为默认值
  DMA_InitStructure.DMA_PeripheralBaseAddr = USART2_DR_Address;		//-外设地址(USART1):0x4001 3800 - 0x4001 3BFF + 0x04(寄存器偏移地址) = 外设绝对地址
  DMA_InitStructure.DMA_MemoryBaseAddr = (UINT32)&port_send[1][0];   //-存储器地址
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;            //-数据传输方向,从外设读
  DMA_InitStructure.DMA_BufferSize = 1;             //-发送缓冲区尺寸,数据传输数量
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //-外设地址增量模式,不执行外设地址增量操作
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;				//-存储器增模式
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;   //-外设数据宽度,8位
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;    //-存储器数据宽度,8位
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;     //-循环模式,执行循环操作,数据传输的数目变为0时，将会自动地被恢复成配置通道时设置的初值
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;   //-通道优先级,高?几个优先级怎么办:总共有4个优先级同一个模块上
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;   //-存储器到存储器模式,非存储器到存储器模式
  DMA_Init(DMA1_Channel7, &DMA_InitStructure);

  // USARTy RX DMA1 Channel (triggered by USARTy Rx event) Config
  DMA_DeInit(DMA1_Channel6);
  DMA_InitStructure.DMA_PeripheralBaseAddr = USART2_DR_Address;
  DMA_InitStructure.DMA_MemoryBaseAddr = (UINT32)&port_recv[1][0];
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = 256;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;		//-需要周期循环,让DMA自动全部接收
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
  //-USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启串口接收中断
  USART_Cmd(USART2, ENABLE);
  while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
	{ //-对于0说明发送缓冲区还有数据没有发送出去,所以需要等待直到发送空
	}

  //-为了解决反复进入接收中断的bug,进行如下修改
  //-USART_ITConfig(USART2, USART_IT_PE, ENABLE);    //开启PE错误接收中断Bit 8PEIE: PE interrupt enable
  //-USART_ITConfig(USART2, USART_IT_ERR, ENABLE);   //CR2 开启ERR中断

  //- Enable USARTy DMA TX request
  USART_DMACmd(USART2, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE);

  //-通道使能 接收
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
   //-----------------------------------定时器TIM1基本配置------------------------------------
   TIM_DeInit(TIM1);//重新将TIM1设为缺省值
   TIM_InternalClockConfig(TIM1);//采用内部时钟给TIM1提供时钟源 假如是8M
   TIM1_TimeBaseStructure.TIM_Prescaler=8;//7分频,TIM1频率为56MHz/7=8M
   TIM1_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;//向上计数模式
   TIM1_TimeBaseStructure.TIM_Period=PWM_Period_Value-1;//计数溢出大小,每计5000个数产生一个更新事件,即PWM输出频率1KHz
   TIM1_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;//时钟分割TDTS=Tck_tim
   TIM1_TimeBaseStructure.TIM_RepetitionCounter=0x0;
   TIM_TimeBaseInit(TIM1,&TIM1_TimeBaseStructure);
   //-向上计数,没到值前参考电压为高,到了之后为低
   //-互补信号的输出是参考电压比较的,如果输出极性是好,那么主输出和参考电压一样,另一个相反,只是都有一个延时
   //- PWM1 Mode configuration: Channel1 		//-GREEN
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable;		//-现在没有用这个引脚所以禁止输出
  TIM_OCInitStructure.TIM_Pulse = PWM_Period_Value;			//-可以先给个初值
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;		//-决定输出的极性:0： OC1高电平有效；1： OC1低电平有效。
  //下面几个参数是高级定时器才会用到，通用定时器不用配置
  TIM_OCInitStructure.TIM_OCNPolarity=TIM_OCNPolarity_High;  //设置互补端输出极性
	TIM_OCInitStructure.TIM_OutputNState=TIM_OutputNState_Enable;//使能互补端输出
	TIM_OCInitStructure.TIM_OCIdleState=TIM_OCIdleState_Set;  //死区后输出状态
	TIM_OCInitStructure.TIM_OCNIdleState=TIM_OCIdleState_Set;//死区后互补端输出状态

  TIM_OC1Init(TIM1, &TIM_OCInitStructure);
  //第五步，死区和刹车功能配置，高级定时器才有的，通用定时器不用配置
	TIM_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Disable;//运行模式下输出选择
	TIM_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Disable;//空闲模式下输出选择
	TIM_BDTRInitStructure.TIM_LOCKLevel = TIM_LOCKLevel_OFF; //锁定设置
	TIM_BDTRInitStructure.TIM_DeadTime = 0; //死区时间设置
	TIM_BDTRInitStructure.TIM_Break = TIM_Break_Disable; //刹车功能使能
	TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_High;//刹车输入极性
	TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Disable;//自动输出使能
	TIM_BDTRConfig(TIM1,&TIM_BDTRInitStructure);

  TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);

  //- PWM1 Mode configuration: Channel2  //-BLUE
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable;
  TIM_OCInitStructure.TIM_Pulse = PWM_Period_Value;

  TIM_OC2Init(TIM1, &TIM_OCInitStructure);
  TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);

  //- PWM1 Mode configuration: Channel3  //-RED
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable;
  //-脉宽设置为PWM_Period_Value时,PWM引脚直接输出 1
  //-脉宽设置为0时,PWM引脚直接输出 0
  TIM_OCInitStructure.TIM_Pulse = PWM_Period_Value;		//-输出1 亮灯;输出0 灭灯

  TIM_OC3Init(TIM1, &TIM_OCInitStructure);
  TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);

  TIM_ARRPreloadConfig(TIM1, ENABLE);
  //- TIM1 enable counter
  TIM_Cmd(TIM1, ENABLE);
  TIM_CtrlPWMOutputs(TIM1, ENABLE);		//-TIMx->BDTR
  */

   //-独立看门狗
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
  IWDG_SetReload(1000);	//-这里的设置值决定了看门狗复位时间每次的时间是0.8mS,总次数将决定时长:100*0.8=80mS,那么在这个时间内必须喂狗
  /* Reload IWDG counter */
  IWDG_ReloadCounter();
  /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
  //-IWDG_Enable();

}

void adc_config(void)
{
  ADC_InitTypeDef ADC_InitStructure;
  DMA_InitTypeDef DMA_InitStructure;


	DMA_DeInit(DMA1_Channel1);  //-就是把所有需要的进行初始化赋值
  DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address; //-外设地址
  DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&ADC_ConvertedValue[0];  //-存储器地址
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //-数据传输方向,从外设读
  //-DMA_InitStructure.DMA_BufferSize = 1; //-数据传输数量为1
  DMA_InitStructure.DMA_BufferSize = maxbuffer;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//-外设地址增量模式,不执行外设地址增量操作
  //-DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Enable;
  //-DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;//-存储器地址增量模式,不执行存储器地址增量操作
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//-外设数据宽度,16位
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;//-存储器数据宽度,16位
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; //-循环模式,执行循环操作
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;//-通道优先级,高
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;//-存储器到存储器模式,非存储器到存储器模式
  DMA_Init(DMA1_Channel1, &DMA_InitStructure);

  /* Enable DMA channel1 */
//-通道开启,,我想通道开启之后就是源处有更新就自动复制到目的处
  /*ADC1和ADC2的区别可能就是两套配置便于切换*/
  /* ADC1 configuration ------------------------------------------------------*/
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;  //-独立模式
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;  //-使用扫描模式
  //-ADC_InitStructure.ADC_ScanConvMode = DISABLE;  //
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;  //-转换将连续进行直到该位被清除
 // ADC_InitStructure.ADC_ContinuousConvMode = DISABLE; //-单次转换模式
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; //-用软件控制位触发转换
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;  //-数据右对齐
  //-ADC_InitStructure.ADC_NbrOfChannel = 1; //-定义在规则通道转换序列中的通道数目为1
  ADC_InitStructure.ADC_NbrOfChannel = 3;
  //ADC_InitStructure.ADC_NbrOfChannel = 7;
  ADC_Init(ADC1, &ADC_InitStructure);

  /* ADC1 regular channel10 configuration PIN15*///-通过下面的函数可以确定转换顺序和个数
  ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_55Cycles5); //-总转换时间TCONV= 1.5 + 12.5 = 14 周期 = 1 μ s

  /* ADC1 regular channel13 configuration PIN18*/
  ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 2, ADC_SampleTime_55Cycles5);

  /* ADC1 regular channel13 configuration PIN18*/
  ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 3, ADC_SampleTime_55Cycles5);


  /* Enable ADC1 DMA */
  ADC_DMACmd(ADC1, ENABLE); //-使用DMA模式,可能当完成之后就会自动发送请求处理信号
// ADC_DMACmd(ADC1, DISABLE);

  ADC_TempSensorVrefintCmd(ENABLE); //使能温度传感器和内部参考电压通道

  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);  //-开启ADC

  /* Enable ADC1 reset calibaration register */
  ADC_ResetCalibration(ADC1); //-初始化校准寄存器
  /* Check the end of ADC1 reset calibration register */
  while(ADC_GetResetCalibrationStatus(ADC1));
  /* Start ADC1 calibaration */
  ADC_StartCalibration(ADC1); //-软件设置以开始校准，并在校准结束时由硬件清除
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
  PWR_BackupAccessCmd(ENABLE);  //-使能后备寄存器访问

  /* Reset Backup Domain */
  BKP_DeInit(); //-将BKP的全部寄存器重设为缺省值

  /* Enable the LSE OSC */
  RCC_LSEConfig(RCC_LSE_ON);  //-设置外部低速晶振(LSE)32.768K  参数指定LSE的状态，可以是：RCC_LSE_ON：LSE晶振ON
  /* Wait till LSE is ready */
  while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
  {
  }

  /* Select the RTC Clock Source */
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE); //-设置RTC时钟，参数指定RTC时钟，可以是：RCC_RTCCLKSource_LSE：选择LSE作为ＲＴＣ时钟

  /* Enable the RTC Clock */
  RCC_RTCCLKCmd(ENABLE);  //-RTC开启

  /* RTC configuration -------------------------------------------------------*/
  /* Wait for RTC APB registers synchronisation */
  RTC_WaitForSynchro(); //-开启后需要等待APB1时钟与RTC时钟同步，才能读写寄存器

  //-每一次读写寄存器前，要确定上一个操作已经结束
  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();

  /* Set the RTC time base to 1s */
  RTC_SetPrescaler(32767);  //-设置RTC分频器，使RTC时钟为1Hz
  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();

  //-/* Enable the RTC Alarm interrupt */
  RTC_ITConfig(RTC_IT_ALR, ENABLE);   //-使能闹钟中断
  //-RTC_ITConfig(RTC_IT_SEC, ENABLE); //-使能秒中断
  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();
}

