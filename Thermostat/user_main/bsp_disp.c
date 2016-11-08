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
点阵屏策略:
每5mS刷一行内容,依次反复刷每一行
*/

/* Includes ------------------------------------------------------------------*/

#include "stm32f10x.h"
#include <stdio.h>
//-#include "bsp_led.h"
//-#include "bsp_timer.h"
//-#include "main.h"
#include "user_conf.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
//#ifdef OLDLED
//
//#define LD_OE(BitVal)  			GPIO_WriteBit(GPIOA,GPIO_Pin_0,BitVal)
////#define LD_ADDR_y(PortVal)  	GPIO_Write(GPIOC,PortVal)
////#define LD_LE(BitVal)  			GPIO_WriteBit(GPIOB,GPIO_Pin_0,BitVal)
//#define LD_SDI(BitVal)  		GPIO_WriteBit(GPIOA,GPIO_Pin_7,BitVal)
//#define LD_CLK(BitVal)  		GPIO_WriteBit(GPIOA,GPIO_Pin_5,BitVal)
//#define LD_LAT(BitVal)  		GPIO_WriteBit(GPIOC,GPIO_Pin_3,BitVal)
//
//#else

#define LD_OE(BitVal)  			GPIO_WriteBit(GPIOA,GPIO_Pin_7,BitVal)
#define LD_CLK(BitVal)  		GPIO_WriteBit(GPIOB,GPIO_Pin_0,BitVal)
#define LD_SDI(BitVal)  		GPIO_WriteBit(GPIOB,GPIO_Pin_1,BitVal)
#define LD_LAT(BitVal)  		GPIO_WriteBit(GPIOA,GPIO_Pin_6,BitVal)

//#endif



/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
u8 LineNumber = 0;   //在定时器中刷行

//-void TouchLedStatus(u8 Power,u8 Up,u8 Menu,u8 Down,u8 Wind);
void TouchLedStatus(u8 Wind,u8 Down,u8 Menu,u8 Up,u8 Power);

unsigned char number[][8] = {
  0x00,0x00,0x07,0x05,0x05,0x05,0x07,0x00,/*"0",0*/
  0x00,0x00,0x02,0x06,0x02,0x02,0x07,0x00,/*"1",0*/
  0x00,0x00,0x07,0x01,0x07,0x04,0x07,0x00,/*"2",0*/
  0x00,0x00,0x07,0x01,0x07,0x01,0x07,0x00,/*"3",0*/
  0x00,0x00,0x05,0x05,0x07,0x01,0x01,0x00,/*"4",0*/
  0x00,0x00,0x07,0x04,0x07,0x01,0x07,0x00,/*"5",0*/
  0x00,0x00,0x07,0x04,0x07,0x05,0x07,0x00,/*"6",0*/
  0x00,0x00,0x07,0x01,0x01,0x01,0x01,0x00,/*"7",0*/
  0x00,0x00,0x07,0x05,0x07,0x05,0x07,0x00,/*"8",0*/
  0x00,0x00,0x07,0x05,0x07,0x01,0x07,0x00,/*"9",0*/
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*" ",空*/
};


unsigned char character[][8] = {
  0x00,0x00,0x04,0x00,0x00,0x00,0x00,0x00,/*摄氏度*/
  0x07,0x04,0x07,0x01,0x07,0x00,0x00,0x00,/*5上标*/
  0x00,0x00,0x00,0x00,0x0F,0x00,0x00,0x00,/*负号*/
  0x00,0x00,0x0E,0x08,0x0E,0x08,0x0E,0x01,/*大写E*/
};

unsigned char WindSpeed[][8] = {
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*无风*/
  0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x00,/*低风*/
  0x00,0x00,0x00,0x10,0x30,0x30,0x30,0x00,/*中风*/
  0x00,0x30,0x30,0x30,0x30,0x30,0x30,0x00,/*高风*/
  0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*点*/
};


unsigned char NetWorkState[][8] = {
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*无网络标识*/
  0x01,0x03,0x07,0x00,0x00,0x00,0x00,0x00 /*有网络标识*/
};

unsigned char NetWorkSet1[][24] = {
  0x00,0x00,0x00,0x06,0x00,0xC0,0x0F,0x01,0xE0,0x03,0xFF,0x80,0x03,0xFF,0x80,0x0F,
  0x01,0xE0,0x06,0x00,0xC0,0x00,0x00,0x00,//0扳手
  0x00,0x00,0x00,0x24,0x93,0x20,0x66,0x95,0x70,0x66,0xD7,0x20,0x3C,0xB4,0x20,0x18,
  0x93,0x17,0x18,0x00,0x00,0x18,0x7F,0xFF,//1扳手+net
  0x00,0x00,0x00,0x24,0x30,0x88,0x66,0x78,0x88,0x66,0x49,0x98,0x3C,0x7A,0xA8,0x18,
  0x4B,0xB8,0x18,0x00,0x00,0x18,0x7F,0xF8,//2扳手+add
  0x00,0x00,0x00,0xF3,0x3C,0x00,0x64,0x98,0x07,0x64,0x98,0x0C,0x64,0x9B,0x18,0xF3,
  0x19,0xB0,0x00,0x00,0xE0,0xFF,0xFC,0x40,//3IOT+勾
  0x00,0x00,0x00,0x24,0x79,0x9E,0x66,0x32,0x4C,0x66,0x32,0x4C,0x3C,0x32,0x4C,0x18,
  0x79,0x8C,0x18,0x00,0x00,0x18,0x7F,0xFE,//4扳手+IOT
  0x00,0x00,0x00,0xF3,0x3C,0x00,0x64,0x98,0xC6,0x64,0x98,0x6C,0x64,0x98,0x38,0xF3,
  0x18,0x38,0x00,0x00,0x6C,0xFF,0xFC,0xC6,//5IOT+X
  0x00,0x00,0x00,0xF3,0x3C,0x06,0x64,0x98,0x0A,0x64,0x98,0x12,0x64,0x98,0x22,0xF3,
  0x18,0x62,0x00,0x00,0xE2,0xFF,0xFC,0xFE,//6IOT+三角形1
  0x00,0x00,0x00,0xF3,0x3C,0x06,0x64,0x98,0x0A,0x64,0x98,0x1A,0x64,0x98,0x3A,0xF3,
  0x18,0x5A,0x00,0x00,0xDA,0xFF,0xFC,0xFE,//7IOT+三角形2
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,//8

};


unsigned char FailNetStata[][8] = {
  0x00,0x00,0x00,0x00,0x05,0x02,0x05,0x00,
};

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/




/**
  * @fun    void Send_Bit
  * @brief
  *         2015/12/22 星期二,Administrator
  * @param  None
  *
  * @retval
  */
void Send_Bit(u8 data)
{
  LD_SDI((BitAction)(data&0x01));
  //-翻转时钟信号上升沿时采用
  LD_CLK((BitAction)0);
  //-TWI_Delay();
  LD_CLK((BitAction)1);
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
void Send_Byte(u32 data, u8 length)
{
  u8 count;
  for(count=0;count<length;count++)
  {
    Send_Bit(data>>count);
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
void Display(u8 x, u32 y)
{
  u16  hang_num = 0;
  hang_num = hang_num | (1 << (7 - x));
  Send_Byte(hang_num,8);
  Send_Byte(y,24);	//-总共是24列,一个数据的显示需要移动24次,来并转串
  LD_LAT((BitAction)1);
  //-TWI_Delay();
  LD_LAT((BitAction)0);
}

///**
//  * @fun    void RefreshLed
//  * @brief  刷新屏显温度及风速
//  * @param  u8 T 温度
//  * @param  u8 N 网络标识
//  * @param  u8 W 风速
//  *
//  * @retval
//  */
//void RefreshLed(u8 T,u8 W,u8 N)
//{
//  static unsigned char i;
//  for(i=0;i<8;i++)
//  {
////    Display(i,NetWorkState[N][i] | WindSpeed[W][i] | character[0][i]<<8 | (number[T%10][i] << 12) | (number[T/10][i] << 16));
//    Display(i,NetWorkState[N][i] | WindSpeed[W][i] | (number[T%10][i] << 12) | (number[T/10][i] << 16));
//  }
//}


/**
  * @fun    void RefreshLed
  * @brief  刷新屏显温度及风速
  * @param  u8 T 温度
  * @param  u8 N 网络标识
  * @param  u8 W 风速
  *
  * @retval
  */
void led_display_hang_it(void)
{
  u32 Line_data;
  static u32 xiu_dis_cn;
  //-u8 N=0;
  //-u8 W=0;
  u16 T=22,temp_data;

  if(STOP_status2 == 0)
  {
    LD_OE((BitAction)1);
    return;
  }
  
  if(menu_set_tt_flag == 1)
  {//-如果定值处于修改中,让屏幕有闪耀的效果.
    xiu_dis_cn++;
    if((LineNumber == 0) && (xiu_dis_cn > 300))
    {
      if(xiu_dis_cn > 350)
        xiu_dis_cn = 0;
      LD_OE((BitAction)1);
      return;
    }
  }
  
  LD_OE((BitAction)0);
  Line_data = 0;
//-E_out_flag = 0x55;
//-灯的行编码是23~0,左移的数字就是显示的起点,比如左移21,就是21点位开始显示
//-4个像素点一个字符包括空格都有了
  if(E_out_flag == 0x55)  //-正处于紧急制热开动模式
  {
    Line_data |= (character[3][LineNumber] << 20);      //-增加标志E
  }
  
    if(menu_set_tt_CorF == 1)
    {//-华氏
      if(menu_set_tt_flag == 0) //-判断数据有没有修改
      {
        if(temperature_data_flag == 1)
          T = 0 - temperature_data_x10;
        else
          T = temperature_data_x10;
        //-华氏温度F  华氏度＝32＋摄氏度×1.8
        temp_data = (u16)(T * 1.8);
        if((temp_data % 10) > 4)
          temp_data = temp_data / 10 + 1;
        else
          temp_data = temp_data / 10;
        T = (u16)(32 + temp_data);
        if(T > 3000)
          T = 0;
      }
      else
      {
        if(RUN_status == 1)
        {
          menu_set_tt_volue = menu_set_tt_min_f;
        }
        else if(RUN_status == 2)
        {
          menu_set_tt_volue = menu_set_tt_max_f;
        }
        
        T = menu_set_tt_volue;
      }
      //-Line_data |= WindSpeed[W][LineNumber];      //-增加风速
      Line_data |= (character[0][LineNumber]<<8);      //-增加字符
      Line_data |= (number[T%10][LineNumber] << 12);      //-增加温度低位
      if(T/10)
        Line_data |= (number[T/10][LineNumber] << 16);      //-增加温度高位
    }
    else
    {//-摄氏
      if(menu_set_tt_flag == 0)
      {
        T = temperature_data_x10/10;
        if(temperature_data_x10%10 > 4)
          Line_data |= (character[1][LineNumber]<<8);      //-增加字符
      }
      else
      {
        if(RUN_status == 1)
        {
          menu_set_tt_volue = menu_set_tt_min_eep;  //-这里可以处理后接收设置值不规整的情况
        }
        else if(RUN_status == 2)
        {
          menu_set_tt_volue = menu_set_tt_max_eep;
        }
        
        T = menu_set_tt_volue / 100;
        if((menu_set_tt_volue % 100) == 50)
          Line_data |= (character[1][LineNumber]<<8);      //-增加字符
      }
      
      //-Line_data |= WindSpeed[W][LineNumber];      //-增加风速
      //-Line_data |= (character[1][LineNumber]<<8);      //-增加字符
      Line_data |= (number[T%10][LineNumber] << 12);      //-增加温度低位
      if(T/10)
        Line_data |= (number[T/10][LineNumber] << 16);      //-增加温度高位
      if(temperature_data_flag == 1)
        Line_data |= (character[2][LineNumber] << 21);    //-增加负号
    }
    
  
  
  if(zigbee_flag == 1)
  {//-开始加网开始闪耀灯
    Line_data |= WindSpeed[0][LineNumber];      //-增加风速
  }
  else if(zigbee_flag == 2)
  {//-开始加网开始闪耀灯
    Line_data |= WindSpeed[4][LineNumber];      //-增加风速
  }
  else if(zigbee_flag == 0x55)
  {//-成功入网灯常亮
    Line_data |= WindSpeed[4][LineNumber];      //-增加风速
  }
  
  //-测试
  if(SYS_power_FLAG == 0x55)
  {
    Line_data |= WindSpeed[2][LineNumber]; 
    GPIO_WriteBit(GPIOB,GPIO_Pin_3,(BitAction)0);
  }
  else
    GPIO_WriteBit(GPIOB,GPIO_Pin_3,(BitAction)1);
  
  
  Display(LineNumber,Line_data);
  LineNumber++;
  if(LineNumber >= 8)
    LineNumber = 0;
}




/**
  * @fun    void TurnOffModeLED
  * @brief  关闭模式led
  * @author huangzibo
  * @param  None
  *
  * @retval
  */
void TurnOffDisLED(void)
{
  LD_OE((BitAction)1);
}


/**
  * @fun    void TouchLedStatus(uint8 Power,uint8 Up,uint8 Menu,uint8 Down,uint8 Wind)
  * @brief  触摸按键指示灯
  * @author huangzibo
  * @param  power
  * @param  up
  * @param  menu
  * @param  down
  * @param  wind
  *
  * @retval
  */
void TouchLedStatus(u8 Wind,u8 Down,u8 Menu,u8 Up,u8 Power)
{
  if(Power == 1){PowerLED((BitAction)1);}
  else if(Power == 0x00){PowerLED((BitAction)0);}

  if(Up == 1){UpLED((BitAction)1);}
  else if(Up == 0x00){UpLED((BitAction)0);}

  if(Menu == 1){MenuLED((BitAction)1);}
  else if(Menu == 0x00){MenuLED((BitAction)0);}

  if(Down == 1){DownLED((BitAction)1);}
  else if(Down == 0x00){DownLED((BitAction)0);}

  if(Wind == 1){WindLED((BitAction)1);}
  else if(Wind == 0x00){WindLED((BitAction)0);}
}



/******************* (C) COPYRIGHT 2015 WuLianGroup ********END OF FIL*********/