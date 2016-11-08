/**
  ******************************************************************************
  * @file    bsp_led.c
  * @author  Wireless Protocol Stack Develop-Dept.
  * @version --
  * @date    2015/12/22 ���ڶ�,Administrator
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

/*
U6�����U10

U6	U10
U4	U5	U7	U8	U9	U11	U12

���ϵ��£�
��ߣ�RH-W1/AUX-W2-E/?-GND-T1
�ұߣ�RC-Y1-Y2-G-O/B-C

��ߣ���Դ��һ�����ȣ��������ȣ�Ԥ�����ӣ��ⲿ�¶ȴ������أ��ⲿ�¶ȴ�����ADC�ɼ���
�ұߣ���Դ��ѹ����һ�������ѹ����������������ȣ���ͨ��������ƶˣ���Դ������

���: ��Դ(24V),   U11,  U12,   U9,    GND,    ADC
�ұ�: ��Դ(��),    U4,   U5,    U7,    U8,     ��Դ������

*/

/* Includes ------------------------------------------------------------------*/

#include "user_conf.h"

//#define S595_OE(BitVal)  			GPIO_WriteBit(GPIOA,GPIO_Pin_11,BitVal)  //-OE ��13�� �����Ч(�͵�ƽ)
//#define S595_SHCP(BitVal)  		GPIO_WriteBit(GPIOC,GPIO_Pin_7,BitVal)  //-SHCP ��11�� ��λ�Ĵ���ʱ������
//#define S595_STCP(BitVal)  		GPIO_WriteBit(GPIOC,GPIO_Pin_6,BitVal)  //-STCP ��12�� �洢�Ĵ���ʱ������,ͨ����Ϊ0,����λ����֮��һ��������,��������
//#define S595_SDI(BitVal)  		GPIO_WriteBit(GPIOC,GPIO_Pin_8,BitVal)  //-DS ��14�� ������������


#define S595_STCP(BitVal)  			GPIO_WriteBit(GPIOC,GPIO_Pin_6,BitVal)  //-OE ��13�� �����Ч(�͵�ƽ)
#define S595_SDI(BitVal)  		  GPIO_WriteBit(GPIOC,GPIO_Pin_8,BitVal)  //-SHCP ��11�� ��λ�Ĵ���ʱ������
#define S595_OE(BitVal)  		    GPIO_WriteBit(GPIOA,GPIO_Pin_11,BitVal)  //-STCP ��12�� �洢�Ĵ���ʱ������,ͨ����Ϊ0,����λ����֮��һ��������,��������
#define S595_SHCP(BitVal)  		  GPIO_WriteBit(GPIOC,GPIO_Pin_7,BitVal)  //-DS ��14�� ������������

static UINT16 temperature_value;  //-ȡ��ʵ����Ҫ�Ƚϵ��¶���ֵ
static UINT8  run_heat_or_cool;   //-��ʾĿǰ�������Ȼ�����ģʽ

/**
  * @fun    void Send_Bit
  * @brief
  *         2015/12/22 ���ڶ�,Administrator
  * @param  None
  *
  * @retval
  */
void S595_Send_Bit(u8 data)
{
  S595_SDI((BitAction)(data&0x01));
  //-��תʱ���ź�������ʱ����
  S595_SHCP((BitAction)1);
  //-NOP_Delay(10);
  S595_SHCP((BitAction)0);
}

/**
  * @fun    void Send_Byte
  * @brief
  *         2015/12/22 ���ڶ�,Administrator
  * @param  u32 data
  * @param  u8 length
  *һ�е������͵�������
  * @retval
*/
void S595_Send_Byte(u32 data, u8 length)
{
  u8 count;
  for(count=0;count<length;count++)
  {
    S595_Send_Bit(data>>count);
  }
}

/**
  * @fun    void Display
  * @brief
  *         2015/12/22 ���ڶ�,Administrator
  * @param  u8 x
  * @param  u32 y
  *
  * @retval
  */
void S595_out(u16 date)
{
  S595_OE((BitAction)0);
  S595_Send_Byte(date,14);
  //-�����������
  S595_STCP((BitAction)1);
  //-NOP_Delay(10);
  S595_STCP((BitAction)0);
}

void out_control(void)
{

//-out_status_new = OUT_U4_D | OUT_U5_D | OUT_U7_D | OUT_U9_D | OUT_U9_D | OUT_U11_D | OUT_U12_D;
//-out_status_new |= OUT_NOP_two(1);
//-out_status_new = OUT_U4_H | OUT_U5_H | OUT_U7_H | OUT_U8_H | OUT_U9_H | OUT_U11_H | OUT_U12_H;
  if((out_flag == 0x55)/* && (out_status_new != out_status)*/)
  {//?����һֱ�����,��һ���һ��ʧ����ô��,���������Ե�����ȽϿɿ���?
      S595_out(out_status_new);
      out_status = out_status_new;
      out_status_new = 0;
      out_flag = 0;
  }
}

void fan_out_judge(void)
{
  UINT16 temp_data;
  	     
    //-�ڴ�֮ǰû�жԷ����й��κ��߼�����
    if((SYS_WORK_MODE & 0xf0) == 0x10)
    {
      if(((out_status_new & OUT_W1orAUX_M) == OUT_W1orAUX(1)) || ((out_status_new & OUT_W2_M) == OUT_W2(1)) || ((out_status_new & OUT_Y1_M) == OUT_Y1(1)) || ((out_status_new & OUT_Y2_M) == OUT_Y2(1)))
      {
          out_status_new |= OUT_wind(1);
          menu_wind_status = 0x51;  //-��ʾĿǰ����ǿ�ƿ���״̬
          out_flag = 0x55;
      }
      else
      {
        if(((menu_wind_status & 0xf0) == 0x50) && (out_flag == 0x55)) //-ֻ�г��ڸ�����,�Żῼ���ջط���
        {
          out_status_new |= OUT_wind(0);
          menu_wind_status = 0x50;  //-��ʾ���ڹرյ���Ϊ�ɿ�״̬
          
        }
      }
    }
    
    if((SYS_WORK_MODE & 0xf0) == 0x20)
    {//-1.	��ͨģʽ����Դϵͳ,���ȿ���ʱ�򣬷��Ȳ��������ȿ����������������ֶ����ơ�
      if(((out_status_new & OUT_Y1_M) == OUT_Y1(1)) || ((out_status_new & OUT_Y2_M) == OUT_Y2(1)))
      {
          out_status_new |= OUT_wind(1);
          menu_wind_status = 0x51;  //-��ʾĿǰ����ǿ�ƿ���״̬
          out_flag = 0x55;
      }
      else
      {
        if(((menu_wind_status & 0xf0) == 0x50) && (out_flag == 0x55)) //-ֻ�г��ڸ�����,�Żῼ���ջط���
        {
          out_status_new |= OUT_wind(0);
          menu_wind_status = 0x50;  //-��ʾ���ڹرյ���Ϊ�ɿ�״̬
          
        }
      }
    }
    
    if((SYS_WORK_MODE & 0xf0) == 0x30)
    {
       if(((out_status_new & OUT_Y1_M) == OUT_Y1(1)) || ((out_status_new & OUT_Y2_M) == OUT_Y2(1)) || ((out_status_new & OUT_W1orAUX_M) == OUT_W1orAUX(1)))
       {
          out_status_new |= OUT_wind(1);
          menu_wind_status = 0x51;
          out_flag = 0x55;
       }
       else
       {
         if(((menu_wind_status & 0xf0) == 0x50) && (out_flag == 0x55))
         {
           out_status_new |= OUT_wind(0);
           menu_wind_status = 0x50;  //-��ʾ���ڹرյ���Ϊ�ɿ�״̬
           
         }
       }
    }

    if((SYS_WORK_MODE & 0xf0) == 0x40)
    {//-2.	�ȱ�ģʽ����Դϵͳ�����ȿ���ʱ�򣬷��Ȳ����Ÿ��ȿ����������������ֶ��������ȱö���Դ������������Ϊ���õø���Ӧ��Ҳ��������
      if(((SYS_WORK_MODE & 0x0f) == 0x01) || ((SYS_WORK_MODE & 0x0f) == 0x02))
      {
        if(((out_status_new & OUT_Y1_M) == OUT_Y1(1)) || ((out_status_new & OUT_Y2_M) == OUT_Y2(1)))
        {//-���� û�и�������¶Է��ȵĿ���
            out_status_new |= OUT_wind(1);
            menu_wind_status = 0x51;  //-��ʾĿǰ����ǿ�ƿ���״̬
            out_flag = 0x55;
        }
        else
        {
          if(((menu_wind_status & 0xf0) == 0x50) && (out_flag == 0x55))
          {
            out_status_new |= OUT_wind(0);
            menu_wind_status = 0x50;  //-��ʾ���ڹرյ���Ϊ�ɿ�״̬
            
          }
        }
      }
      else
      {
        if((((out_status_new & OUT_Y1_M) == OUT_Y1(1)) || ((out_status_new & OUT_Y2_M) == OUT_Y2(1))) && ((out_status_new & OUT_W1orAUX_M) == OUT_W1orAUX(0)))
        {
            out_status_new |= OUT_wind(1);
            menu_wind_status = 0x51;  //-��ʾĿǰ����ǿ�ƿ���״̬
            out_flag = 0x55;
        } 
        else
        {
          if(((menu_wind_status & 0xf0) == 0x50) && (out_flag == 0x55))
          {
            out_status_new |= OUT_wind(0);
            menu_wind_status = 0x50;  //-��ʾ���ڹرյ���Ϊ�ɿ�״̬
            
          }
        }
      }           
    }
    
    if(menu_wind_status != 0x51)
	  {
	    if((menu_wind_status & 0x01) == 1)
	    {
        temp_data = out_status_new & (~OUT_G_M);
        out_status_new = temp_data | OUT_wind(1);
	      out_flag = 0x55;
	    }
	    else
	    {//-������ܾ���Ҫע�͵�,�������߼���������״̬
        temp_data = out_status_new & (~OUT_G_M);
        out_status_new = temp_data | OUT_wind(0);
	      out_flag = 0x55;
	    }
	  }
}

///////////////////////////////////////////////////////////////////////////////
//-�����˱Ƚ���ֵ
#define   normal_level_min_value      menu_set_tt_min
#define   one_level_min_value      (menu_set_tt_min - menu_set_tt_swing)
#define   two_level_min_value      (menu_set_tt_min - menu_set_tt_diff)
#define   three_level_min_value    (menu_set_tt_min - menu_set_tt_third)
#define   normal_level_max_value      menu_set_tt_max
#define   one_level_max_value      (menu_set_tt_max + menu_set_tt_swing)
#define   two_level_max_value      (menu_set_tt_max + menu_set_tt_diff)
#define   three_level_max_value    (menu_set_tt_max + menu_set_tt_third)

///////////////////////////////////////////////////////////////////////////////
void xx_H1_judge(void)
{
      if(temperature_value <= one_level_min_value)
      {

            out_status_new = OUT_HEAT_one(1) | OUT_HEAT_two(0) | OUT_COOL_one(0) | OUT_COOL_two(0);
            //-if(OUT_HEAT_one_flag != 0x55)
            //-{
            //-		OUT_HEAT_one_time = cticks_s;		//-��¼ϵͳʱ��,����Ϊ��λ,���Ե�
            //-		OUT_HEAT_one_flag = 0x55;		//-������һ������
          	//-}

          out_flag = 0x55;
      }
      else
      {
      	  if(temperature_value >= normal_level_min_value)
      	  {
      	  		out_status_new = OUT_HEAT_one(0) | OUT_HEAT_two(0) | OUT_COOL_one(0) | OUT_COOL_two(0);
							out_flag = 0x55;
      	  }
      	  else
      	  {//-��û�дﵽ�ɿ�����,���ı��κγ���

					}
      }
}

void xx_H2_judge(void)
{
      if(temperature_value <= one_level_min_value)
      {
          if(temperature_value <= two_level_min_value)
          {//-����һ�����ȺͶ��������ǿ���ͬʱ�򿪵�
            if(OUT_HEAT_one_flag == 0xaa)
            {
              out_status_new = OUT_HEAT_one(1) | OUT_HEAT_two(1) | OUT_COOL_one(0) | OUT_COOL_two(0);
              out_hold_flag = 0x55;
            }
            else
              out_status_new = OUT_HEAT_one(1) | OUT_HEAT_two(0) | OUT_COOL_one(0) | OUT_COOL_two(0);
          }
          else
          {
            if(out_hold_flag == 0x55)
              out_status_new = OUT_HEAT_one(1) | OUT_HEAT_two(1) | OUT_COOL_one(0) | OUT_COOL_two(0);
            else
              out_status_new = OUT_HEAT_one(1) | OUT_HEAT_two(0) | OUT_COOL_one(0) | OUT_COOL_two(0);            
          }
          if((OUT_HEAT_one_flag != 0xaa) && (OUT_HEAT_one_flag != 0x55))
          {
            		OUT_HEAT_one_time = cticks_s;		//-��¼ϵͳʱ��,����Ϊ��λ,���Ե�
            		OUT_HEAT_one_flag = 0x55;		//-������һ������
          }
          out_flag = 0x55;
      }
      else
      {
      	  if(temperature_value >= normal_level_min_value)
      	  {
      	  		out_status_new = OUT_HEAT_one(0) | OUT_HEAT_two(0) | OUT_COOL_one(0) | OUT_COOL_two(0);
              out_hold_flag = 0;
              OUT_HEAT_one_flag = 0;
							out_flag = 0x55;
              OUT_COOL_one_flag = 0;  //-ͬ�������л���ʱ��,Ҳ��Ҫ����������͵ı�־,��֤��ʼ״̬��ȷ.
      	  }
      	  else
      	  {//-��û�дﵽ�ɿ�����,���ı��κγ���

					}
      }
}

void xx_C1_judge(void)
{
    if(temperature_value >= one_level_max_value)  //-ʵ���¶� > �����¶� ��ֹͣ����
    {
        out_status_new = OUT_HEAT_one(0) | OUT_HEAT_two(0) | OUT_COOL_one(1) | OUT_COOL_two(0);

        out_flag = 0x55;
    }
    else
    {
				if(temperature_value <= normal_level_max_value)
				{
						out_status_new = OUT_HEAT_one(0) | OUT_HEAT_two(0) | OUT_COOL_one(0) | OUT_COOL_two(0);
						out_flag = 0x55;
				}
    }
}

void xx_C2_judge(void)
{
    if(temperature_value >= one_level_max_value)  //-ʵ���¶� > �����¶� ��ֹͣ����
    {
        if(temperature_value >= two_level_max_value)  //-������ֵ������������
        {
          if(OUT_COOL_one_flag == 0xaa)
          {
            out_status_new = OUT_HEAT_one(0) | OUT_HEAT_two(0) | OUT_COOL_one(1) | OUT_COOL_two(1);
            out_hold_flag = 0x55;
          }
          else
            out_status_new = OUT_HEAT_one(0) | OUT_HEAT_two(0) | OUT_COOL_one(1) | OUT_COOL_two(0);
        }
        else
        {
          if(out_hold_flag == 0x55)
            out_status_new = OUT_HEAT_one(0) | OUT_HEAT_two(0) | OUT_COOL_one(1) | OUT_COOL_two(1);
          else
            out_status_new = OUT_HEAT_one(0) | OUT_HEAT_two(0) | OUT_COOL_one(1) | OUT_COOL_two(0);
        }
        if((OUT_COOL_one_flag != 0xaa) && (OUT_COOL_one_flag != 0x55) && (OUT_Y1_flag == 1))
          {
            		OUT_COOL_one_time = cticks_s;		//-��¼ϵͳʱ��,����Ϊ��λ,���Ե�
            		OUT_COOL_one_flag = 0x55;		//-������һ������
          }
        out_flag = 0x55;
    }
    else
    {
				if(temperature_value <= normal_level_max_value)
				{
						out_status_new = OUT_HEAT_one(0) | OUT_HEAT_two(0) | OUT_COOL_one(0) | OUT_COOL_two(0);
            out_hold_flag = 0;
            OUT_COOL_one_flag = 0;
						out_flag = 0x55;
            OUT_HEAT_one_flag = 0;  //-�����жϵ�ʱ��,���ȵı�־Ҳ��Ҫ���,�������޹���������ʱ��Ż�������
				}
    }
}

///////////////////////////////////////////////////////////////////////////////

/*
����W1��ʱ6S��W2,�ر�һ���,�ٴ�����W12û����ʱ����.
*/
void H2_out_judge(void)
{
  out_status_new = 0;
  //-�����趨��ֵ�ĺ�������Ҫ����
  if(RUN_status == 1)
  {//-����
      xx_H2_judge();
  }
  else
  {//-�쳣����
      OUT_HEAT_one_flag = 0;
      out_flag = 0;
  }
	//-
}

/*
�ȿ�Y1��ʱ6S����Y2,�ر�һ��ر�,�رպ�ѹ��������������,�ſ��Եڶ��ο���
*/
void C2_out_judge(void)
{
  out_status_new = 0;
  //-�����趨��ֵ�ĺ�������Ҫ����
  if(RUN_status == 2)
  {//-����
    xx_C2_judge();
  }
  else
  {//-�쳣����
      out_flag = 0;
  }




	//-
}

/*
����û���κα���,������Ҫ2����ѹ��������,����ģʽ���һ��
*/
void C1H1_out_judge(void)
{
  out_status_new = 0;
  //-�����趨��ֵ�ĺ�������Ҫ����
  if(RUN_status == 1)
  {//-����
    run_heat_or_cool = 1;      
  }
  else if(RUN_status == 2)
  {//-����
    run_heat_or_cool = 2;    
  }
  else if(RUN_status == 3)
  {//-�Զ�
      if(temperature_value < normal_level_min_value)
      {//-�Զ�����
        run_heat_or_cool = 1;
      }
      else
      {
          //-if(temperature_value >= normal_level_max_value)  //-ʵ���¶� > �����¶� ��ֹͣ����
          //-�Զ�����
            run_heat_or_cool = 2;              
      }
  }
  else
  {//-�쳣����
      out_flag = 0;
  }

  if(run_heat_or_cool == 1)
  {//-����
    xx_H1_judge();
  }
  else if(run_heat_or_cool == 2)
  {//-����
    xx_C1_judge();
  }
  else
  {//-�쳣����
      out_flag = 0;
  }
    


	//-
}

void C1H2_out_judge(void)
{
  out_status_new = 0;
  //-�����趨��ֵ�ĺ�������Ҫ����
  if(RUN_status == 1)
  {//-����
    run_heat_or_cool = 1;
  }
  else if(RUN_status == 2)
  {//-����
    run_heat_or_cool = 2;     
  }
  else if(RUN_status == 3)
  {//-�Զ�
      if(temperature_value < normal_level_min_value)
      {//-�Զ�����
        run_heat_or_cool = 1; 
      }
      else
      {
          //-if(temperature_value >= normal_level_max_value)  //-ʵ���¶� > �����¶� ��ֹͣ����
          //-�Զ�����
            run_heat_or_cool = 2; 

      }
  }
  else
  {//-�쳣����
      out_flag = 0;
  }

  if(run_heat_or_cool == 1)
  {//-����
    xx_H2_judge();
  }
  else if(run_heat_or_cool == 2)
  {//-����
    xx_C1_judge();
  }
  else
  {//-�쳣����
      out_flag = 0;
  }



	//-
}

void C2H1_out_judge(void)
{
  out_status_new = 0;
  //-�����趨��ֵ�ĺ�������Ҫ����
  if(RUN_status == 1)
  {//-����
    run_heat_or_cool = 1;    
  }
  else if(RUN_status == 2)
  {//-����
    run_heat_or_cool = 2;    
  }
  else if(RUN_status == 3)
  {//-�Զ�
      if(temperature_value < normal_level_min_value)
      {//-�Զ�����
        run_heat_or_cool = 1;
      }
      else
      {
          //-if(temperature_value >= normal_level_max_value)  //-ʵ���¶� > �����¶� ��ֹͣ����
          //-�Զ�����
            run_heat_or_cool = 2;
      }
  }
  else
  {//-�쳣����
      out_flag = 0;
  }

  if(run_heat_or_cool == 1)
  {//-����
    xx_H1_judge();
  }
  else if(run_heat_or_cool == 2)
  {//-����
    xx_C2_judge();
  }
  else
  {//-�쳣����
      out_flag = 0;
  }


	//-
}

/*
ʵ���¶� > �����¶� ��ֹͣ����

�ϵ��һ��:��֤���ڲ���
��������еĳ��ڽ��п����ж�

1.�¶��ڷ�ֵ���¿��ٲ�����ô��?
һ�����������֮��ſ��ǹر�
�û���һ���趨ֵmenu_set_tt_cool Ȼ��menu_set_tt_swing��һ��ҡ��ֵ,��ֹ��������������
2.��������Ҫ���Ƕ�ֵ�����Լ��?
*/
void C2H2_out_judge(void)
{
  out_status_new = 0;
  //-�����趨��ֵ�ĺ�������Ҫ����
  if(RUN_status == 1)
  {//-����
    run_heat_or_cool = 1;     
  }
  else if(RUN_status == 2)
  {//-����
    run_heat_or_cool = 2;
  }
  else if(RUN_status == 3)
  {//-�Զ�
      if(temperature_value < normal_level_min_value)
      {//-�Զ�����
        run_heat_or_cool = 1;
      }
      else
      {
          //-if(temperature_value >= normal_level_max_value)  //-ʵ���¶� > �����¶� ��ֹͣ����
          //-�Զ�����
            run_heat_or_cool = 2;
      }
  }
  else
  {//-�쳣����
      out_flag = 0;
  }

  if(run_heat_or_cool == 1)
  {//-����
    xx_H2_judge();
  }
  else if(run_heat_or_cool == 2)
  {//-����
    xx_C2_judge();
  }
  else
  {//-�쳣����
      out_flag = 0;
  }


	//-
}

///////////////////////////////////////////////////////////////////////////////
void xx_one_P_heal_judge(void)
{
      if(temperature_value <= one_level_min_value)
      {

          {
            out_status_new = OUT_Y1(1) | OUT_Y2(0) | OUT_OorB(0) | OUT_EorNOP(1);
            //-if(OUT_HEAT_one_flag != 0x55)
            //-{
            //-		OUT_HEAT_one_time = cticks_s;		//-��¼ϵͳʱ��,����Ϊ��λ,���Ե�
            //-		OUT_HEAT_one_flag = 0x55;		//-������һ������
          	//-}
          }
          out_flag = 0x55;
      }
      else
      {
      	  if(temperature_value >= normal_level_min_value)
      	  {
      	  		out_status_new = OUT_Y1(0) | OUT_Y2(0) | OUT_OorB(0) | OUT_EorNOP(1);
							out_flag = 0x55;
      	  }
      	  else
      	  {//-��û�дﵽ�ɿ�����,���ı��κγ���

					}
      }
}

void xx_one_P_cool_judge(void)
{
    if(temperature_value >= one_level_max_value)  //-ʵ���¶� > �����¶� ��ֹͣ����
    {

        {
            out_status_new = OUT_Y1(1) | OUT_Y2(0) | OUT_OorB(1) | OUT_EorNOP(0);
        }
        out_flag = 0x55;
    }
    else
    {
				if(temperature_value <= normal_level_max_value)
				{
						out_status_new = OUT_Y1(0) | OUT_Y2(0) | OUT_OorB(1) | OUT_EorNOP(0);
						out_flag = 0x55;
				}
    }
}

void xx_two_P_heal_judge(void)  //-�ȿ�Y1��ʱ6S����Y2,�ر�һ��ر�,�رպ�ѹ��������������,�ſ��Եڶ��ο���
{
      if(temperature_value <= one_level_min_value)
      {
          if(temperature_value <= two_level_min_value)
          {//-����һ�����ȺͶ��������ǿ���ͬʱ�򿪵�
            if(OUT_HEAT_one_flag == 0xaa)
            {
              out_status_new = OUT_Y1(1) | OUT_Y2(1) | OUT_OorB(0) | OUT_EorNOP(1);
              out_hold_flag = 0x55;
            }
            else
              out_status_new = OUT_Y1(1) | OUT_Y2(0) | OUT_OorB(0) | OUT_EorNOP(1);
          }
          else
          {
            if(out_hold_flag == 0x55)
              out_status_new = OUT_Y1(1) | OUT_Y2(1) | OUT_OorB(0) | OUT_EorNOP(1);
            else
              out_status_new = OUT_Y1(1) | OUT_Y2(0) | OUT_OorB(0) | OUT_EorNOP(1);            
          }
          if((OUT_HEAT_one_flag != 0xaa) && (OUT_HEAT_one_flag != 0x55))
          {
            		OUT_HEAT_one_time = cticks_s;		//-��¼ϵͳʱ��,����Ϊ��λ,���Ե�
            		OUT_HEAT_one_flag = 0x55;		//-������һ������
          }
          out_flag = 0x55;
      }
      else
      {
      	  if(temperature_value >= normal_level_min_value)
      	  {
      	  		out_status_new = OUT_Y1(0) | OUT_Y2(0) | OUT_OorB(0) | OUT_EorNOP(1);
              out_hold_flag = 0;
              OUT_HEAT_one_flag = 0;
							out_flag = 0x55;
              OUT_COOL_one_flag = 0;
      	  }
      	  else
      	  {//-��û�дﵽ�ɿ�����,���ı��κγ���

					}
      }
}

void xx_two_P_cool_judge(void)
{
    if(temperature_value >= one_level_max_value)  //-ʵ���¶� > �����¶� ��ֹͣ����
    {
        if(temperature_value >= two_level_max_value)  //-������ֵ������������
        {
          if(OUT_COOL_one_flag == 0xaa)
          {
            out_status_new = OUT_Y1(1) | OUT_Y2(1) | OUT_OorB(1) | OUT_EorNOP(0);
            out_hold_flag = 0x55;
          }
          else
            out_status_new = OUT_Y1(1) | OUT_Y2(0) | OUT_OorB(1) | OUT_EorNOP(0);
        }
        else
        {
          if(out_hold_flag == 0x55)
            out_status_new = OUT_Y1(1) | OUT_Y2(1) | OUT_OorB(1) | OUT_EorNOP(0);
          else
            out_status_new = OUT_Y1(1) | OUT_Y2(0) | OUT_OorB(1) | OUT_EorNOP(0);
        }
        if((OUT_COOL_one_flag != 0xaa) && (OUT_COOL_one_flag != 0x55) && (OUT_Y1_flag == 1))
          {
            		OUT_COOL_one_time = cticks_s;		//-��¼ϵͳʱ��,����Ϊ��λ,���Ե�
            		OUT_COOL_one_flag = 0x55;		//-������һ������
          }
        out_flag = 0x55;
    }
    else
    {
				if(temperature_value <= normal_level_max_value)
				{
						out_status_new = OUT_Y1(0) | OUT_Y2(0) | OUT_OorB(1) | OUT_EorNOP(0);
            out_hold_flag = 0;
            OUT_COOL_one_flag = 0;
						out_flag = 0x55;
            OUT_HEAT_one_flag = 0;
				}
    }
}

void xx_one_PaAUX_heal_judge(void)
{
  UINT16 temp_data;
  
      if(temperature_value <= one_level_min_value)
      {
          if(temperature_value <= three_level_min_value)
          {//-����һ�����ȺͶ��������ǿ���ͬʱ�򿪵�
            //-if(OUT_W1orAUX_flag == 0)
            {
              out_status_new = OUT_Y1(1) | OUT_Y2(0) | OUT_OorB(0) | OUT_EorNOP(1) | OUT_W1orAUX(1);
              out_hold_flag = 0x55;
              //-OUT_W1orAUX_flag = 1;
            }
          }
          else
          {
            if(out_hold_flag == 0x55)
              out_status_new = OUT_Y1(1) | OUT_Y2(0) | OUT_OorB(0) | OUT_EorNOP(1) | OUT_W1orAUX(1);
            else
              out_status_new = OUT_Y1(1) | OUT_Y2(0) | OUT_OorB(0) | OUT_EorNOP(1) | OUT_W1orAUX(0);
            //-if(OUT_HEAT_one_flag != 0x55)
            //-{
            //-		OUT_HEAT_one_time = cticks_s;		//-��¼ϵͳʱ��,����Ϊ��λ,���Ե�
            //-		OUT_HEAT_one_flag = 0x55;		//-������һ������
          	//-}
          }
          
          
          if((temperature_data_out < 30) || (temperature_data_out & 0x8000))  //-�������Ҫ���Ǹ���������
          {
              temp_data = out_status_new & (~OUT_W1orAUX_M);
              out_status_new = temp_data | OUT_W1orAUX(1);
          }
          out_flag = 0x55;
      }
      else
      {
      	  if(temperature_value >= normal_level_min_value)
      	  {
      	  		out_status_new = OUT_Y1(0) | OUT_Y2(0) | OUT_OorB(0) | OUT_EorNOP(1) | OUT_W1orAUX(0);
              out_hold_flag = 0;
							out_flag = 0x55;
      	  }
      	  else
      	  {//-��û�дﵽ�ɿ�����,���ı��κγ���

					}
      }
      
      if((OUT_emergency_hot == 1) || (E_out_flag == 1))
      {
        out_status_new = OUT_Y1(0) | OUT_Y2(0) | OUT_OorB(0) | OUT_EorNOP(1) | OUT_W1orAUX(1);
        OUT_emergency_hot = 1;
        E_out_flag = 0x55;
        E_start_flag = 0x55;
      }
}

void xx_one_PaAUX_cool_judge(void)
{
    if(temperature_value >= one_level_max_value)  //-ʵ���¶� > �����¶� ��ֹͣ����
    {
        
            out_status_new = OUT_Y1(1) | OUT_Y2(0) | OUT_OorB(1) | OUT_EorNOP(0) | OUT_W1orAUX(0);
        
        out_flag = 0x55;
    }
    else
    {
				if(temperature_value <= normal_level_max_value)
				{
						out_status_new = OUT_Y1(0) | OUT_Y2(0) | OUT_OorB(1) | OUT_EorNOP(0) | OUT_W1orAUX(0);
            out_hold_flag = 0;
						out_flag = 0x55;
				}
    }
}

void xx_two_PaAUX_heal_judge(void)
{
  UINT16 temp_data;
  
      if(temperature_value <= one_level_min_value)
      {
          if(temperature_value <= two_level_min_value)
          {//-����һ�����ȺͶ��������ǿ���ͬʱ�򿪵�
            if(temperature_value <= three_level_min_value)
            {
              if(OUT_HEAT_one_flag == 0xaa)
              {
                out_status_new = OUT_Y1(1) | OUT_Y2(1) | OUT_OorB(0) | OUT_EorNOP(1) | OUT_W1orAUX(1);
                out_hold_flag = 0xaa;
              }
              else
                out_status_new = OUT_Y1(1) | OUT_Y2(0) | OUT_OorB(0) | OUT_EorNOP(1) | OUT_W1orAUX(1);
            }
            else
            {
              if(OUT_HEAT_one_flag == 0xaa)
              {
                if(out_hold_flag == 0xaa)
                  out_status_new = OUT_Y1(1) | OUT_Y2(1) | OUT_OorB(0) | OUT_EorNOP(1) | OUT_W1orAUX(1);
                else
                {
                  out_status_new = OUT_Y1(1) | OUT_Y2(1) | OUT_OorB(0) | OUT_EorNOP(1) | OUT_W1orAUX(0);
                  out_hold_flag = 0x55;
                }
              }
              else
              {
                  out_status_new = OUT_Y1(1) | OUT_Y2(0) | OUT_OorB(0) | OUT_EorNOP(1) | OUT_W1orAUX(0);
              }
            }
          }
          else
          {
            if(out_hold_flag == 0xaa)
              out_status_new = OUT_Y1(1) | OUT_Y2(1) | OUT_OorB(0) | OUT_EorNOP(1) | OUT_W1orAUX(1);
            else if(out_hold_flag == 0x55)
              out_status_new = OUT_Y1(1) | OUT_Y2(1) | OUT_OorB(0) | OUT_EorNOP(1) | OUT_W1orAUX(0);
            else
              out_status_new = OUT_Y1(1) | OUT_Y2(0) | OUT_OorB(0) | OUT_EorNOP(1) | OUT_W1orAUX(0);            
          }
          if((OUT_HEAT_one_flag != 0x55) && (OUT_HEAT_one_flag != 0xaa))
          {
            		OUT_HEAT_one_time = cticks_s;		//-��¼ϵͳʱ��,����Ϊ��λ,���Ե�
            		OUT_HEAT_one_flag = 0x55;		//-������һ������
          }
          if((temperature_data_out < 30) || (temperature_data_out & 0x8000)) //-�������Ҫ���Ǹ���������
          {
              temp_data = out_status_new & (~OUT_W1orAUX_M);
              out_status_new = temp_data | OUT_W1orAUX(1);
          }
          out_flag = 0x55;
      }
      else
      {
      	  if(temperature_value >= normal_level_min_value)
      	  {
      	  		out_status_new = OUT_Y1(0) | OUT_Y2(0) | OUT_OorB(0) | OUT_EorNOP(1) | OUT_W1orAUX(0);
              out_hold_flag = 0;
              OUT_HEAT_one_flag = 0;
							out_flag = 0x55;
              OUT_COOL_one_flag = 0;
      	  }
      	  else
      	  {//-��û�дﵽ�ɿ�����,���ı��κγ���

					}
      }
      
      if((OUT_emergency_hot == 1) || (E_out_flag == 1))
      {
        out_status_new = OUT_Y1(0) | OUT_Y2(0) | OUT_OorB(0) | OUT_EorNOP(1) | OUT_W1orAUX(1);
        OUT_emergency_hot = 1;
        E_out_flag = 0x55;
        E_start_flag = 0x55;
      }
}

void xx_two_PaAUX_cool_judge(void)
{
    if(temperature_value >= one_level_max_value)  //-ʵ���¶� > �����¶� ��ֹͣ����
    {
        if(temperature_value >= two_level_max_value)  //-������ֵ������������
        {
              if(OUT_COOL_one_flag == 0xaa)
                out_status_new = OUT_Y1(1) | OUT_Y2(1) | OUT_OorB(1) | OUT_EorNOP(0) | OUT_W1orAUX(0);
              else
              {
                out_status_new = OUT_Y1(1) | OUT_Y2(0) | OUT_OorB(1) | OUT_EorNOP(0) | OUT_W1orAUX(0);
                out_hold_flag = 0x55;
              }            
        }
        else
        {
          if(out_hold_flag == 0x55)
            out_status_new = OUT_Y1(1) | OUT_Y2(1) | OUT_OorB(1) | OUT_EorNOP(0) | OUT_W1orAUX(0);
          else
            out_status_new = OUT_Y1(1) | OUT_Y2(0) | OUT_OorB(1) | OUT_EorNOP(0) | OUT_W1orAUX(0);
        }
        if((OUT_COOL_one_flag != 0x55) && (OUT_COOL_one_flag != 0xaa))
          {
            		OUT_COOL_one_time = cticks_s;		//-��¼ϵͳʱ��,����Ϊ��λ,���Ե�
            		OUT_COOL_one_flag = 0x55;		//-������һ������
          }
        out_flag = 0x55;
    }
    else
    {
				if(temperature_value <= normal_level_max_value)
				{
						out_status_new = OUT_Y1(0) | OUT_Y2(0) | OUT_OorB(1) | OUT_EorNOP(0) | OUT_W1orAUX(0);
            out_hold_flag = 0;
            OUT_COOL_one_flag = 0;
						out_flag = 0x55;
            OUT_HEAT_one_flag = 0;  //-Ϊ���л�������ʱ��ȷ�ĳ�ʼ״̬���г�ʼ��
				}
    }
}

///////////////////////////////////////////////////////////////////////////////

/*
һ���ȱ��ж�:
���������������,���ȿ�����Ҫ����˼��
*/
void one_P_out_judge(void)
{
  out_status_new = 0;
  //-�����趨��ֵ�ĺ�������Ҫ����
  if(RUN_status == 1)
  {//-����
    xx_one_P_heal_judge();
  }
  else if(RUN_status == 2)
  {//-����
    xx_one_P_cool_judge();
  }
  else if(RUN_status == 3)
  {//-�Զ�
      if(temperature_value < normal_level_min_value)
      {//-�Զ�����
          xx_one_P_heal_judge();
      }
      else
      {
          //-if(temperature_value >= normal_level_max_value)  //-ʵ���¶� > �����¶� ��ֹͣ����
          {//-�Զ�����
              xx_one_P_cool_judge();
          }
          //-else
          {

          }
      }
  }
  else
  {//-�쳣����
      out_flag = 0;
  }


	//-
}

/*
һ���ȱ��ж�:
���������������,���ȿ�����Ҫ����˼��
*/
void two_P_out_judge(void)
{
  out_status_new = 0;
  //-�����趨��ֵ�ĺ�������Ҫ����
  if(RUN_status == 1)
  {//-����
    xx_two_P_heal_judge();
  }
  else if(RUN_status == 2)
  {//-����
    xx_two_P_cool_judge();
  }
  else if(RUN_status == 3)
  {//-�Զ�
      if(temperature_value < normal_level_min_value)
      {//-�Զ�����
          xx_two_P_heal_judge();
      }
      else
      {
          //-if(temperature_value >= normal_level_max_value)  //-ʵ���¶� > �����¶� ��ֹͣ����
          {//-�Զ�����
              xx_two_P_cool_judge();
          }
          //-else
          {

          }
      }
  }
  else
  {//-�쳣����
      out_flag = 0;
  }


	//-
}

/*
ֻ����������²ſ�������,
�ⲿ�¶ȵ���3��
���趨�¶ȴﵽ8���ֵ��ʱ��
�ر�ʱ��Y1 Y2�ر�
*/
void one_PaAUX_out_judge(void)
{
  out_status_new = 0;
  //-�����趨��ֵ�ĺ�������Ҫ����
  if(RUN_status == 1)
  {//-����
     xx_one_PaAUX_heal_judge();
  }
  else if(RUN_status == 2)
  {//-����
    xx_one_PaAUX_cool_judge();
  }
  else if(RUN_status == 3)
  {//-�Զ�
      if(temperature_value < normal_level_min_value)
      {//-�Զ�����
          xx_one_PaAUX_heal_judge();
      }
      else
      {
          //-if(temperature_value >= normal_level_max_value)  //-ʵ���¶� > �����¶� ��ֹͣ����
          {//-�Զ�����
              xx_one_PaAUX_cool_judge();
          }
          //-else
          {

          }
      }
  }
  else
  {//-�쳣����
      out_flag = 0;
  }


	//-
}

void two_PaAUX_out_judge(void)
{
  out_status_new = 0;
  //-�����趨��ֵ�ĺ�������Ҫ����
  if(RUN_status == 1)
  {//-����
    xx_two_PaAUX_heal_judge();
  }
  else if(RUN_status == 2)
  {//-����
    xx_two_PaAUX_cool_judge();
  }
  else if(RUN_status == 3)
  {//-�Զ�
      if(temperature_value < normal_level_min_value)
      {//-�Զ�����
          xx_two_PaAUX_heal_judge();
      }
      else
      {
          //-if(temperature_value >= normal_level_max_value)  //-ʵ���¶� > �����¶� ��ֹͣ����
          {//-�Զ�����
              xx_two_PaAUX_cool_judge();
          }
          //-else
          {

          }
      }
  }
  else
  {//-�쳣����
      out_flag = 0;
  }


	//-
}

void HEAT_one_two_delay(UINT8 time)   //-�������ʱ�Ǹ���ǰ��ı�־λ���е�,��������Ҫǰ��ı�־���
{
     if(OUT_HEAT_one_flag == 0x55)
     {
         if(Judge_STime_In_MainLoop(OUT_HEAT_one_time,time)==YES)
         {
            OUT_HEAT_one_time = cticks_s;
            OUT_HEAT_one_flag = 0xaa;   //-������ڴ���
            
            //-UART1_transmit_control = 4;
            //-UART1_transmit_flag=YES;            
         }
     }
}

void COOL_one_two_delay(UINT8 time)
{
     if(OUT_COOL_one_flag == 0x55)
     {
         if(Judge_STime_In_MainLoop(OUT_COOL_one_time,time)==YES)
         {
            OUT_COOL_one_time = cticks_s;
            OUT_COOL_one_flag = 0xaa;   //-������ڴ���
            
            //-UART1_transmit_control = 4;
            //-UART1_transmit_flag=YES;            
         }
     }
}

void Y1_protect_judge(UINT16 time)  //-����Ǹ��ݳ������жϵ�,ǰ�治��Ҫ�κα�־λ
{
  UINT16  temp_data;
  
 //-OUT_W1orAUXһ��������,����رպ�,��Ҫ����4���Ӳſ��Ա�����
    if((out_status_new & OUT_Y1_M) == OUT_Y1(1))
    {
      if(OUT_Y1_flag == 0)
        OUT_Y1_flag = 1;
    }
    else
    {
        if((out_status_new & OUT_Y1_M) == OUT_Y1(0))
        {
            if(OUT_Y1_flag == 1) //-1 ��ʾ���ڿ���״̬,��ô���ڹرվ���Ҫ��ʱ��ʼ
            {
              OUT_Y1_flag = 0x55;
              OUT_Y1_time = cticks_s;
              //-OUT_COOL_one_flag = 0;
            }
        }
    }
    if(OUT_Y1_flag == 0x55)
    {//-��Ҫ�ж��Ƿ���ʱ����
        temp_data = out_status_new & (~OUT_Y1_M);
        out_status_new = temp_data | OUT_Y1(0);
        OUT_HEAT_one_flag = 0;  //-ѹ����һ��������ڹر�״̬,��ô��仰�Ϳ���2����Զ���ᶯ��
        OUT_COOL_one_flag = 0;
        if(Judge_STime_In_MainLoop(OUT_Y1_time,time)==YES)
        {
            OUT_Y1_flag = 0;
        }
    }
}

void Y2_protect_judge(UINT16 time)  //-����Ǹ��ݳ������жϵ�,ǰ�治��Ҫ�κα�־λ
{
  UINT16  temp_data;
  
    if((out_status_new & OUT_Y2_M) == OUT_Y2(1))
    {
      if(OUT_Y2_flag == 0)
        OUT_Y2_flag = 1;
    }
    else
    {
        if((out_status_new & OUT_Y2_M) == OUT_Y2(0))
        {
            if(OUT_Y2_flag == 1) //-1 ��ʾ���ڿ���״̬,��ô���ڹرվ���Ҫ��ʱ��ʼ
            {
              OUT_Y2_flag = 0x55;
              OUT_Y2_time = cticks_s;
            }
        }
    }
    if(OUT_Y2_flag == 0x55)
    {//-��Ҫ�ж��Ƿ���ʱ����
        temp_data = out_status_new & (~OUT_Y2_M);
        out_status_new = temp_data | OUT_Y2(0);
        if(Judge_STime_In_MainLoop(OUT_Y2_time,time)==YES)
        {
            OUT_Y2_flag = 0;
        }
    }
}

/*
�������Ķ˿�,����ʱЧ���ж�
������һ����־λ,������Чʱ����ڿ��Զ���
*/
void out_delay_judge(void)
{
  //-UINT16  temp_data;

  if((SYS_WORK_MODE == 0x11) || (SYS_WORK_MODE == 0x21))
  {
     HEAT_one_two_delay(6);
  }
  else if((SYS_WORK_MODE == 0x12) || (SYS_WORK_MODE == 0x22))
  {
     COOL_one_two_delay(6);
     
     Y1_protect_judge(60*2);
    
    Y2_protect_judge(60*2);
  }
  else if((SYS_WORK_MODE == 0x13) || (SYS_WORK_MODE == 0x23))
  {//-ѹ��������
    Y1_protect_judge(60*2);
  }
  else if((SYS_WORK_MODE == 0x14) || (SYS_WORK_MODE == 0x24))
  {
    COOL_one_two_delay(6);
    
    Y1_protect_judge(60*2);
    
    Y2_protect_judge(60*2);
  }
  else if((SYS_WORK_MODE == 0x15) || (SYS_WORK_MODE == 0x25))
  {
    HEAT_one_two_delay(6);
    
    if(run_heat_or_cool == 2) //-��Щ������,ֻ��������Ҫ����   
      Y1_protect_judge(60*2);
  }
  else if((SYS_WORK_MODE == 0x16) || (SYS_WORK_MODE == 0x26))
  {
    HEAT_one_two_delay(6);
    COOL_one_two_delay(6);
    
    if(run_heat_or_cool == 2) //-��Щ������,ֻ��������Ҫ���� 
    {
      Y1_protect_judge(60*2);
      Y2_protect_judge(60*2);
    }
  }
  else if((SYS_WORK_MODE == 0x31) || (SYS_WORK_MODE == 0x41))
  {
    Y1_protect_judge(60*2);
    Y2_protect_judge(60*2);
  }
  else if((SYS_WORK_MODE == 0x32) || (SYS_WORK_MODE == 0x42))
  {
    HEAT_one_two_delay(6);
    COOL_one_two_delay(6);
    
    Y1_protect_judge(60*2);
    Y2_protect_judge(60*2);
  }
  else if(SYS_WORK_MODE == 0x33)
  {
    Y1_protect_judge(60*2);
    //-���Ӹ����ж�
    
  }
  else if(SYS_WORK_MODE == 0x34)
  {
    HEAT_one_two_delay(6);
    COOL_one_two_delay(6);
    
    Y1_protect_judge(60*2);
    Y2_protect_judge(60*2);
    //-���Ӹ����ж�
    
  }
  else if(SYS_WORK_MODE == 0x43)
  {
    Y1_protect_judge(60*2);
    
  }
  else if(SYS_WORK_MODE == 0x44)
  {
    HEAT_one_two_delay(6);
    COOL_one_two_delay(6);
    
    Y1_protect_judge(60*2);
    Y2_protect_judge(60*2);
    
  }
  else
  {
    
  }
}

void programme_judge(void)
{
  UINT8 i,j,max=0;
  UINT16  temp_min;

  if((RUN_status >=1) && (RUN_status <= 3))
  {
    j = RUN_status - 1;   //-��������ģʽ��λ��������
    j = 7*j + m_week;
  }
  else
    return;

  temp_min = m_hour * 60 + m_min;

  if(RUN_status == 3)
  {
    menu_set_tt_max = auto_set_tt_max_eep/10;
    menu_set_tt_min = auto_set_tt_min_eep/10;  
  }
  else
  {
    menu_set_tt_max = menu_set_tt_max_eep/10;
    menu_set_tt_min = menu_set_tt_min_eep/10;
  }
  
  for(i = 0;i < PROGRAMME_DAY_dot_NUM;i++)
  {//-Ѱ�ҵ��ض�һ�ű���е����ݸ���
    if(programme_day[j].day_dot[i].MIN > 1440)
      break;
    max = i+1;
  }

  for(i = 0;i < max;i++)
  {
    if((i + 1) < max)
    {
      if(programme_day[j].day_dot[i].MIN <= temp_min)
      {//-û�л��ֵһ����Чֵ
         if(programme_day[j].day_dot[i+1].MIN <= temp_min)
           continue;
         else
         {//-�����ҵ�����Ч����ֵ
           if((programme_day[j].day_dot[i].MIN > now_programme_time)/* || (now_programme_flag == 0)*/)
           {//-�����Чʱ����ڱ�̵���ִ��,�����̵����ȼ���
            menu_set_tt_max = programme_day[j].day_dot[i].MAX_VOULE * 10;
            menu_set_tt_min = programme_day[j].day_dot[i].MIN_VOULE * 10;
            now_programme_flag = 1;
            if(now_programme_redata != 0x55)
            {
              UART1_transmit_control = 4;
              UART1_transmit_flag=YES;
              now_programme_redata = 0x55;
            }
           }
           else
           {//-�������ʱ�������Ϊ��Ԥ��,���Բ����ñ��ֵ,��Ϊ��Ԥ���ȼ���
              now_programme_flag = 0;
              now_programme_redata = 0;
           }
         }
      }
    }
    else
    {//-��������һ�����ò�����
      if(programme_day[j].day_dot[i].MIN <= temp_min)
      {
        if((programme_day[j].day_dot[i].MIN > now_programme_time)/* || (now_programme_flag == 0)*/)
        {
          menu_set_tt_max = programme_day[j].day_dot[i].MAX_VOULE * 10;
          menu_set_tt_min = programme_day[j].day_dot[i].MIN_VOULE * 10;
          now_programme_flag = 1;
          if(now_programme_redata != 0x55)
            {
              UART1_transmit_control = 4;
              UART1_transmit_flag=YES;
              now_programme_redata = 0x55;
            }
        }
        else
        {
          now_programme_flag = 0;
          now_programme_redata = 0;
        }
      }
      //-else
      //-  menu_set_tt_max = menu_set_tt_max_eep;

    }
  }

}

//-���ݸ��ȵ�״̬������ѹ����
void Yx_control_by_AUX(void)
{
  UINT16 		temp_data;
  
    if((out_status_new & OUT_W1orAUX_M) == OUT_W1orAUX(1))
    {
        temp_data = out_status_new & ~(OUT_Y1_M | OUT_Y2_M);
        out_status_new = temp_data | OUT_Y1(0) | OUT_Y2(0);
    }
}

void init_out_sys(void)   //-������еı�������Ҫ������ȷ��ֵ,���ܿ�Ĭ�ϸ�ֵ
{
    OUT_HEAT_one_flag = 0;  //-0˵��û�п���һ������,�ȴ�һ�����ȿ����ж�
    OUT_COOL_one_flag = 0;  //-0˵��û�п���һ������,�ȴ�һ�����俪��,���������ܵ���ʱ�ж�
    //-out_flag = 0;        //-����ĺ��������ڵ���,��ô����������,����ھͲ���ˢ����
    out_hold_flag = 0;      //-���Ǳ�֤һ������ͬʱ�ջص�,�����ʼ����,�͸ø�0
    
    
}

/*
�����趨��ϵͳģʽֵѡ����ڵ��жϷ�֧
*/
void sys_mode_judge(void)
{
  //-UINT8 		temp_data;
  //-static UINT8 OUT_CN;
  run_heat_or_cool = 0;   //-����ģʽ,�������κ��߼��ж�
  E_start_flag = 0;
  
  if(temperature_data_flag == 1)
    temperature_value = 0;
  else
    temperature_value = temperature_data_x10;

  programme_judge();  //-��ȡ��ǰ��Ҫ�ıȽ��¶�

  //-���߼��ж�֮ǰ,�ж��¹���ģʽ�Ƿ��޸���,������޸�,һЩ������Ҫ���³�ʼ��
  if(RUN_status_flag == 0x55)
  {
     init_out_sys();
     RUN_status_flag = 0;
  }
  
	//-ģʽ��Ϊ������,��һ���ָ���λ��ʾ,�Ӳ��ֵ���λ��ʾ
	if((SYS_WORK_MODE & 0xf0) == 0x10)
	{

    if(SYS_WORK_MODE == 0x11)	//-��ͨ�ӵ���
		{//-��������
			H2_out_judge();
		}
    else if(SYS_WORK_MODE == 0x12)	//-��ͨ�ӵ���
		{//-��������
			C2_out_judge();
		}
		else if(SYS_WORK_MODE == 0x13)	//-��ͨ�ӵ���
		{//-һ������һ������
			C1H1_out_judge();
		}
		else if(SYS_WORK_MODE == 0x14)
		{//-һ�����ȶ�������
			C2H1_out_judge();
		}
		else if(SYS_WORK_MODE == 0x15)
		{//-��������һ������
			C1H2_out_judge();
		}
		else if(SYS_WORK_MODE == 0x16)
		{//-�������ȶ�������
			C2H2_out_judge();
		}


	}
	else if((SYS_WORK_MODE & 0xf0) == 0x20)
	{        
    if(SYS_WORK_MODE == 0x21)	//-��ͨ��ȼ��
		{//-��������
			H2_out_judge(); //-������ڼ�����,���Ȳ�����ر�,
		}
    else if(SYS_WORK_MODE == 0x22)	//-��ͨ��ȼ��
		{//-��������
			C2_out_judge();
		}
		else if(SYS_WORK_MODE == 0x23)	//-��ͨ��ȼ��
		{//-һ������һ������
			C1H1_out_judge();
		}
		else if(SYS_WORK_MODE == 0x24)
		{//-һ�����ȶ�������
			C2H1_out_judge();
		}
		else if(SYS_WORK_MODE == 0x25)
		{//-��������һ������
			C1H2_out_judge();
		}
		else if(SYS_WORK_MODE == 0x26)
		{//-�������ȶ�������
			C2H2_out_judge();
		}
	}
	else if((SYS_WORK_MODE & 0xf0) == 0x30)
	{

		if(SYS_WORK_MODE == 0x31)	//-�ȱüӵ�Դ  == ����Դ�ȱ�
		{//-һ���ȱ�
      one_P_out_judge();
		}
		else if(SYS_WORK_MODE == 0x32)
		{//-�����ȱ�
      two_P_out_judge();
		}
		else if(SYS_WORK_MODE == 0x33)
		{//-һ���ȱ�+����
      one_PaAUX_out_judge();
		}
		else if(SYS_WORK_MODE == 0x34)
		{//-�����ȱ�+����
      two_PaAUX_out_judge();
		}
		/*else if(SYS_WORK_MODE == 0x35)
		{//-һ���ȱ�+��������

		}
		else if(SYS_WORK_MODE == 0x36)
		{//-�����ȱ�+��������

		}*/
	}
	else if((SYS_WORK_MODE & 0xf0) == 0x40)
	{
		if(SYS_WORK_MODE == 0x41)	//-�ȱü�ȼ��  ==  ˫��Դ�ȱ�
		{//-һ���ȱ�
        one_P_out_judge();
		}
		else if(SYS_WORK_MODE == 0x42)
		{//-�����ȱ�
        two_P_out_judge();
		}
		else if(SYS_WORK_MODE == 0x43)
		{//-һ���ȱ�+����
        one_PaAUX_out_judge();
        Yx_control_by_AUX();
		}
		else if(SYS_WORK_MODE == 0x44)
		{//-�����ȱ�+����
        two_PaAUX_out_judge();
        Yx_control_by_AUX();
		}
		/*else if(SYS_WORK_MODE == 0x45)
		{//-һ���ȱ�+��������

		}
		else if(SYS_WORK_MODE == 0x46)
		{//-�����ȱ�+��������

		}*/
	}
	else
	{

	}

  if(RUN_ONOFF_status == 0)
  {
    out_status_new = OUT_HEAT_one(0) | OUT_HEAT_two(0) | OUT_NOP_one(0) | OUT_COOL_one(0) | OUT_COOL_two(0) | OUT_NOP_two(0);
    init_out_sys();
  }
  
  out_delay_judge();
  
  if(E_start_flag == 0)
  {//-˵����������û������
    OUT_emergency_hot = 0;
    E_out_flag = 0;
  }
  //-if(OUT_emergency_hot == 1)
  //-{
  //-  out_flag = 0x55;
  //-  out_status_new = OUT_EorNOP(1);   //-������л����˸���OUT_W1orAUX
  //-}

  //-������
  //-out_flag = 0x55;
  //-out_status_new = OUT_HEAT_one(1) | OUT_HEAT_two(1) | OUT_COOL_one(1) | OUT_COOL_two(1);
  fan_out_judge();

  if(out_status != out_status_new)
  {
    out_onoff_wait_time = cticks_5ms;
    out_onoff_flag = 0x55;
  }
  
  if(out_onoff_flag == 0x55) 
    out_control();
  else
  {
    //-S595_OE((BitAction)1);
    out_status_new = 0;
    S595_out(out_status_new);
    out_flag = 0;
  }
}


/******************* (C) COPYRIGHT 2015 WuLianGroup ********END OF FIL*********/