/**
  ******************************************************************************
  * @file    bsp_led.c
  * @author  Wireless Protocol Stack Develop-Dept.
  * @version --
  * @date    2015/12/22 星期二,Administrator
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
U6输出接U10

U6	U10
U4	U5	U7	U8	U9	U11	U12

从上倒下：
左边：RH-W1/AUX-W2-E/?-GND-T1
右边：RC-Y1-Y2-G-O/B-C

左边：电源，一级制热，二级制热，预留端子，外部温度传感器地，外部温度传感器ADC采集端
右边：电源，压缩机一级输出，压缩机二级输出，风扇，四通阀换向控制端，电源公共端

左边: 电源(24V),   U11,  U12,   U9,    GND,    ADC
右边: 电源(空),    U4,   U5,    U7,    U8,     电源公共地

*/

/* Includes ------------------------------------------------------------------*/

#include "user_conf.h"

//#define S595_OE(BitVal)  			GPIO_WriteBit(GPIOA,GPIO_Pin_11,BitVal)  //-OE 第13脚 输出有效(低电平)
//#define S595_SHCP(BitVal)  		GPIO_WriteBit(GPIOC,GPIO_Pin_7,BitVal)  //-SHCP 第11脚 移位寄存器时钟输入
//#define S595_STCP(BitVal)  		GPIO_WriteBit(GPIOC,GPIO_Pin_6,BitVal)  //-STCP 第12脚 存储寄存器时钟输入,通常置为0,当移位好了之后一个正脉冲,更新数据
//#define S595_SDI(BitVal)  		GPIO_WriteBit(GPIOC,GPIO_Pin_8,BitVal)  //-DS 第14脚 串行数据输入


#define S595_STCP(BitVal)  			GPIO_WriteBit(GPIOC,GPIO_Pin_6,BitVal)  //-OE 第13脚 输出有效(低电平)
#define S595_SDI(BitVal)  		  GPIO_WriteBit(GPIOC,GPIO_Pin_8,BitVal)  //-SHCP 第11脚 移位寄存器时钟输入
#define S595_OE(BitVal)  		    GPIO_WriteBit(GPIOA,GPIO_Pin_11,BitVal)  //-STCP 第12脚 存储寄存器时钟输入,通常置为0,当移位好了之后一个正脉冲,更新数据
#define S595_SHCP(BitVal)  		  GPIO_WriteBit(GPIOC,GPIO_Pin_7,BitVal)  //-DS 第14脚 串行数据输入

static UINT16 temperature_value;  //-取得实际需要比较的温度数值
static UINT8  run_heat_or_cool;   //-表示目前处于制热或制冷模式

/**
  * @fun    void Send_Bit
  * @brief
  *         2015/12/22 星期二,Administrator
  * @param  None
  *
  * @retval
  */
void S595_Send_Bit(u8 data)
{
  S595_SDI((BitAction)(data&0x01));
  //-翻转时钟信号上升沿时采用
  S595_SHCP((BitAction)1);
  //-NOP_Delay(10);
  S595_SHCP((BitAction)0);
}

/**
  * @fun    void Send_Byte
  * @brief
  *         2015/12/22 星期二,Administrator
  * @param  u32 data
  * @param  u8 length
  *一列的数据送到总线上
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
  *         2015/12/22 星期二,Administrator
  * @param  u8 x
  * @param  u32 y
  *
  * @retval
  */
void S595_out(u16 date)
{
  S595_OE((BitAction)0);
  S595_Send_Byte(date,14);
  //-下面更新数据
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
  {//?可以一直输出吗,万一输出一次失败怎么办,还是周期性的输出比较可靠呢?
      S595_out(out_status_new);
      out_status = out_status_new;
      out_status_new = 0;
      out_flag = 0;
  }
}

void fan_out_judge(void)
{
  UINT16 temp_data;
  	     
    //-在次之前没有对风扇有过任何逻辑处理
    if((SYS_WORK_MODE & 0xf0) == 0x10)
    {
      if(((out_status_new & OUT_W1orAUX_M) == OUT_W1orAUX(1)) || ((out_status_new & OUT_W2_M) == OUT_W2(1)) || ((out_status_new & OUT_Y1_M) == OUT_Y1(1)) || ((out_status_new & OUT_Y2_M) == OUT_Y2(1)))
      {
          out_status_new |= OUT_wind(1);
          menu_wind_status = 0x51;  //-表示目前处于强制开启状态
          out_flag = 0x55;
      }
      else
      {
        if(((menu_wind_status & 0xf0) == 0x50) && (out_flag == 0x55)) //-只有出口更新了,才会考虑收回风扇
        {
          out_status_new |= OUT_wind(0);
          menu_wind_status = 0x50;  //-表示处于关闭但人为可控状态
          
        }
      }
    }
    
    if((SYS_WORK_MODE & 0xf0) == 0x20)
    {//-1.	普通模式多能源系统,制热开启时候，风扇不随着制热开启而开启，可以手动控制。
      if(((out_status_new & OUT_Y1_M) == OUT_Y1(1)) || ((out_status_new & OUT_Y2_M) == OUT_Y2(1)))
      {
          out_status_new |= OUT_wind(1);
          menu_wind_status = 0x51;  //-表示目前处于强制开启状态
          out_flag = 0x55;
      }
      else
      {
        if(((menu_wind_status & 0xf0) == 0x50) && (out_flag == 0x55)) //-只有出口更新了,才会考虑收回风扇
        {
          out_status_new |= OUT_wind(0);
          menu_wind_status = 0x50;  //-表示处于关闭但人为可控状态
          
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
           menu_wind_status = 0x50;  //-表示处于关闭但人为可控状态
           
         }
       }
    }

    if((SYS_WORK_MODE & 0xf0) == 0x40)
    {//-2.	热泵模式多能源系统，辅热开启时候，风扇不随着辅热开启而开启，可以手动开启。热泵多能源，紧急制热因为是用得辅热应该也不开风扇
      if(((SYS_WORK_MODE & 0x0f) == 0x01) || ((SYS_WORK_MODE & 0x0f) == 0x02))
      {
        if(((out_status_new & OUT_Y1_M) == OUT_Y1(1)) || ((out_status_new & OUT_Y2_M) == OUT_Y2(1)))
        {//-增加 没有辅热情况下对风扇的控制
            out_status_new |= OUT_wind(1);
            menu_wind_status = 0x51;  //-表示目前处于强制开启状态
            out_flag = 0x55;
        }
        else
        {
          if(((menu_wind_status & 0xf0) == 0x50) && (out_flag == 0x55))
          {
            out_status_new |= OUT_wind(0);
            menu_wind_status = 0x50;  //-表示处于关闭但人为可控状态
            
          }
        }
      }
      else
      {
        if((((out_status_new & OUT_Y1_M) == OUT_Y1(1)) || ((out_status_new & OUT_Y2_M) == OUT_Y2(1))) && ((out_status_new & OUT_W1orAUX_M) == OUT_W1orAUX(0)))
        {
            out_status_new |= OUT_wind(1);
            menu_wind_status = 0x51;  //-表示目前处于强制开启状态
            out_flag = 0x55;
        } 
        else
        {
          if(((menu_wind_status & 0xf0) == 0x50) && (out_flag == 0x55))
          {
            out_status_new |= OUT_wind(0);
            menu_wind_status = 0x50;  //-表示处于关闭但人为可控状态
            
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
	    {//-这里可能就需要注释掉,让其他逻辑决定风扇状态
        temp_data = out_status_new & (~OUT_G_M);
        out_status_new = temp_data | OUT_wind(0);
	      out_flag = 0x55;
	    }
	  }
}

///////////////////////////////////////////////////////////////////////////////
//-定义了比较数值
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
            //-		OUT_HEAT_one_time = cticks_s;		//-记录系统时间,以秒为单位,线性的
            //-		OUT_HEAT_one_flag = 0x55;		//-开启了一级制热
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
      	  {//-还没有达到可靠区域,不改变任何出口

					}
      }
}

void xx_H2_judge(void)
{
      if(temperature_value <= one_level_min_value)
      {
          if(temperature_value <= two_level_min_value)
          {//-美标一级制热和二级制热是可以同时打开的
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
            		OUT_HEAT_one_time = cticks_s;		//-记录系统时间,以秒为单位,线性的
            		OUT_HEAT_one_flag = 0x55;		//-开启了一级制热
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
              OUT_COOL_one_flag = 0;  //-同样类型切换的时候,也需要清除其它类型的标志,保证初始状态正确.
      	  }
      	  else
      	  {//-还没有达到可靠区域,不改变任何出口

					}
      }
}

void xx_C1_judge(void)
{
    if(temperature_value >= one_level_max_value)  //-实际温度 > 设置温度 就停止加热
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
    if(temperature_value >= one_level_max_value)  //-实际温度 > 设置温度 就停止加热
    {
        if(temperature_value >= two_level_max_value)  //-超过阀值域开启二级制热
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
            		OUT_COOL_one_time = cticks_s;		//-记录系统时间,以秒为单位,线性的
            		OUT_COOL_one_flag = 0x55;		//-开启了一级制热
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
            OUT_HEAT_one_flag = 0;  //-制冷判断的时候,制热的标志也需要清除,这样在无规则跳动的时候才会避免错误
				}
    }
}

///////////////////////////////////////////////////////////////////////////////

/*
先起W1延时6S启W2,关闭一起关,再次启动W12没有延时保护.
*/
void H2_out_judge(void)
{
  out_status_new = 0;
  //-对于设定定值的合理性需要考虑
  if(RUN_status == 1)
  {//-制热
      xx_H2_judge();
  }
  else
  {//-异常处理
      OUT_HEAT_one_flag = 0;
      out_flag = 0;
  }
	//-
}

/*
先开Y1延时6S启动Y2,关闭一起关闭,关闭后压缩机保护两分钟,才可以第二次开启
*/
void C2_out_judge(void)
{
  out_status_new = 0;
  //-对于设定定值的合理性需要考虑
  if(RUN_status == 2)
  {//-制冷
    xx_C2_judge();
  }
  else
  {//-异常处理
      out_flag = 0;
  }




	//-
}

/*
制热没有任何保护,制冷需要2分钟压缩机保护,三种模式情况一样
*/
void C1H1_out_judge(void)
{
  out_status_new = 0;
  //-对于设定定值的合理性需要考虑
  if(RUN_status == 1)
  {//-制热
    run_heat_or_cool = 1;      
  }
  else if(RUN_status == 2)
  {//-制冷
    run_heat_or_cool = 2;    
  }
  else if(RUN_status == 3)
  {//-自动
      if(temperature_value < normal_level_min_value)
      {//-自动制热
        run_heat_or_cool = 1;
      }
      else
      {
          //-if(temperature_value >= normal_level_max_value)  //-实际温度 > 设置温度 就停止加热
          //-自动制冷
            run_heat_or_cool = 2;              
      }
  }
  else
  {//-异常处理
      out_flag = 0;
  }

  if(run_heat_or_cool == 1)
  {//-制热
    xx_H1_judge();
  }
  else if(run_heat_or_cool == 2)
  {//-制冷
    xx_C1_judge();
  }
  else
  {//-异常处理
      out_flag = 0;
  }
    


	//-
}

void C1H2_out_judge(void)
{
  out_status_new = 0;
  //-对于设定定值的合理性需要考虑
  if(RUN_status == 1)
  {//-制热
    run_heat_or_cool = 1;
  }
  else if(RUN_status == 2)
  {//-制冷
    run_heat_or_cool = 2;     
  }
  else if(RUN_status == 3)
  {//-自动
      if(temperature_value < normal_level_min_value)
      {//-自动制热
        run_heat_or_cool = 1; 
      }
      else
      {
          //-if(temperature_value >= normal_level_max_value)  //-实际温度 > 设置温度 就停止加热
          //-自动制冷
            run_heat_or_cool = 2; 

      }
  }
  else
  {//-异常处理
      out_flag = 0;
  }

  if(run_heat_or_cool == 1)
  {//-制热
    xx_H2_judge();
  }
  else if(run_heat_or_cool == 2)
  {//-制冷
    xx_C1_judge();
  }
  else
  {//-异常处理
      out_flag = 0;
  }



	//-
}

void C2H1_out_judge(void)
{
  out_status_new = 0;
  //-对于设定定值的合理性需要考虑
  if(RUN_status == 1)
  {//-制热
    run_heat_or_cool = 1;    
  }
  else if(RUN_status == 2)
  {//-制冷
    run_heat_or_cool = 2;    
  }
  else if(RUN_status == 3)
  {//-自动
      if(temperature_value < normal_level_min_value)
      {//-自动制热
        run_heat_or_cool = 1;
      }
      else
      {
          //-if(temperature_value >= normal_level_max_value)  //-实际温度 > 设置温度 就停止加热
          //-自动制冷
            run_heat_or_cool = 2;
      }
  }
  else
  {//-异常处理
      out_flag = 0;
  }

  if(run_heat_or_cool == 1)
  {//-制热
    xx_H1_judge();
  }
  else if(run_heat_or_cool == 2)
  {//-制冷
    xx_C2_judge();
  }
  else
  {//-异常处理
      out_flag = 0;
  }


	//-
}

/*
实际温度 > 设置温度 就停止加热

上电第一步:保证出口不动
其次最所有的出口进行控制判断

1.温度在阀值上下快速波动怎么办?
一但启动五分钟之后才考虑关闭
用户有一个设定值menu_set_tt_cool 然后menu_set_tt_swing是一个摇摆值,防止机器反复抖动的
2.程序中需要考虑定值合理性检测?
*/
void C2H2_out_judge(void)
{
  out_status_new = 0;
  //-对于设定定值的合理性需要考虑
  if(RUN_status == 1)
  {//-制热
    run_heat_or_cool = 1;     
  }
  else if(RUN_status == 2)
  {//-制冷
    run_heat_or_cool = 2;
  }
  else if(RUN_status == 3)
  {//-自动
      if(temperature_value < normal_level_min_value)
      {//-自动制热
        run_heat_or_cool = 1;
      }
      else
      {
          //-if(temperature_value >= normal_level_max_value)  //-实际温度 > 设置温度 就停止加热
          //-自动制冷
            run_heat_or_cool = 2;
      }
  }
  else
  {//-异常处理
      out_flag = 0;
  }

  if(run_heat_or_cool == 1)
  {//-制热
    xx_H2_judge();
  }
  else if(run_heat_or_cool == 2)
  {//-制冷
    xx_C2_judge();
  }
  else
  {//-异常处理
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
            //-		OUT_HEAT_one_time = cticks_s;		//-记录系统时间,以秒为单位,线性的
            //-		OUT_HEAT_one_flag = 0x55;		//-开启了一级制热
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
      	  {//-还没有达到可靠区域,不改变任何出口

					}
      }
}

void xx_one_P_cool_judge(void)
{
    if(temperature_value >= one_level_max_value)  //-实际温度 > 设置温度 就停止加热
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

void xx_two_P_heal_judge(void)  //-先开Y1延时6S启动Y2,关闭一起关闭,关闭后压缩机保护两分钟,才可以第二次开启
{
      if(temperature_value <= one_level_min_value)
      {
          if(temperature_value <= two_level_min_value)
          {//-美标一级制热和二级制热是可以同时打开的
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
            		OUT_HEAT_one_time = cticks_s;		//-记录系统时间,以秒为单位,线性的
            		OUT_HEAT_one_flag = 0x55;		//-开启了一级制热
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
      	  {//-还没有达到可靠区域,不改变任何出口

					}
      }
}

void xx_two_P_cool_judge(void)
{
    if(temperature_value >= one_level_max_value)  //-实际温度 > 设置温度 就停止加热
    {
        if(temperature_value >= two_level_max_value)  //-超过阀值域开启二级制热
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
            		OUT_COOL_one_time = cticks_s;		//-记录系统时间,以秒为单位,线性的
            		OUT_COOL_one_flag = 0x55;		//-开启了一级制热
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
          {//-美标一级制热和二级制热是可以同时打开的
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
            //-		OUT_HEAT_one_time = cticks_s;		//-记录系统时间,以秒为单位,线性的
            //-		OUT_HEAT_one_flag = 0x55;		//-开启了一级制热
          	//-}
          }
          
          
          if((temperature_data_out < 30) || (temperature_data_out & 0x8000))  //-这里必须要考虑负数的问题
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
      	  {//-还没有达到可靠区域,不改变任何出口

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
    if(temperature_value >= one_level_max_value)  //-实际温度 > 设置温度 就停止加热
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
          {//-美标一级制热和二级制热是可以同时打开的
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
            		OUT_HEAT_one_time = cticks_s;		//-记录系统时间,以秒为单位,线性的
            		OUT_HEAT_one_flag = 0x55;		//-开启了一级制热
          }
          if((temperature_data_out < 30) || (temperature_data_out & 0x8000)) //-这里必须要考虑负数的问题
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
      	  {//-还没有达到可靠区域,不改变任何出口

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
    if(temperature_value >= one_level_max_value)  //-实际温度 > 设置温度 就停止加热
    {
        if(temperature_value >= two_level_max_value)  //-超过阀值域开启二级制热
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
            		OUT_COOL_one_time = cticks_s;		//-记录系统时间,以秒为单位,线性的
            		OUT_COOL_one_flag = 0x55;		//-开启了一级制热
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
            OUT_HEAT_one_flag = 0;  //-为了切换回制热时正确的初始状态进行初始化
				}
    }
}

///////////////////////////////////////////////////////////////////////////////

/*
一级热泵判断:
包括制冷制热输出,风扇考虑需要另外思考
*/
void one_P_out_judge(void)
{
  out_status_new = 0;
  //-对于设定定值的合理性需要考虑
  if(RUN_status == 1)
  {//-制热
    xx_one_P_heal_judge();
  }
  else if(RUN_status == 2)
  {//-制冷
    xx_one_P_cool_judge();
  }
  else if(RUN_status == 3)
  {//-自动
      if(temperature_value < normal_level_min_value)
      {//-自动制热
          xx_one_P_heal_judge();
      }
      else
      {
          //-if(temperature_value >= normal_level_max_value)  //-实际温度 > 设置温度 就停止加热
          {//-自动制冷
              xx_one_P_cool_judge();
          }
          //-else
          {

          }
      }
  }
  else
  {//-异常处理
      out_flag = 0;
  }


	//-
}

/*
一级热泵判断:
包括制冷制热输出,风扇考虑需要另外思考
*/
void two_P_out_judge(void)
{
  out_status_new = 0;
  //-对于设定定值的合理性需要考虑
  if(RUN_status == 1)
  {//-制热
    xx_two_P_heal_judge();
  }
  else if(RUN_status == 2)
  {//-制冷
    xx_two_P_cool_judge();
  }
  else if(RUN_status == 3)
  {//-自动
      if(temperature_value < normal_level_min_value)
      {//-自动制热
          xx_two_P_heal_judge();
      }
      else
      {
          //-if(temperature_value >= normal_level_max_value)  //-实际温度 > 设置温度 就停止加热
          {//-自动制冷
              xx_two_P_cool_judge();
          }
          //-else
          {

          }
      }
  }
  else
  {//-异常处理
      out_flag = 0;
  }


	//-
}

/*
只有制热情况下才开启辅热,
外部温度低于3°
或当设定温度达到8°差值的时候
关闭时和Y1 Y2关闭
*/
void one_PaAUX_out_judge(void)
{
  out_status_new = 0;
  //-对于设定定值的合理性需要考虑
  if(RUN_status == 1)
  {//-制热
     xx_one_PaAUX_heal_judge();
  }
  else if(RUN_status == 2)
  {//-制冷
    xx_one_PaAUX_cool_judge();
  }
  else if(RUN_status == 3)
  {//-自动
      if(temperature_value < normal_level_min_value)
      {//-自动制热
          xx_one_PaAUX_heal_judge();
      }
      else
      {
          //-if(temperature_value >= normal_level_max_value)  //-实际温度 > 设置温度 就停止加热
          {//-自动制冷
              xx_one_PaAUX_cool_judge();
          }
          //-else
          {

          }
      }
  }
  else
  {//-异常处理
      out_flag = 0;
  }


	//-
}

void two_PaAUX_out_judge(void)
{
  out_status_new = 0;
  //-对于设定定值的合理性需要考虑
  if(RUN_status == 1)
  {//-制热
    xx_two_PaAUX_heal_judge();
  }
  else if(RUN_status == 2)
  {//-制冷
    xx_two_PaAUX_cool_judge();
  }
  else if(RUN_status == 3)
  {//-自动
      if(temperature_value < normal_level_min_value)
      {//-自动制热
          xx_two_PaAUX_heal_judge();
      }
      else
      {
          //-if(temperature_value >= normal_level_max_value)  //-实际温度 > 设置温度 就停止加热
          {//-自动制冷
              xx_two_PaAUX_cool_judge();
          }
          //-else
          {

          }
      }
  }
  else
  {//-异常处理
      out_flag = 0;
  }


	//-
}

void HEAT_one_two_delay(UINT8 time)   //-这里的延时是根据前面的标志位进行的,这样就需要前面的标志配合
{
     if(OUT_HEAT_one_flag == 0x55)
     {
         if(Judge_STime_In_MainLoop(OUT_HEAT_one_time,time)==YES)
         {
            OUT_HEAT_one_time = cticks_s;
            OUT_HEAT_one_flag = 0xaa;   //-防错后期处理
            
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
            OUT_COOL_one_flag = 0xaa;   //-防错后期处理
            
            //-UART1_transmit_control = 4;
            //-UART1_transmit_flag=YES;            
         }
     }
}

void Y1_protect_judge(UINT16 time)  //-这个是根据出口来判断的,前面不需要任何标志位
{
  UINT16  temp_data;
  
 //-OUT_W1orAUX一旦被开启,如果关闭后,需要经过4分钟才可以被开启
    if((out_status_new & OUT_Y1_M) == OUT_Y1(1))
    {
      if(OUT_Y1_flag == 0)
        OUT_Y1_flag = 1;
    }
    else
    {
        if((out_status_new & OUT_Y1_M) == OUT_Y1(0))
        {
            if(OUT_Y1_flag == 1) //-1 表示处于开启状态,那么现在关闭就需要计时开始
            {
              OUT_Y1_flag = 0x55;
              OUT_Y1_time = cticks_s;
              //-OUT_COOL_one_flag = 0;
            }
        }
    }
    if(OUT_Y1_flag == 0x55)
    {//-需要判断是否延时到了
        temp_data = out_status_new & (~OUT_Y1_M);
        out_status_new = temp_data | OUT_Y1(0);
        OUT_HEAT_one_flag = 0;  //-压缩机一级如果处于关闭状态,那么这句话就控制2级永远不会动作
        OUT_COOL_one_flag = 0;
        if(Judge_STime_In_MainLoop(OUT_Y1_time,time)==YES)
        {
            OUT_Y1_flag = 0;
        }
    }
}

void Y2_protect_judge(UINT16 time)  //-这个是根据出口来判断的,前面不需要任何标志位
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
            if(OUT_Y2_flag == 1) //-1 表示处于开启状态,那么现在关闭就需要计时开始
            {
              OUT_Y2_flag = 0x55;
              OUT_Y2_time = cticks_s;
            }
        }
    }
    if(OUT_Y2_flag == 0x55)
    {//-需要判断是否延时到了
        temp_data = out_status_new & (~OUT_Y2_M);
        out_status_new = temp_data | OUT_Y2(0);
        if(Judge_STime_In_MainLoop(OUT_Y2_time,time)==YES)
        {
            OUT_Y2_flag = 0;
        }
    }
}

/*
针对特殊的端口,进行时效性判断
采用置一个标志位,超过有效时间出口可以动作
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
  {//-压缩机保护
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
    
    if(run_heat_or_cool == 2) //-这些类型下,只有制冷需要保护   
      Y1_protect_judge(60*2);
  }
  else if((SYS_WORK_MODE == 0x16) || (SYS_WORK_MODE == 0x26))
  {
    HEAT_one_two_delay(6);
    COOL_one_two_delay(6);
    
    if(run_heat_or_cool == 2) //-这些类型下,只有制冷需要保护 
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
    //-增加辅热判断
    
  }
  else if(SYS_WORK_MODE == 0x34)
  {
    HEAT_one_two_delay(6);
    COOL_one_two_delay(6);
    
    Y1_protect_judge(60*2);
    Y2_protect_judge(60*2);
    //-增加辅热判断
    
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
    j = RUN_status - 1;   //-根据运行模式定位到哪类表格
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
  {//-寻找到特定一张表格中的数据个数
    if(programme_day[j].day_dot[i].MIN > 1440)
      break;
    max = i+1;
  }

  for(i = 0;i < max;i++)
  {
    if((i + 1) < max)
    {
      if(programme_day[j].day_dot[i].MIN <= temp_min)
      {//-没有获得值一个有效值
         if(programme_day[j].day_dot[i+1].MIN <= temp_min)
           continue;
         else
         {//-这里找到了有效的数值
           if((programme_day[j].day_dot[i].MIN > now_programme_time)/* || (now_programme_flag == 0)*/)
           {//-这个有效时间点在编程点后就执行,否则编程点优先级高
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
           {//-由于这个时间段内人为干预了,所以不采用编程值,人为干预优先级高
              now_programme_flag = 0;
              now_programme_redata = 0;
           }
         }
      }
    }
    else
    {//-这个是最后一个设置参数了
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

//-根据辅热的状态来控制压缩机
void Yx_control_by_AUX(void)
{
  UINT16 		temp_data;
  
    if((out_status_new & OUT_W1orAUX_M) == OUT_W1orAUX(1))
    {
        temp_data = out_status_new & ~(OUT_Y1_M | OUT_Y2_M);
        out_status_new = temp_data | OUT_Y1(0) | OUT_Y2(0);
    }
}

void init_out_sys(void)   //-最初所有的变量都需要给定明确初值,不能靠默认赋值
{
    OUT_HEAT_one_flag = 0;  //-0说明没有开启一级制热,等待一级制热开启判断
    OUT_COOL_one_flag = 0;  //-0说明没有开启一级制冷,等待一级制冷开启,并继续可能的延时判断
    //-out_flag = 0;        //-这里的函数会周期调用,那么这个不能清楚,否则口就不能刷新了
    out_hold_flag = 0;      //-这是保证一级二级同时收回的,这里初始化了,就该给0
    
    
}

/*
根据设定的系统模式值选择出口的判断分支
*/
void sys_mode_judge(void)
{
  //-UINT8 		temp_data;
  //-static UINT8 OUT_CN;
  run_heat_or_cool = 0;   //-空闲模式,不进行任何逻辑判断
  E_start_flag = 0;
  
  if(temperature_data_flag == 1)
    temperature_value = 0;
  else
    temperature_value = temperature_data_x10;

  programme_judge();  //-读取当前需要的比较温度

  //-在逻辑判断之前,判断下工作模式是否修改了,如果有修改,一些变量需要重新初始化
  if(RUN_status_flag == 0x55)
  {
     init_out_sys();
     RUN_status_flag = 0;
  }
  
	//-模式分为两部分,第一部分高四位表示,子部分第四位表示
	if((SYS_WORK_MODE & 0xf0) == 0x10)
	{

    if(SYS_WORK_MODE == 0x11)	//-普通加电热
		{//-二级制热
			H2_out_judge();
		}
    else if(SYS_WORK_MODE == 0x12)	//-普通加电热
		{//-二级制冷
			C2_out_judge();
		}
		else if(SYS_WORK_MODE == 0x13)	//-普通加电热
		{//-一级制热一级制冷
			C1H1_out_judge();
		}
		else if(SYS_WORK_MODE == 0x14)
		{//-一级制热二级制冷
			C2H1_out_judge();
		}
		else if(SYS_WORK_MODE == 0x15)
		{//-二级制热一级制冷
			C1H2_out_judge();
		}
		else if(SYS_WORK_MODE == 0x16)
		{//-二级制热二级制冷
			C2H2_out_judge();
		}


	}
	else if((SYS_WORK_MODE & 0xf0) == 0x20)
	{        
    if(SYS_WORK_MODE == 0x21)	//-普通加燃料
		{//-二级制热
			H2_out_judge(); //-如果处于加热中,风扇不允许关闭,
		}
    else if(SYS_WORK_MODE == 0x22)	//-普通加燃料
		{//-二级制冷
			C2_out_judge();
		}
		else if(SYS_WORK_MODE == 0x23)	//-普通加燃料
		{//-一级制热一级制冷
			C1H1_out_judge();
		}
		else if(SYS_WORK_MODE == 0x24)
		{//-一级制热二级制冷
			C2H1_out_judge();
		}
		else if(SYS_WORK_MODE == 0x25)
		{//-二级制热一级制冷
			C1H2_out_judge();
		}
		else if(SYS_WORK_MODE == 0x26)
		{//-二级制热二级制冷
			C2H2_out_judge();
		}
	}
	else if((SYS_WORK_MODE & 0xf0) == 0x30)
	{

		if(SYS_WORK_MODE == 0x31)	//-热泵加电源  == 单能源热泵
		{//-一级热泵
      one_P_out_judge();
		}
		else if(SYS_WORK_MODE == 0x32)
		{//-二级热泵
      two_P_out_judge();
		}
		else if(SYS_WORK_MODE == 0x33)
		{//-一级热泵+辅热
      one_PaAUX_out_judge();
		}
		else if(SYS_WORK_MODE == 0x34)
		{//-二级热泵+辅热
      two_PaAUX_out_judge();
		}
		/*else if(SYS_WORK_MODE == 0x35)
		{//-一级热泵+二级辅热

		}
		else if(SYS_WORK_MODE == 0x36)
		{//-二级热泵+二级辅热

		}*/
	}
	else if((SYS_WORK_MODE & 0xf0) == 0x40)
	{
		if(SYS_WORK_MODE == 0x41)	//-热泵加燃料  ==  双能源热泵
		{//-一级热泵
        one_P_out_judge();
		}
		else if(SYS_WORK_MODE == 0x42)
		{//-二级热泵
        two_P_out_judge();
		}
		else if(SYS_WORK_MODE == 0x43)
		{//-一级热泵+辅热
        one_PaAUX_out_judge();
        Yx_control_by_AUX();
		}
		else if(SYS_WORK_MODE == 0x44)
		{//-二级热泵+辅热
        two_PaAUX_out_judge();
        Yx_control_by_AUX();
		}
		/*else if(SYS_WORK_MODE == 0x45)
		{//-一级热泵+二级辅热

		}
		else if(SYS_WORK_MODE == 0x46)
		{//-二级热泵+二级辅热

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
  {//-说明紧急制热没有启动
    OUT_emergency_hot = 0;
    E_out_flag = 0;
  }
  //-if(OUT_emergency_hot == 1)
  //-{
  //-  out_flag = 0x55;
  //-  out_status_new = OUT_EorNOP(1);   //-这个被切换到了辅热OUT_W1orAUX
  //-}

  //-测试用
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