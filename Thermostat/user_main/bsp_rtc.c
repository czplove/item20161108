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

u32 Month_Days_Accu_C[13] = {0,31,59,90,120,151,181,212,243,273,304,334,365};
u32 Month_Days_Accu_L[13] = {0,31,60,91,121,152,182,213,244,274,305,335,366};
#define SecsPerDay (3600*24)

//-change Year-Month-Data-Hour-Minute-Seconds into X(Second) to set RTC->CNTR
//-2000年为基准时间,计算时间必须大于这个数calendar_to_sec(2000,1,1,0,0,1);最小赋值
u32 calendar_to_sec(u16 Tmp_Year,u8 Tmp_Month,u8 Tmp_Date,u8 Tmp_HH,u8 Tmp_MM,u8 Tmp_SS)
{
  u32 LeapY, ComY, TotSeconds, TotDays;

    if(Tmp_Year==2000)
          LeapY = 0;
    else
          LeapY = (Tmp_Year - 2000 -1)/4 +1;  //-计算得到从2000年开始的闰年数目

    ComY = (Tmp_Year - 2000)-(LeapY);   //-普通年的数目

    if (Tmp_Year%4)
        //common year,,普通年365日
        TotDays = LeapY*366 + ComY*365 + Month_Days_Accu_C[Tmp_Month-1] + (Tmp_Date-1);
    else
        //leap year,,闰年为366日
        TotDays = LeapY*366 + ComY*365 + Month_Days_Accu_L[Tmp_Month-1] + (Tmp_Date-1);

    TotSeconds = TotDays*SecsPerDay + (Tmp_HH*3600 + Tmp_MM*60 + Tmp_SS);
    return TotSeconds;
}

#define SecsPerComYear 3153600//(365*3600*24)
#define SecsPerLeapYear 31622400//(366*3600*24)
#define SecsPerFourYear 126230400//((365*3600*24)*3+(366*3600*24))
#define SecsPerDay      (3600*24)

//-四年中的秒数累加
s32 Year_Secs_Accu[5]={0,
                      31622400,
                      63158400,
                      94694400,
                      126230400};
//-普通年中月的秒数累加
s32 Month_Secs_Accu_C[13] = { 0,
                            2678400,
                            5097600,
                            7776000,
                            10368000,
                            13046400,
                            15638400,
                            18316800,
                            20995200,
                            23587200,
                            26265600,
                            28857600,
                            31536000};
//-闰年中月的秒数累加
s32 Month_Secs_Accu_L[13] = {0,
                            2678400,
                            5184000,
                            7862400,
                            10454400,
                            13132800,
                            15724800,
                            18403200,
                            21081600,
                            23673600,
                            26352000,
                            28944000,
                            31622400};

u8 const table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5}; //月修正数据表

//获得现在是星期几
//功能描述:输入公历日期得到星期(只允许1901-2099年)
//输入参数：公历年月日
//返回值：星期号
u8 RTC_Get_Week(u16 year,u8 month,u8 day)
{
       u16 temp2;
       u8 yearH,yearL;

       yearH=year/100;
       yearL=year%100;
       // 如果为21世纪,年份数加100
       if (yearH>19)
       	yearL+=100;
       // 所过闰年数只算1900年之后的
       temp2=yearL+yearL/4;
       temp2=temp2%7;
       temp2=temp2+day+table_week[month-1];
       if (yearL%4==0&&month<3)temp2--;
       return(temp2%7);
}

//-输入的是秒数,需要转化为日历时间2000为基准
void sec_to_calendar(u32 TimeVar)
{
  u16 TY;
  u8 TM,TD,THH,TMM,TSS;
  u16 Num4Y,i,Off4Y = 0,NumY,NumDay;
  u32 OffSec;


    Num4Y = TimeVar/SecsPerFourYear;  //-整数的四年个数
    OffSec = TimeVar%SecsPerFourYear; //-余下的秒数

    i=1;
    while(OffSec > Year_Secs_Accu[i++])
      Off4Y++;    //-得到余下的中年的整数

    /* Numer of Complete Year */
    NumY = Num4Y*4 + Off4Y;
      /* 2000,2001,...~2000+NumY-1 complete year before, so this year is 2000+NumY*/
    TY = 2000+NumY;   //-最终的年号

    OffSec = OffSec - Year_Secs_Accu[i-2];  //-减去整年的秒数后,剩余的秒数

    /* Month (TBD with OffSec)*/
    i=0;
    if(TY%4)
    {// common year
      while(OffSec > Month_Secs_Accu_C[i++]);
      TM = i-1;
      OffSec = OffSec - Month_Secs_Accu_C[i-2];
    }
    else
    {// leap year
      while(OffSec > Month_Secs_Accu_L[i++]);
      TM = i-1;
      OffSec = OffSec - Month_Secs_Accu_L[i-2];
    }

    /* Date (TBD with OffSec) */
    NumDay = OffSec/SecsPerDay;
    OffSec = OffSec%SecsPerDay;
    TD = NumDay+1;

    /* Compute hours */
    THH = OffSec/3600;
    /* Compute minutes */
    TMM = (OffSec % 3600)/60;
    /* Compute seconds */
    TSS = (OffSec % 3600)% 60;
    //-至此上面已经转化好了日历时间,下面可以从这些局部变量中输出
    m_year  = TY;
    m_month =TM;
    m_date  =TD;
    m_hour  =THH;
    m_min   =TMM;
    m_sec   =TSS;
    m_week  =RTC_Get_Week(TY,TM,TD);
    //-m_week  =RTC_Get_Week(2003,5,3);
}

/**
  * @brief  Adjusts time.
  * @param  None
  * @retval None
  */
void Time_Adjust(void)
{
  //-u8 temp_year=16;
  //-u8 temp_month=3;
  //-u8 temp_date=28;
  //-u8 temp_hour=20;
  //-u8 temp_min=16;
  //-u8 temp_sec=0;

  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();
  /* Change the current time */
  RTC_SetCounter(calendar_to_sec(REG_year,REG_month,REG_date,REG_hour,REG_min,REG_sec));
  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();
}

void AutoWakeupConfigure(void)
{
  //首先配置Exit_line17为上升沿
  EXTI_InitTypeDef EXTI_InitStructure;
  //-NVIC_InitTypeDef NVIC_InitStructure;

  /* Configure EXTI Line17(RTC Alarm) to generate an interrupt on rising edge */
  EXTI_ClearITPendingBit(EXTI_Line17);
  EXTI_InitStructure.EXTI_Line = EXTI_Line17;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_Init(&EXTI_InitStructure);

   //然后配置Alarm

}

void RTC_init(void)
{
    //-从指定的后备寄存器中读取数据，参数用来选择后备寄存器，可以是BKP_DR1~BKP_DR10 10个后备寄存器
   if (BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)
   {//-判断后备区数据是否丢失，如果丢失说明系统的RTC已经掉电，时间信息丢失，需要重新配置
      /* RTC Configuration */
      RTC_Configuration();

      /* Adjust time by values entered by the user on the hyperterminal */
      Time_Adjust();

      BKP_WriteBackupRegister(BKP_DR1, 0xA5A5); //-向指定的后备寄存器中写入用户程序数据  这里是向BKP_DR1中写入0xA5A5
      BKP_WriteBackupRegister(BKP_DR2, 0x03E8);
      BKP_WriteBackupRegister(BKP_DR3, 0x09C4);
      BKP_WriteBackupRegister(BKP_DR4, 0x0001);
      BKP_WriteBackupRegister(BKP_DR5, 0x0000);
      BKP_WriteBackupRegister(BKP_DR6, 0x0040);
      BKP_WriteBackupRegister(BKP_DR7, 0x0000);
      BKP_WriteBackupRegister(BKP_DR8, 0x03E8);
      BKP_WriteBackupRegister(BKP_DR9, 0x09C4);
      BKP_WriteBackupRegister(BKP_DR10, 0x0135);
   }
   else
   {
     //-这里通过检查标志位,可以知道复位的情况
     /* Allow access to BKP Domain */
  PWR_BackupAccessCmd(ENABLE);  //-使能后备寄存器访问
  /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();

      /* Wait for RTC registers synchronization */
      RTC_WaitForSynchro();
      /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();

      RTC_ITConfig(RTC_IT_ALR, ENABLE);   //-使能闹钟中断
      //-RTC_ITConfig(RTC_IT_SEC, ENABLE);  //-使能或失能指定的RTC中断

      /* Wait until last write operation on RTC registers has finished */
      RTC_WaitForLastTask();

      //-RCC_ClearFlag();
   }

}



///////////////////////////////////////////////////////////////////////////////
//-定值保存处
//-备份寄存器是42个16位的寄存器，可用来存储84个字节的用户应用程序数据。
//-20字节数据后备寄存器(中容量和小容量产品)，或84字节数据后备寄存器(大容量和互联型产品)
//-目前系统(中容量)只有20个字节备份寄存器,先以10个字为单位使用,如果不够再说
//-BKP_DR1    RTC是否有效
//-BKP_DR2    菜单设置的制热温度值
//-BKP_DR3    菜单设置的制冷温度值 
//-BKP_DR4    最后设置的工作模式,具有掉电保持功能
//-BKP_DR5    温控器运行状态(开或关)
//-BKP_DR6    温度显示格式 蜂鸣器开关 紧急制热 风机 马达开关 0 0 0
//-BKP_DR7    温控器类型
//-BKP_DR8    自动模式设置的制热温度值
//-BKP_DR9    自动模式设置的制冷温度值
//-BKP_DR10   swing设置 diff设置 third设置  (每个占四位)



//-把定值写入到备份寄存器
u8 write_bkp_eep(uint16_t BKP_DR, uint16_t Data)
{
    BKP_WriteBackupRegister(BKP_DR, Data);

    if(BKP_ReadBackupRegister(BKP_DR) != Data)
      return 0;
    else
      return 1;
}

uint16_t read_bkp_eep(uint16_t BKP_DR)
{
    return BKP_ReadBackupRegister(BKP_DR);
}

























/******************* (C) COPYRIGHT 2015 WuLianGroup ********END OF FIL*********/