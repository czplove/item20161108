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
* @brief  触摸按键值处理函数
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
  {//-关机
    if(key_value & 0x03)
    {
      key_value = KEY_ON;   //-风扇特殊处理
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
    {//-待机情况下仅仅是唤醒
      Standby_status = 0;   //-按键退出待机模式
      key_value = 0;
    }
    else
    {
     if(key_value & 0x03)
        key_value = KEY_OFF;
    }
  }

  if(key_value & KEY_ON)
  {//-开机
    write_data_buffer(Buffer,0,3);
    BEEP_On();              //开启蜂鸣器
    beep_wait_time = cticks_5ms;
    beep_status = 1;
    MotorStatus(0x02);     //开启马达

    RUN_ONOFF_status = 1;
    RUN_status = BKP_ReadBackupRegister(BKP_DR4);
    out_hold_flag = 0;
    if((RUN_status == 0) || (RUN_status > 3))
    {//-冷启动,还没有设置
        RUN_status = 1;
        RUN_status_flag = 0x55;
    }

    BKP_WriteBackupRegister(BKP_DR5, 1);

    UART1_transmit_control = 4;
    UART1_transmit_flag=YES;
    //-RUN_status_flag = 0x55;
    //-RUN_status_wait_time = cticks_5ms;
    //-出口输出
    //-out_status_new = OUT_U4_H | OUT_U5_H | OUT_U7_H | OUT_U8_H | OUT_U9_H | OUT_U11_H | OUT_U12_H;
    //-out_status_new = OUT_U4_H | OUT_U5_H | OUT_U7_H | OUT_U8_H | OUT_U9_H | OUT_U11_H | OUT_U12_H;
    //-out_flag = 0x55;

    //-测试用
    //-DMA_Cmd(DMA1_Channel4, DISABLE);
		//-  DMA1_Channel4->CNDTR = 10; //-传输数量寄存器,指示剩余的待传输字节数目
		//-  DMA_Cmd(DMA1_Channel4, ENABLE);
    //-  uart1_to_EXTI();  //-串口1临时改为外部中断

  }
  else
  {
    if(key_value & KEY_OFF)
    {//-关机
        write_data_buffer(Buffer,0,3);
        BEEP_On();              //开启蜂鸣器
        beep_wait_time = cticks_5ms;
        beep_status = 1;
        MotorStatus(0x02);     //开启马达

        //-menu_status = 0;  //-切换到了关机模式
        RUN_ONOFF_status = 0;
        out_hold_flag = 0;
        menu_wind_status = 0x80;

        BKP_WriteBackupRegister(BKP_DR5, 0);
        //-出口输出
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
          BEEP_On();              //开启蜂鸣器
          beep_wait_time = cticks_5ms;
          beep_status = 1;
          MotorStatus(0x02);     //开启马达
         
            if(RUN_status == 2)
            {//-处于制冷模式:室温>设定值时,就需要开始制冷
              if(menu_set_tt_CorF == 1)
              {//-华氏
                if(menu_set_tt_max_f < 90)
                {
                  menu_set_tt_max_f++;
                }
                else
                  menu_set_tt_min_f = 90;
                menu_set_tt_max_eep = (menu_set_tt_max_f - 32) * 500 / 9;  //-数据扩大了100倍 100/1.8 .= 56
                menu_set_tt_volue = menu_set_tt_max_f;
              }
              else
              {//-摄氏
                if(menu_set_tt_max_eep < 3200)
                {//-首先对数据规整处理,防止出现错误数据
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
                temp_data = menu_set_tt_max_f % 100;  //-华氏仅仅是整数部分没有小数
                if(temp_data >= 50)
                   menu_set_tt_max_f = (menu_set_tt_max_f / 100) + 1;
                else
                   menu_set_tt_max_f = (menu_set_tt_max_f / 100);
                  
                menu_set_tt_volue = menu_set_tt_max_eep;  //-摄氏的时候扩大了100倍
              }              
              
              menu_set_tt_flag = 1;
            }
            else if(RUN_status == 1)
            {//-处于制热模式
              if(menu_set_tt_CorF == 1)
              {//-华氏
                if(menu_set_tt_min_f < 90)
                {
                  menu_set_tt_min_f++;
                }
                else
                  menu_set_tt_min_f = 90;
                menu_set_tt_min_eep = (menu_set_tt_min_f - 32) * 500 / 9;  //-数据扩大了100倍 100/1.8 .= 56
                menu_set_tt_volue = menu_set_tt_min_f;
              }
              else
              {//-摄氏
                if(menu_set_tt_min_eep < 3200)
                {//-首先对数据规整处理,防止出现错误数据
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
                temp_data = menu_set_tt_min_f % 100;  //-华氏仅仅是整数部分没有小数
                if(temp_data >= 50)
                   menu_set_tt_min_f = (menu_set_tt_min_f / 100) + 1;
                else
                   menu_set_tt_min_f = (menu_set_tt_min_f / 100);
                  
                menu_set_tt_volue = menu_set_tt_min_eep;  //-摄氏的时候扩大了100倍
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
         {//-菜单
            write_data_buffer(Buffer,0,3);
            BEEP_On();              //开启蜂鸣器
            beep_wait_time = cticks_5ms;
            beep_status = 1;
            MotorStatus(0x02);     //开启马达

            out_hold_flag = 0;

            //-            
            if((SYS_WORK_MODE == 0x11) || (SYS_WORK_MODE == 0x21))
            {//-只有制热
              RUN_status = 1;  //-进入制热模式
              if(menu_set_tt_CorF == 1)
                menu_set_tt_volue = menu_set_tt_min_f;
              else
              {
                menu_set_tt_volue = menu_set_tt_min_eep;
              }
              menu_set_tt_flag = 1;
            }
            else if((SYS_WORK_MODE == 0x12) || (SYS_WORK_MODE == 0x22))
            {//-只有制冷
              RUN_status = 2;  //-进入制冷模式
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
                RUN_status = 2;  //-进入制冷模式
                RUN_status_flag = 0x55;
                if(menu_set_tt_CorF == 1)
                  menu_set_tt_volue = menu_set_tt_max_f;
                else
                  menu_set_tt_volue = menu_set_tt_max_eep;
                menu_set_tt_flag = 1;
              }
              else if(RUN_status == 2)
              {
                RUN_status = 3;  //-进入自动模式
                RUN_status_flag = 0x55;
                //-menu_set_tt_volue = menu_set_tt_more;
                menu_set_tt_flag = 0;
              }            
              else
              {
                RUN_status = 1;  //-进入制热模式
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
                BEEP_On();              //开启蜂鸣器
                beep_wait_time = cticks_5ms;
                beep_status = 1;
                MotorStatus(0x02);     //开启马达
              
                  //-
                  if(RUN_status == 2)
                  {
                    if(menu_set_tt_CorF == 1)
                    {//-华氏
                      if(menu_set_tt_max_f > 50)
                      {
                        menu_set_tt_max_f--;
                      }
                      else
                        menu_set_tt_max_f = 50;
                      menu_set_tt_max_eep = (menu_set_tt_max_f - 32) * 500 / 9;  //-数据扩大了100倍 100/1.8 .= 56
                      menu_set_tt_volue = menu_set_tt_max_f;
                    }
                    else
                    {//-摄氏
                      if(menu_set_tt_max_eep >= 1050)
                      {//-首先对数据规整处理,防止出现错误数据
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
                      temp_data = menu_set_tt_max_f % 100;  //-华氏仅仅是整数部分没有小数
                      if(temp_data >= 50)
                         menu_set_tt_max_f = (menu_set_tt_max_f / 100) + 1;
                      else
                         menu_set_tt_max_f = (menu_set_tt_max_f / 100);
                        
                      menu_set_tt_volue = menu_set_tt_max_eep;  //-摄氏的时候扩大了100倍
                    }
                    
                    menu_set_tt_flag = 1;
                  }
                  else if(RUN_status == 1)
                  {
                      if(menu_set_tt_CorF == 1)
                      {//-华氏
                        if(menu_set_tt_min_f > 50)
                        {
                          menu_set_tt_min_f--;
                        }
                        else
                          menu_set_tt_min_f = 50;
                        menu_set_tt_min_eep = (menu_set_tt_min_f - 32) * 500 / 9;  //-数据扩大了100倍 100/1.8 .= 56
                        menu_set_tt_volue = menu_set_tt_min_f;
                      }
                      else
                      {//-摄氏
                        if(menu_set_tt_min_eep >= 1050)
                        {//-首先对数据规整处理,防止出现错误数据
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
                        temp_data = menu_set_tt_min_f % 100;  //-华氏仅仅是整数部分没有小数
                        if(temp_data >= 50)
                           menu_set_tt_min_f = (menu_set_tt_min_f / 100) + 1;
                        else
                           menu_set_tt_min_f = (menu_set_tt_min_f / 100);
                          
                        menu_set_tt_volue = menu_set_tt_min_eep;  //-摄氏的时候扩大了100倍
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
                 {//-风扇
                    write_data_buffer(Buffer,0,3);
                    BEEP_On();              //开启蜂鸣器
                    beep_wait_time = cticks_5ms;
                    beep_status = 1;
                    MotorStatus(0x02);     //开启马达
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
    //-RUN_status = 0;   //-关机后默认的工作模式是0,不进行任何出口处理,开机默认为制热模式
    menu_key_status = KEY_WIND | KEY_ON | KEY_OFF;


    //-menu_wind_status = 0x80;
    STOP_status2 = 1;
    STOP_wait_time2 = 0;
}






