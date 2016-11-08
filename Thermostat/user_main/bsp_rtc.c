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
����������:
ÿ5mSˢһ������,���η���ˢÿһ��
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
//-2000��Ϊ��׼ʱ��,����ʱ�������������calendar_to_sec(2000,1,1,0,0,1);��С��ֵ
u32 calendar_to_sec(u16 Tmp_Year,u8 Tmp_Month,u8 Tmp_Date,u8 Tmp_HH,u8 Tmp_MM,u8 Tmp_SS)
{
  u32 LeapY, ComY, TotSeconds, TotDays;

    if(Tmp_Year==2000)
          LeapY = 0;
    else
          LeapY = (Tmp_Year - 2000 -1)/4 +1;  //-����õ���2000�꿪ʼ��������Ŀ

    ComY = (Tmp_Year - 2000)-(LeapY);   //-��ͨ�����Ŀ

    if (Tmp_Year%4)
        //common year,,��ͨ��365��
        TotDays = LeapY*366 + ComY*365 + Month_Days_Accu_C[Tmp_Month-1] + (Tmp_Date-1);
    else
        //leap year,,����Ϊ366��
        TotDays = LeapY*366 + ComY*365 + Month_Days_Accu_L[Tmp_Month-1] + (Tmp_Date-1);

    TotSeconds = TotDays*SecsPerDay + (Tmp_HH*3600 + Tmp_MM*60 + Tmp_SS);
    return TotSeconds;
}

#define SecsPerComYear 3153600//(365*3600*24)
#define SecsPerLeapYear 31622400//(366*3600*24)
#define SecsPerFourYear 126230400//((365*3600*24)*3+(366*3600*24))
#define SecsPerDay      (3600*24)

//-�����е������ۼ�
s32 Year_Secs_Accu[5]={0,
                      31622400,
                      63158400,
                      94694400,
                      126230400};
//-��ͨ�����µ������ۼ�
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
//-�������µ������ۼ�
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

u8 const table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5}; //���������ݱ�

//������������ڼ�
//��������:���빫�����ڵõ�����(ֻ����1901-2099��)
//�������������������
//����ֵ�����ں�
u8 RTC_Get_Week(u16 year,u8 month,u8 day)
{
       u16 temp2;
       u8 yearH,yearL;

       yearH=year/100;
       yearL=year%100;
       // ���Ϊ21����,�������100
       if (yearH>19)
       	yearL+=100;
       // ����������ֻ��1900��֮���
       temp2=yearL+yearL/4;
       temp2=temp2%7;
       temp2=temp2+day+table_week[month-1];
       if (yearL%4==0&&month<3)temp2--;
       return(temp2%7);
}

//-�����������,��Ҫת��Ϊ����ʱ��2000Ϊ��׼
void sec_to_calendar(u32 TimeVar)
{
  u16 TY;
  u8 TM,TD,THH,TMM,TSS;
  u16 Num4Y,i,Off4Y = 0,NumY,NumDay;
  u32 OffSec;


    Num4Y = TimeVar/SecsPerFourYear;  //-�������������
    OffSec = TimeVar%SecsPerFourYear; //-���µ�����

    i=1;
    while(OffSec > Year_Secs_Accu[i++])
      Off4Y++;    //-�õ����µ����������

    /* Numer of Complete Year */
    NumY = Num4Y*4 + Off4Y;
      /* 2000,2001,...~2000+NumY-1 complete year before, so this year is 2000+NumY*/
    TY = 2000+NumY;   //-���յ����

    OffSec = OffSec - Year_Secs_Accu[i-2];  //-��ȥ�����������,ʣ�������

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
    //-���������Ѿ�ת����������ʱ��,������Դ���Щ�ֲ����������
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
  //��������Exit_line17Ϊ������
  EXTI_InitTypeDef EXTI_InitStructure;
  //-NVIC_InitTypeDef NVIC_InitStructure;

  /* Configure EXTI Line17(RTC Alarm) to generate an interrupt on rising edge */
  EXTI_ClearITPendingBit(EXTI_Line17);
  EXTI_InitStructure.EXTI_Line = EXTI_Line17;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_Init(&EXTI_InitStructure);

   //Ȼ������Alarm

}

void RTC_init(void)
{
    //-��ָ���ĺ󱸼Ĵ����ж�ȡ���ݣ���������ѡ��󱸼Ĵ�����������BKP_DR1~BKP_DR10 10���󱸼Ĵ���
   if (BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)
   {//-�жϺ��������Ƿ�ʧ�������ʧ˵��ϵͳ��RTC�Ѿ����磬ʱ����Ϣ��ʧ����Ҫ��������
      /* RTC Configuration */
      RTC_Configuration();

      /* Adjust time by values entered by the user on the hyperterminal */
      Time_Adjust();

      BKP_WriteBackupRegister(BKP_DR1, 0xA5A5); //-��ָ���ĺ󱸼Ĵ�����д���û���������  ��������BKP_DR1��д��0xA5A5
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
     //-����ͨ������־λ,����֪����λ�����
     /* Allow access to BKP Domain */
  PWR_BackupAccessCmd(ENABLE);  //-ʹ�ܺ󱸼Ĵ�������
  /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();

      /* Wait for RTC registers synchronization */
      RTC_WaitForSynchro();
      /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();

      RTC_ITConfig(RTC_IT_ALR, ENABLE);   //-ʹ�������ж�
      //-RTC_ITConfig(RTC_IT_SEC, ENABLE);  //-ʹ�ܻ�ʧ��ָ����RTC�ж�

      /* Wait until last write operation on RTC registers has finished */
      RTC_WaitForLastTask();

      //-RCC_ClearFlag();
   }

}



///////////////////////////////////////////////////////////////////////////////
//-��ֵ���洦
//-���ݼĴ�����42��16λ�ļĴ������������洢84���ֽڵ��û�Ӧ�ó������ݡ�
//-20�ֽ����ݺ󱸼Ĵ���(��������С������Ʒ)����84�ֽ����ݺ󱸼Ĵ���(�������ͻ����Ͳ�Ʒ)
//-Ŀǰϵͳ(������)ֻ��20���ֽڱ��ݼĴ���,����10����Ϊ��λʹ��,���������˵
//-BKP_DR1    RTC�Ƿ���Ч
//-BKP_DR2    �˵����õ������¶�ֵ
//-BKP_DR3    �˵����õ������¶�ֵ 
//-BKP_DR4    ������õĹ���ģʽ,���е��籣�ֹ���
//-BKP_DR5    �¿�������״̬(�����)
//-BKP_DR6    �¶���ʾ��ʽ ���������� �������� ��� ��￪�� 0 0 0
//-BKP_DR7    �¿�������
//-BKP_DR8    �Զ�ģʽ���õ������¶�ֵ
//-BKP_DR9    �Զ�ģʽ���õ������¶�ֵ
//-BKP_DR10   swing���� diff���� third����  (ÿ��ռ��λ)



//-�Ѷ�ֵд�뵽���ݼĴ���
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