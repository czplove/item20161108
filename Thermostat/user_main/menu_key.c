#include "user_conf.h"

extern u8 USART2MemoryBuffer[3];
extern u8 USART2ReceiveBuffer[30];


void write_data_buffer(u8 *Buffer,u8 data,u8 num)
{
  u8 i;
  for(i=0;i<num;i++)
  {
    Buffer[i] = data;
  }
}


/**
* @fun    void TouchValueHandle
* @brief  ��������ֵ������
* @author huangzibo
* @param  uint8 Value
*
* @retval
*/
void TouchValueHandle(u8 *Buffer)
{
  u8 key_value = 0;
  u16 temp_data;

  if(Buffer[1] == 0)
    return;

  key_value = 1 << (Buffer[1] - 1);
  
  if(RUN_ONOFF_status == 0)
  {//-�ػ�
    if(key_value & 0x03)
    {
      key_value = KEY_ON;   //-�������⴦��
      Standby_status = 0;
    }
    else if(key_value & 0x20)
    {
      if(Standby_status == 0x55)
      {
        key_value = 0;
        Standby_status = 0;
      }
      else
        Standby_status = 0;
    }
    else
      key_value = 0;
  }
  else
  {
    if(Standby_status == 0x55)
    {//-��������½����ǻ���
      Standby_status = 0;   //-�����˳�����ģʽ
      key_value = 0;
    }
    else
    {
     if(key_value & 0x03)
        key_value = KEY_OFF;
    }
  }

  if(key_value & KEY_ON)
  {//-����
    write_data_buffer(Buffer,0,3);
    BEEP_On();              //����������
    beep_wait_time = cticks_5ms;
    beep_status = 1;
    MotorStatus(0x02);     //�������

    RUN_ONOFF_status = 1;
    RUN_status = BKP_ReadBackupRegister(BKP_DR4);
    out_hold_flag = 0;
    if((RUN_status == 0) || (RUN_status > 3))
    {//-������,��û������
        RUN_status = 1;
        RUN_status_flag = 0x55;
    }

    BKP_WriteBackupRegister(BKP_DR5, 1);

    UART1_transmit_control = 4;
    UART1_transmit_flag=YES;
    //-RUN_status_flag = 0x55;
    //-RUN_status_wait_time = cticks_5ms;
    //-�������
    //-out_status_new = OUT_U4_H | OUT_U5_H | OUT_U7_H | OUT_U8_H | OUT_U9_H | OUT_U11_H | OUT_U12_H;
    //-out_status_new = OUT_U4_H | OUT_U5_H | OUT_U7_H | OUT_U8_H | OUT_U9_H | OUT_U11_H | OUT_U12_H;
    //-out_flag = 0x55;

    //-������
    //-DMA_Cmd(DMA1_Channel4, DISABLE);
		//-  DMA1_Channel4->CNDTR = 10; //-���������Ĵ���,ָʾʣ��Ĵ������ֽ���Ŀ
		//-  DMA_Cmd(DMA1_Channel4, ENABLE);
    //-  uart1_to_EXTI();  //-����1��ʱ��Ϊ�ⲿ�ж�

  }
  else
  {
    if(key_value & KEY_OFF)
    {//-�ػ�
        write_data_buffer(Buffer,0,3);
        BEEP_On();              //����������
        beep_wait_time = cticks_5ms;
        beep_status = 1;
        MotorStatus(0x02);     //�������

        //-menu_status = 0;  //-�л����˹ػ�ģʽ
        RUN_ONOFF_status = 0;
        out_hold_flag = 0;
        menu_wind_status = 0x80;

        BKP_WriteBackupRegister(BKP_DR5, 0);
        //-�������
        //-out_status_new = OUT_U4_D | OUT_U5_D | OUT_U7_D | OUT_U8_D | OUT_U9_D | OUT_U11_D | OUT_U12_D;
        //-out_flag = 0x55;

        UART1_transmit_control = 4;
        UART1_transmit_flag=YES;
    }
    else
    {
       if(key_value & KEY_UP)
       {//-UP
          write_data_buffer(Buffer,0,3);
          BEEP_On();              //����������
          beep_wait_time = cticks_5ms;
          beep_status = 1;
          MotorStatus(0x02);     //�������
         
            if(RUN_status == 2)
            {//-��������ģʽ:����>�趨ֵʱ,����Ҫ��ʼ����
              if(menu_set_tt_CorF == 1)
              {//-����
                if(menu_set_tt_max_f < 90)
                {
                  menu_set_tt_max_f++;
                }
                else
                  menu_set_tt_min_f = 90;
                menu_set_tt_max_eep = (menu_set_tt_max_f - 32) * 500 / 9;  //-����������100�� 100/1.8 .= 56
                menu_set_tt_volue = menu_set_tt_max_f;
              }
              else
              {//-����
                if(menu_set_tt_max_eep < 3200)
                {//-���ȶ����ݹ�������,��ֹ���ִ�������
                  temp_data = menu_set_tt_max_eep % 100;
                  if(temp_data >= 50)
                    menu_set_tt_max_eep = (menu_set_tt_max_eep / 100) * 100 + 50;
                  else
                    menu_set_tt_max_eep = (menu_set_tt_max_eep / 100) * 100;
                  menu_set_tt_max_eep += 50;
                }
                else
                  menu_set_tt_max_eep = 3200;
                menu_set_tt_max_f = (u16)(3200 + menu_set_tt_max_eep * 1.8);
                temp_data = menu_set_tt_max_f % 100;  //-���Ͻ�������������û��С��
                if(temp_data >= 50)
                   menu_set_tt_max_f = (menu_set_tt_max_f / 100) + 1;
                else
                   menu_set_tt_max_f = (menu_set_tt_max_f / 100);
                  
                menu_set_tt_volue = menu_set_tt_max_eep;  //-���ϵ�ʱ��������100��
              }              
              
              menu_set_tt_flag = 1;
            }
            else if(RUN_status == 1)
            {//-��������ģʽ
              if(menu_set_tt_CorF == 1)
              {//-����
                if(menu_set_tt_min_f < 90)
                {
                  menu_set_tt_min_f++;
                }
                else
                  menu_set_tt_min_f = 90;
                menu_set_tt_min_eep = (menu_set_tt_min_f - 32) * 500 / 9;  //-����������100�� 100/1.8 .= 56
                menu_set_tt_volue = menu_set_tt_min_f;
              }
              else
              {//-����
                if(menu_set_tt_min_eep < 3200)
                {//-���ȶ����ݹ�������,��ֹ���ִ�������
                  temp_data = menu_set_tt_min_eep % 100;
                  if(temp_data >= 50)
                    menu_set_tt_min_eep = (menu_set_tt_min_eep / 100) * 100 + 50;
                  else
                    menu_set_tt_min_eep = (menu_set_tt_min_eep / 100) * 100;
                  menu_set_tt_min_eep += 50;
                }
                else
                  menu_set_tt_min_eep = 3200;
                menu_set_tt_min_f = (u16)(3200 + menu_set_tt_min_eep * 1.8);
                temp_data = menu_set_tt_min_f % 100;  //-���Ͻ�������������û��С��
                if(temp_data >= 50)
                   menu_set_tt_min_f = (menu_set_tt_min_f / 100) + 1;
                else
                   menu_set_tt_min_f = (menu_set_tt_min_f / 100);
                  
                menu_set_tt_volue = menu_set_tt_min_eep;  //-���ϵ�ʱ��������100��
              }
              
              menu_set_tt_flag = 1;
            }
            else
            {

            }

          
          SET_Modify_flag = 0x55;
          //-now_programme_flag = 0x55;
          now_programme_time = m_hour * 60 + m_min;
       }
       else
       {
         if(key_value & KEY_MENU)
         {//-�˵�
            write_data_buffer(Buffer,0,3);
            BEEP_On();              //����������
            beep_wait_time = cticks_5ms;
            beep_status = 1;
            MotorStatus(0x02);     //�������

            out_hold_flag = 0;

            //-            
            if((SYS_WORK_MODE == 0x11) || (SYS_WORK_MODE == 0x21))
            {//-ֻ������
              RUN_status = 1;  //-��������ģʽ
              if(menu_set_tt_CorF == 1)
                menu_set_tt_volue = menu_set_tt_min_f;
              else
              {
                menu_set_tt_volue = menu_set_tt_min_eep;
              }
              menu_set_tt_flag = 1;
            }
            else if((SYS_WORK_MODE == 0x12) || (SYS_WORK_MODE == 0x22))
            {//-ֻ������
              RUN_status = 2;  //-��������ģʽ
              if(menu_set_tt_CorF == 1)
                menu_set_tt_volue = menu_set_tt_max_f;
              else
                menu_set_tt_volue = menu_set_tt_max_eep;
              menu_set_tt_flag = 1;
            }
            else
            {
              if(RUN_status == 1)
              {
                RUN_status = 2;  //-��������ģʽ
                RUN_status_flag = 0x55;
                if(menu_set_tt_CorF == 1)
                  menu_set_tt_volue = menu_set_tt_max_f;
                else
                  menu_set_tt_volue = menu_set_tt_max_eep;
                menu_set_tt_flag = 1;
              }
              else if(RUN_status == 2)
              {
                RUN_status = 3;  //-�����Զ�ģʽ
                RUN_status_flag = 0x55;
                //-menu_set_tt_volue = menu_set_tt_more;
                menu_set_tt_flag = 0;
              }            
              else
              {
                RUN_status = 1;  //-��������ģʽ
                RUN_status_flag = 0x55;
                if(menu_set_tt_CorF == 1)
                  menu_set_tt_volue = menu_set_tt_min_f;
                else
                  menu_set_tt_volue = menu_set_tt_min_eep;
                menu_set_tt_flag = 1;
              }
              
            }

            //-RUN_status_flag = 0x55;
            //-RUN_status_wait_time = cticks_5ms;

            SET_Modify_flag = 0x55;
            //-now_programme_flag = 0x55;
            now_programme_time = m_hour * 60 + m_min;


         }
         else
         {
             if(key_value & KEY_DOWN)
             {//-DOWN
                write_data_buffer(Buffer,0,3);
                BEEP_On();              //����������
                beep_wait_time = cticks_5ms;
                beep_status = 1;
                MotorStatus(0x02);     //�������
              
                  //-
                  if(RUN_status == 2)
                  {
                    if(menu_set_tt_CorF == 1)
                    {//-����
                      if(menu_set_tt_max_f > 50)
                      {
                        menu_set_tt_max_f--;
                      }
                      else
                        menu_set_tt_max_f = 50;
                      menu_set_tt_max_eep = (menu_set_tt_max_f - 32) * 500 / 9;  //-����������100�� 100/1.8 .= 56
                      menu_set_tt_volue = menu_set_tt_max_f;
                    }
                    else
                    {//-����
                      if(menu_set_tt_max_eep >= 1050)
                      {//-���ȶ����ݹ�������,��ֹ���ִ�������
                        temp_data = menu_set_tt_max_eep % 100;
                        if(temp_data >= 50)
                          menu_set_tt_max_eep = (menu_set_tt_max_eep / 100) * 100 + 50;
                        else
                          menu_set_tt_max_eep = (menu_set_tt_max_eep / 100) * 100;
                        menu_set_tt_max_eep -= 50;
                      }
                      else
                        menu_set_tt_max_eep = 1000;
                      menu_set_tt_max_f = (u16)(3200 + menu_set_tt_max_eep * 1.8);
                      temp_data = menu_set_tt_max_f % 100;  //-���Ͻ�������������û��С��
                      if(temp_data >= 50)
                         menu_set_tt_max_f = (menu_set_tt_max_f / 100) + 1;
                      else
                         menu_set_tt_max_f = (menu_set_tt_max_f / 100);
                        
                      menu_set_tt_volue = menu_set_tt_max_eep;  //-���ϵ�ʱ��������100��
                    }
                    
                    menu_set_tt_flag = 1;
                  }
                  else if(RUN_status == 1)
                  {
                      if(menu_set_tt_CorF == 1)
                      {//-����
                        if(menu_set_tt_min_f > 50)
                        {
                          menu_set_tt_min_f--;
                        }
                        else
                          menu_set_tt_min_f = 50;
                        menu_set_tt_min_eep = (menu_set_tt_min_f - 32) * 500 / 9;  //-����������100�� 100/1.8 .= 56
                        menu_set_tt_volue = menu_set_tt_min_f;
                      }
                      else
                      {//-����
                        if(menu_set_tt_min_eep >= 1050)
                        {//-���ȶ����ݹ�������,��ֹ���ִ�������
                          temp_data = menu_set_tt_min_eep % 100;
                          if(temp_data >= 50)
                            menu_set_tt_min_eep = (menu_set_tt_min_eep / 100) * 100 + 50;
                          else
                            menu_set_tt_min_eep = (menu_set_tt_min_eep / 100) * 100;
                          menu_set_tt_min_eep -= 50;
                        }
                        else
                          menu_set_tt_min_eep = 1000;
                        menu_set_tt_min_f = (u16)(3200 + menu_set_tt_min_eep * 1.8);
                        temp_data = menu_set_tt_min_f % 100;  //-���Ͻ�������������û��С��
                        if(temp_data >= 50)
                           menu_set_tt_min_f = (menu_set_tt_min_f / 100) + 1;
                        else
                           menu_set_tt_min_f = (menu_set_tt_min_f / 100);
                          
                        menu_set_tt_volue = menu_set_tt_min_eep;  //-���ϵ�ʱ��������100��
                      }

                      menu_set_tt_flag = 1;                    
                  }
                  else
                  {
                    
                  }

                SET_Modify_flag = 0x55;    
                //-now_programme_flag = 0x55;
                now_programme_time = m_hour * 60 + m_min;
             }
             else
             {
                 if(key_value & KEY_WIND)
                 {//-����
                    write_data_buffer(Buffer,0,3);
                    BEEP_On();              //����������
                    beep_wait_time = cticks_5ms;
                    beep_status = 1;
                    MotorStatus(0x02);     //�������
                    if(menu_wind_status != 0x51)
                    {
                      if((menu_wind_status & 0x01) == 0)
                      {
                        menu_wind_status = 0x81;
                        //-menu_set_test_flag = 0x55;
                      }
                      else
                      {
                        menu_wind_status = 0x80;
                        //-menu_set_test_flag = 0;
                      }
                    }

                    UART1_transmit_control = 4;
                    UART1_transmit_flag=YES;
                 }
                 else
                 {
                    write_data_buffer(Buffer,0,3);
                 }
             }
         }
       }
    }
  }


}


void menu_init(void)
{
    //-menu_status = 0;
    //-RUN_status = 0;   //-�ػ���Ĭ�ϵĹ���ģʽ��0,�������κγ��ڴ���,����Ĭ��Ϊ����ģʽ
    menu_key_status = KEY_WIND | KEY_ON | KEY_OFF;


    //-menu_wind_status = 0x80;
    STOP_status2 = 1;
    STOP_wait_time2 = 0;
}






