/*
主芯片:STM32F103R8<T6>
系统时钟:SYSCLK_FREQ_56MHz
编译环境:IAR FOR ARM 7.20.
低功耗:采用STOP模式
  
功能描述:
1mS的定时器
刷点阵屏
led灯
触摸模块之间串口通讯UART2
一个按键输入
motor部件
//-RTC就是通过I2C实现读写的 待做
SHT30
继电器输出
NTC_ADC
蜂鸣器
和网关UART3通讯
和ZigBee通讯UART1
*/

#include "user_conf.h"


extern u8 USART2MemoryBuffer[3];

/* Private variables ---------------------------------------------------------*/


/*
系统实现逻辑(低功耗模式STOP下):
不用的时候处于低功耗模式,一但醒来定时恢复低功耗模式.
无人为干预时,1S自动醒来一次处理常规任务.
当醒来时需要全速工作.

运行模式的修改:
当菜单切换到哪个模式下后,延迟一段时间后确认有效
测试模式需要在特殊的场景下进行,不能影响正常使用


注意:
各种外部中断需要重新考虑
最终需要使用看门狗,最大喂狗时间是26S
?直接上电起不来蜂鸣器一直响,是不是和RTC有关?  --已解决,BAK引脚没有焊接
?如果有干扰近场感应一直触发怎么处理
?串口中断唤醒(转过外部中断线)后,出现数据错误的情况,需要考虑容错,可能发送的无效数据
可以用特殊的避免这种情况
?在不影响功能的情况下尽量减少中断个数,反复进入中断效率低下
?唤醒的第一件事什么事也不干,那么要是近场先唤醒呢
所有的控制当进入一个新状态判断的时候必须保证初始状态是正确的.
*/
int main(void)
{
  //-int i;
  //-u32 temp_date;

  RCC_Configuration();

  GPIO_Configuration();
  NVIC_Configuration();

  EXTI_Configuration();

  uart2_config();
  uart1_config();
  uart3_config();

  //-adc_config();

  /* Setup SysTick Timer for 1 sec interrupts  */
  //-init_system_tick();
  //-NVIC_SetPriority(SysTick_IRQn,0); //-优先级最高

  commonIT_Config();
  AutoWakeupConfigure();
  
  //-GPIO_WriteBit(GPIOC,GPIO_Pin_1,0);  //-亮灯
  RTC_init();
  
  Host_LowLevelDelay(3000);
  SHT3X_init();

  Host_LowLevelDelay(2000);
  //-开始进行软件初始化
  sys_init();
  menu_init();
#if SYS_iwatch_DOG
  iwdg_init();  //-目前设定为26S一个周期
#endif

  BEEP_On();
  //-启动转化
  //-ADC_SoftwareStartConvCmd(ADC1, ENABLE);

  while(1)
  {
    if(STOP_status == 0)
    {//-处于正常状态

       TouchValueHandle(USART2MemoryBuffer);   //处理触摸按键的指令

       //-检查触摸按键值
       uart2_Main();    //-触摸按键通讯

       uart1_Main();    //-ZigBee通讯
       uart3_Main();    //-WiFi通讯

       //-周期检查输出需要
       //-out_judge();
       sys_mode_judge();

       //-MotorStatus(0x02);     //开启马达
       if(Judge_Time_In_MainLoop(CO2_poll_wait_time,5000)==YES)
       {
          CO2_poll_wait_time = cticks_5ms;
          if(SHT3X_status == 0x55)
          {
            SHT3X_ReadMeasurementBuffer(&temperature_data_x10,&humidity_data);  //读取温湿度
            //-显示的时候进行四舍五入处理
            if(temperature_data_x10%10 > 4)
            	temperature_data = (temperature_data_x10/10) * 10 + 5;
            else
            	temperature_data = (temperature_data_x10/10) * 10;
            //-if(menu_set_test_flag == 0x55)
            //-  temperature_data_x10 = menu_set_tt_simu;
          }
          else
            SHT3X_init();
       }
       
       if(Judge_Time_In_MainLoop(pm_renew_wait_time,1000)==YES)
       {
         pm_renew_wait_time = cticks_5ms;
         
          /* Wait until last write operation on RTC registers has finished */
          RTC_WaitForLastTask();
          cticks_s = RTC_GetCounter();  //-获得RTC计数器的值  返回值是u32类型的RTC计数器的值
          sec_to_calendar(cticks_s);
          /* Wait until last write operation on RTC registers has finished */
          RTC_WaitForLastTask();
       }

       sys_delay_judge();
       
       LED_renew();
       //-KEY_read();
       

#if SYS_iwatch_DOG
  IWDG_ReloadCounter();   //-喂狗
#endif

    }
    else
    {//-做好了一切准备,下面进入了低功耗STOP模式,醒来之后建立好正常环境后进入正常处理
      //-进入STOP之前,把不希望唤醒的事件关闭
      //-TIM_Cmd(TIM2, DISABLE); //关闭定时器2，熄灭点阵屏
      //-测试用
      //-DMA_Cmd(DMA1_Channel4, DISABLE);
		  //-DMA1_Channel4->CNDTR = 10; //-传输数量寄存器,指示剩余的待传输字节数目
		  //-DMA_Cmd(DMA1_Channel4, ENABLE);
      //-while(DMA_GetFlagStatus(DMA1_FLAG_TC4) != SET)
      //-  i++;

      uart2_to_EXTI();  //-串口2临时改为外部中断
      uart1_to_EXTI();  //-串口1临时改为外部中断
      uart3_to_EXTI();

      /* Wait until last write operation on RTC registers has finished */
        RTC_WaitForLastTask();
      /* Alarm in 3 second */
        RTC_SetAlarm(RTC_GetCounter()+ 15);
        /* Wait until last write operation on RTC registers has finished */
        RTC_WaitForLastTask();

        //-测试用
        //-DMA_Cmd(DMA1_Channel4, DISABLE);
		  //-DMA1_Channel4->CNDTR = 10; //-传输数量寄存器,指示剩余的待传输字节数目
		  //-DMA_Cmd(DMA1_Channel4, ENABLE);

       /* Request to enter STOP mode with regulator in low power mode*/
       PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);

       //-从STOP中醒来之后执行的第一局话(除中断外)

      /* Configures system clock after wake-up from STOP: enable HSE, PLL and select
         PLL as system clock source (HSE and PLL are disabled in STOP mode) */
      SYSCLKConfig_STOP();
      //-RCC_Configuration();

       //-恢复了正常模式后的处理部分
       //-EXTI_to_uart2();
       STOP_wait_time = cticks_5ms;
       STOP_status = 0;
       //-TIM_Cmd(TIM2, ENABLE);  //打开定时器2，点亮点阵屏
    }

  }
}

