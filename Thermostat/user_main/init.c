/*

*/

#include "user_conf.h"

extern BYTE swing_constant[];
extern BYTE diff_constant[];
extern BYTE third_constant[];

void sys_init(void)
{
  BYTE temp_data;
  WORD temp_data16;
  
	//-menu_set_tt_max = 18;
  //-menu_set_tt_min = 18;
  //-menu_set_tt_more = 3;

  //-���Ƕ�ֵ�ı�����ʼ��
  beep_wait_time = cticks_5ms;
  beep_status = 1;

  STOP_status = 0;  //-����������״̬
  menu_set_tt_cal = 2;
  SET_send_one_flag = 0xff;   //-������������
  Standby_status = 0;
  temp_data_out_flag = 0;
  temp_data_out_time = cticks_5ms;
  temperature_data_out = 250;
  //-menu_set_tt_simu = 180;

  
  //-���붨ֵ,��Щ��ֵ��Ҫд��Flash,���籣��
  //-����ģʽ ���������¶�
  RUN_status = read_bkp_eep(BKP_DR4);  //-����ģʽ(����,����,�Զ�)
  RUN_ONOFF_status = read_bkp_eep(BKP_DR5);
  menu_set_tt_min_eep = read_bkp_eep(BKP_DR2);  //-�˵����õ������¶�ֵ
  if((menu_set_tt_min_eep > 3200) || (menu_set_tt_min_eep < 1000))
    menu_set_tt_min_eep = 1000;
  menu_set_tt_max_eep = read_bkp_eep(BKP_DR3);  //-�˵����õ������¶�ֵ
  if((menu_set_tt_max_eep > 3200) || (menu_set_tt_max_eep < 1000))
    menu_set_tt_max_eep = 3200;
  auto_set_tt_min_eep = read_bkp_eep(BKP_DR8);
  if((auto_set_tt_min_eep > 3200) || (auto_set_tt_min_eep < 1000))
    auto_set_tt_min_eep = 1000;
  auto_set_tt_max_eep = read_bkp_eep(BKP_DR9);
  if((auto_set_tt_max_eep > 3200) || (auto_set_tt_max_eep < 1000))
    auto_set_tt_max_eep = 3200;
  //-�¶���ʾ��ʽ ���������� �������� ��� 0 0 0 0
  temp_data = read_bkp_eep(BKP_DR6);
  menu_set_tt_CorF = (temp_data >> 7) & 0x01;
  beep_onoff = (temp_data >> 6) & 0x01;
  OUT_emergency_hot = (temp_data >> 5) & 0x01;
  menu_wind_status = (temp_data >> 4) & 0x01;  //-����ָ�״̬��Ҫ˼��
  Motor_onoff = (temp_data >> 3) & 0x01;
  //-�¿�������
  SYS_WORK_MODE = read_bkp_eep(BKP_DR7);
  //-SYS_WORK_MODE = 0x16;
  temp_data16 = read_bkp_eep(BKP_DR10);
  //-swing����
  menu_set_tt_swing_pt = (temp_data16 & 0x0f00) >> 8;
  menu_set_tt_swing = swing_constant[menu_set_tt_swing_pt];
  //-diff����
  menu_set_tt_diff_pt = (temp_data16 & 0x00f0) >> 4;
  menu_set_tt_diff = diff_constant[menu_set_tt_diff_pt];
  //-third����
  menu_set_tt_third_pt = temp_data16 & 0x000f;
  menu_set_tt_third = third_constant[menu_set_tt_third_pt];

  
  //-if(menu_set_tt_CorF == 1)
  {
    menu_set_tt_max_f = (u16)(3200 + menu_set_tt_max_eep * 1.8);
    temp_data16 = menu_set_tt_max_f % 100;  //-���Ͻ�������������û��С��
    if(temp_data16 >= 50)
       menu_set_tt_max_f = (menu_set_tt_max_f / 100) + 1;
    else
       menu_set_tt_max_f = (menu_set_tt_max_f / 100);
    menu_set_tt_min_f = (u16)(3200 + menu_set_tt_min_eep * 1.8);
    temp_data16 = menu_set_tt_min_f % 100;  //-���Ͻ�������������û��С��
    if(temp_data16 >= 50)
       menu_set_tt_min_f = (menu_set_tt_min_f / 100) + 1;
    else
       menu_set_tt_min_f = (menu_set_tt_min_f / 100);
  }

  //-menu_set_tt_swing = 1;
  //-menu_set_tt_diff = 3;
  //-menu_set_tt_third = 5;
  //-beep_onoff = 1;
  //-OUT_emergency_hot = 0;
  //-menu_set_tt_CorF = 1;

  //-UART1_transmit_control = 3;
  //-UART1_transmit_flag=YES;

  //-��̶�ֵ����������ڴ���,ÿ����ҳΪ��λ���ж�ȡд��
  MONI_EEPROM_read();

  now_programme_flag = 0;

  temperature_data_flag = 0;
  //-�����ʼ��
  //-if (BKP_ReadBackupRegister(BKP_DR5) != 0x5555)
  //-{//-�ػ�״̬
  //-    RUN_ONOFF_status = 0;
  //-}
  //-else
  //-{//-����
  //-    RUN_status = BKP_ReadBackupRegister(BKP_DR4);
  //-  
  //-}
  
}







void sys_delay_judge(void)
{
  BYTE temp_data;
  WORD temp_data16;
  static BYTE ZigBee_cn = 0,BJ_cn = 0;
  
       if(Judge_Time_In_MainLoop(Touch_wait_time,1100)==YES)
       {
          Touch_wait_time = cticks_5ms;
          Touch_Count2 = 0;
          Touch_Count2_pt = 0;
          test_pt = 0;
       }

       if((beep_status == 1) && (Judge_Time_In_MainLoop(beep_wait_time,70)==YES))
       {
          beep_wait_time = cticks_5ms;
          beep_status = 0;

          BEEP_Off();
          MotorStatus(0);
       }
       
       if((out_onoff_flag == 0x55) && (Judge_Time_In_MainLoop(out_onoff_wait_time,1)==YES))
       {
          out_onoff_flag = 0;
          UART1_transmit_control = 4;
          UART1_transmit_flag=YES;
       }

       if((Judge_Time_In_MainLoop(STOP_wait_time,1000)==YES)) //-һ��250mS  ,,80
       {
         STOP_wait_time = cticks_5ms;

          if(STOP_status2 == 1) //-��Ϊ���ѵ�ʱ����Ҫ�ӳٹر�
          {
            STOP_wait_time2++;
            if(STOP_wait_time2 == 4)
            {
                menu_set_tt_flag = 0;
                
                UART1_transmit_control = 4;
                UART1_transmit_flag=YES;
            }
            if(STOP_wait_time2 >= 10)
            {

              TurnOffDisLED();
#if SYS_MODE_STOP
              STOP_status = 1;
#endif
              menu_set_tt_flag = 0;
              STOP_status2 = 0;
              STOP_wait_time2 = 0;
              Standby_status = 0x55;  //-�������ģʽ

              MONI_EEPROM_write();
              //-������趨������״̬���浽��ֵ������,�����Ż���Ҫ���ǵ����û���޸ĵĻ��Ͳ���Ҫд��Ĵ���
              if(SET_Modify_flag == 0x55)
              {
                write_bkp_eep(BKP_DR2,menu_set_tt_min_eep);
                write_bkp_eep(BKP_DR3,menu_set_tt_max_eep);
                write_bkp_eep(BKP_DR4,RUN_status);
                write_bkp_eep(BKP_DR5,RUN_ONOFF_status);
                temp_data = 0;
                if(menu_set_tt_CorF & 0x01)
                  temp_data = temp_data | 0x80;
                if(beep_onoff & 0x01)
                  temp_data = temp_data | 0x40;
                if(OUT_emergency_hot & 0x01)
                  temp_data = temp_data | 0x20;
                if(menu_wind_status & 0x01)
                  temp_data = temp_data | 0x10;
                if(Motor_onoff & 0x01)
                  temp_data = temp_data | 0x08;
                write_bkp_eep(BKP_DR6,temp_data);
                write_bkp_eep(BKP_DR7,SYS_WORK_MODE);
                temp_data16 = ((menu_set_tt_swing_pt & 0x0f) << 8) + ((menu_set_tt_diff_pt & 0x0f) << 4) + ((menu_set_tt_third_pt & 0x0f) << 0);
                write_bkp_eep(BKP_DR8,auto_set_tt_min_eep);
                write_bkp_eep(BKP_DR9,auto_set_tt_max_eep);
                write_bkp_eep(BKP_DR10,temp_data16);
                
                UART1_transmit_control = 4;
                UART1_transmit_flag=YES;
                
                SET_Modify_flag = 0;
              }
              //-menu_status = 0;    //-�л���Ĭ��ͣ��ģʽ
            }
          }
          else
          {

              TurnOffDisLED();
#if SYS_MODE_STOP
              STOP_status = 1;
#endif
              menu_set_tt_flag = 0;
              STOP_status2 = 0;
              STOP_wait_time2 = 0;

              //-menu_status = 0;
          }

       }

       if(SHT3X_err_flag > 10)  //-ʮ�ζ�дʧ�����ݾ��������²���,�����������л���һֱΪ0�����,������Ҫ��λ������
       {
         SHT3X_status = 0;
         SHT3X_err_flag = 0;
       }

       //-if(Judge_STime_In_MainLoop(OUT_HEAT_one_time,60*5)==YES)
       //-{
       //-   OUT_HEAT_one_flag = 0;
       //-}
       
       if(SET_send_one_flag == 0xff)
       {
         if(Judge_STime_In_MainLoop(SET_send_one_time,60*20)==YES)
         {
            SET_send_one_time = cticks_s;
            SET_send_one_flag = 0;
            
            UART1_transmit_control = 4;
            UART1_transmit_flag=YES;
            
            SET_send_two_flag = 0xff;
            SET_send_two_time = cticks_s;
         }
       }
       
       if(SET_send_two_flag == 0xff)
       {
          if(Judge_STime_In_MainLoop(SET_send_two_time,1)==YES)
          {
            SET_send_two_flag = 0;
            UART1_transmit_control = 5;
            UART1_transmit_flag=YES;
            
            SET_send_one_flag = 0xff;
          }
       }

       //-if(RUN_status_flag == 0x55)
       //-{
       //-    if(Judge_Time_In_MainLoop(RUN_status_wait_time,5000)==YES)
       //-    {
       //-       //-RUN_status = menu_status; //-��ĻϨ��ǰ���ͣ����״̬�����趨������״̬
       //-       out_hold_flag = 0;
       //-       BKP_WriteBackupRegister(BKP_DR4, RUN_status);
       //-       RUN_status_flag = 0;

       //-       UART1_transmit_control = 4;
       //-       UART1_transmit_flag=YES;
       //-    }
       //-}

       //-����̶�ֵ�Ƿ������,���Կ���ʱ��β�ѯ
       //-MONI_EEPROM_write();

       if(now_programme_flag != 0)
       {
          if((m_hour == 23) && (m_min == 59))
          {
            now_programme_time = 0;
            now_programme_flag = 0;
          }
       }
       
       if(temp_data_out_flag == 0)
       {
          if(Judge_Time_In_MainLoop(temp_data_out_time,5000)==YES)
          {
            temp_data_out_time = cticks_5ms;
            UART1_transmit_control = 6;
            UART1_transmit_flag=YES;
          }
       }
       else if(temp_data_out_flag == 1)
       {
          if(Judge_STime_In_MainLoop(temp_data_out_time,60*30)==YES)
          {
              temp_data_out_time = cticks_5ms;
              temp_data_out_flag = 0;
              UART1_transmit_control = 6;
              UART1_transmit_flag=YES;
          }
       }

      if((zigbee_flag == 1) || (zigbee_flag == 2))
      {
        if(Judge_Time_In_MainLoop(zigbee_wait_time,300)==YES)
        {
          if(zigbee_flag == 1)
            zigbee_flag = 2;
          else
            zigbee_flag = 1;
          zigbee_wait_time = cticks_5ms;
          ZigBee_cn ++;
        }
      }
      
      if(ZigBee_cn > 50)
      {
        ZigBee_cn = 0;
        zigbee_flag = 0;
      }
      
      if((Judge_STime_In_MainLoop(SYS_power_wait_time,60*60)==YES) && (SYS_power_FLAG == 0x55))
      {        
        BJ_cn++;          
        UART1_transmit_control = 7;
        UART1_transmit_flag=YES;
        
        if(BJ_cn >= 10)
        {
          SYS_power_wait_time = cticks_s;
          BJ_cn = 0;
        }
      }
      
}

//-�ػ� ����  ���� (�ػ�����µĴ��� ��������µĴ���)
void LED_renew(void)
{
  if(RUN_ONOFF_status == 0)
  {//-�ػ�
      if(Standby_status == 0)
      {//-�Ǵ���
          TouchLedStatus(1,0,0,0,1);
          HeatLED((BitAction)0); //-�ص�
          CoolLED((BitAction)0); //-�ص�
          if((menu_wind_status & 0x01) == 1)
              AirLED((BitAction)1);
          else
              AirLED((BitAction)0);
      }
      else
      {
          TouchLedStatus(0,0,0,0,1);
          HeatLED((BitAction)0); //-�ص�
          CoolLED((BitAction)0); //-�ص�
          AirLED((BitAction)0);
      }
      
  }
  else
  {
      if(Standby_status == 0)
      {//-��������������
          TouchLedStatus(1,1,1,1,1);
          if(RUN_status == 1)
          {
                  CoolLED((BitAction)0); //-����
                  HeatLED((BitAction)1); //-�ص�
          }
          else if(RUN_status == 2)
          {
                  HeatLED((BitAction)0); //-����
                  CoolLED((BitAction)1); //-����
          }
          else
          {
                  HeatLED((BitAction)1); //-����
                  CoolLED((BitAction)1); //-�ص�
          }
          if((menu_wind_status & 0x01) == 1)
              AirLED((BitAction)1);
          else
              AirLED((BitAction)0);
      }
      else
      {
          TouchLedStatus(0,0,1,0,0);
          HeatLED((BitAction)0); //-�ص�
          CoolLED((BitAction)0); //-�ص�
          AirLED((BitAction)0);
      }
  }
}

void KEY_read(void)
{
  static WORD powerlow_cn=0;
  
  if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_1) == 0)
  {
    if(powerlow_cn < 10000)
      powerlow_cn++;    
  }
  else
  {
    powerlow_cn = 0;
  }
  
  if(powerlow_cn > 5000)
  {
    if(SYS_power_FLAG != 0x55)
    {
      SYS_power_wait_time = cticks_s - 60*60 + 10;  //-���ӳ�10S�ϱ�����
      SYS_power_FLAG = 1;
    }    
  }
  else
  {
    if(SYS_power_FLAG == 0x55)
    {
        UART1_transmit_control = 7;
        UART1_transmit_flag=YES;
    }
    SYS_power_FLAG = 0;
  }
  
  if(SYS_power_FLAG == 1)
    SYS_power_FLAG = 0x55;
  
}



