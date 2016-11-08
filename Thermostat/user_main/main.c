/*
��оƬ:STM32F103R8<T6>
ϵͳʱ��:SYSCLK_FREQ_56MHz
���뻷��:IAR FOR ARM 7.20.
�͹���:����STOPģʽ
  
��������:
1mS�Ķ�ʱ��
ˢ������
led��
����ģ��֮�䴮��ͨѶUART2
һ����������
motor����
//-RTC����ͨ��I2Cʵ�ֶ�д�� ����
SHT30
�̵������
NTC_ADC
������
������UART3ͨѶ
��ZigBeeͨѶUART1
*/

#include "user_conf.h"


extern u8 USART2MemoryBuffer[3];

/* Private variables ---------------------------------------------------------*/


/*
ϵͳʵ���߼�(�͹���ģʽSTOP��):
���õ�ʱ���ڵ͹���ģʽ,һ��������ʱ�ָ��͹���ģʽ.
����Ϊ��Ԥʱ,1S�Զ�����һ�δ���������.
������ʱ��Ҫȫ�ٹ���.

����ģʽ���޸�:
���˵��л����ĸ�ģʽ�º�,�ӳ�һ��ʱ���ȷ����Ч
����ģʽ��Ҫ������ĳ����½���,����Ӱ������ʹ��


ע��:
�����ⲿ�ж���Ҫ���¿���
������Ҫʹ�ÿ��Ź�,���ι��ʱ����26S
?ֱ���ϵ�����������һֱ��,�ǲ��Ǻ�RTC�й�?  --�ѽ��,BAK����û�к���
?����и��Ž�����Ӧһֱ������ô����
?�����жϻ���(ת���ⲿ�ж���)��,�������ݴ�������,��Ҫ�����ݴ�,���ܷ��͵���Ч����
����������ı����������
?�ڲ�Ӱ�칦�ܵ�����¾��������жϸ���,���������ж�Ч�ʵ���
?���ѵĵ�һ����ʲô��Ҳ����,��ôҪ�ǽ����Ȼ�����
���еĿ��Ƶ�����һ����״̬�жϵ�ʱ����뱣֤��ʼ״̬����ȷ��.
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
  //-NVIC_SetPriority(SysTick_IRQn,0); //-���ȼ����

  commonIT_Config();
  AutoWakeupConfigure();
  
  //-GPIO_WriteBit(GPIOC,GPIO_Pin_1,0);  //-����
  RTC_init();
  
  Host_LowLevelDelay(3000);
  SHT3X_init();

  Host_LowLevelDelay(2000);
  //-��ʼ���������ʼ��
  sys_init();
  menu_init();
#if SYS_iwatch_DOG
  iwdg_init();  //-Ŀǰ�趨Ϊ26Sһ������
#endif

  BEEP_On();
  //-����ת��
  //-ADC_SoftwareStartConvCmd(ADC1, ENABLE);

  while(1)
  {
    if(STOP_status == 0)
    {//-��������״̬

       TouchValueHandle(USART2MemoryBuffer);   //������������ָ��

       //-��鴥������ֵ
       uart2_Main();    //-��������ͨѶ

       uart1_Main();    //-ZigBeeͨѶ
       uart3_Main();    //-WiFiͨѶ

       //-���ڼ�������Ҫ
       //-out_judge();
       sys_mode_judge();

       //-MotorStatus(0x02);     //�������
       if(Judge_Time_In_MainLoop(CO2_poll_wait_time,5000)==YES)
       {
          CO2_poll_wait_time = cticks_5ms;
          if(SHT3X_status == 0x55)
          {
            SHT3X_ReadMeasurementBuffer(&temperature_data_x10,&humidity_data);  //��ȡ��ʪ��
            //-��ʾ��ʱ������������봦��
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
          cticks_s = RTC_GetCounter();  //-���RTC��������ֵ  ����ֵ��u32���͵�RTC��������ֵ
          sec_to_calendar(cticks_s);
          /* Wait until last write operation on RTC registers has finished */
          RTC_WaitForLastTask();
       }

       sys_delay_judge();
       
       LED_renew();
       //-KEY_read();
       

#if SYS_iwatch_DOG
  IWDG_ReloadCounter();   //-ι��
#endif

    }
    else
    {//-������һ��׼��,��������˵͹���STOPģʽ,����֮���������������������������
      //-����STOP֮ǰ,�Ѳ�ϣ�����ѵ��¼��ر�
      //-TIM_Cmd(TIM2, DISABLE); //�رն�ʱ��2��Ϩ�������
      //-������
      //-DMA_Cmd(DMA1_Channel4, DISABLE);
		  //-DMA1_Channel4->CNDTR = 10; //-���������Ĵ���,ָʾʣ��Ĵ������ֽ���Ŀ
		  //-DMA_Cmd(DMA1_Channel4, ENABLE);
      //-while(DMA_GetFlagStatus(DMA1_FLAG_TC4) != SET)
      //-  i++;

      uart2_to_EXTI();  //-����2��ʱ��Ϊ�ⲿ�ж�
      uart1_to_EXTI();  //-����1��ʱ��Ϊ�ⲿ�ж�
      uart3_to_EXTI();

      /* Wait until last write operation on RTC registers has finished */
        RTC_WaitForLastTask();
      /* Alarm in 3 second */
        RTC_SetAlarm(RTC_GetCounter()+ 15);
        /* Wait until last write operation on RTC registers has finished */
        RTC_WaitForLastTask();

        //-������
        //-DMA_Cmd(DMA1_Channel4, DISABLE);
		  //-DMA1_Channel4->CNDTR = 10; //-���������Ĵ���,ָʾʣ��Ĵ������ֽ���Ŀ
		  //-DMA_Cmd(DMA1_Channel4, ENABLE);

       /* Request to enter STOP mode with regulator in low power mode*/
       PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);

       //-��STOP������֮��ִ�еĵ�һ�ֻ�(���ж���)

      /* Configures system clock after wake-up from STOP: enable HSE, PLL and select
         PLL as system clock source (HSE and PLL are disabled in STOP mode) */
      SYSCLKConfig_STOP();
      //-RCC_Configuration();

       //-�ָ�������ģʽ��Ĵ�����
       //-EXTI_to_uart2();
       STOP_wait_time = cticks_5ms;
       STOP_status = 0;
       //-TIM_Cmd(TIM2, ENABLE);  //�򿪶�ʱ��2������������
    }

  }
}

