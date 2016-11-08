/*****************************************************************************/
/*       FileName  :   COMUSE.C                                              */
/*       Content   :   DSA-208 COMMON_USE Module                             */
/*       Date      :   Fri  02-22-2002                                       */
/*                     DSASoftWare(c)                                        */
/*                     CopyRight 2002             DSA-GROUP                  */
/*****************************************************************************/
#include "stm32f10x.h"
//-#include "demo.h"
#include "integer.h"
#include "user_conf.h"



//-#include "ff.h"
//-#include "diskio.h"
//-#include "misc.h"
//-#include "es705_escore.h"


/*---------------------------------------------------------------------------*/
/*                    Definition  of  global  variables                      */
/*---------------------------------------------------------------------------*/

//    none


/*---------------------------------------------------------------------------*/
/*                    Definition  of  local  variables                       */
/*---------------------------------------------------------------------------*/

//    none
typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;



/*---------------------------------------------------------------------------*/
/*                        IMPORT            functions                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                        LOCAL             functions                        */
/*---------------------------------------------------------------------------*/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/






/*---------------------------------------------------------------------------*/
/*                        PUBLIC            functions                        */
/*---------------------------------------------------------------------------*/
//-这个延时是mS级的
void Host_LowLevelDelay(UINT16 milliseconds)  //-最大延时65.535 S
{
    UINT16 temp_cn,temp_data;

    temp_cn = cticks_5ms;
    temp_data = 0;

    while(temp_data < milliseconds)
    {
    	 if(cticks_5ms >= temp_cn)
    	 	 temp_data = cticks_5ms - temp_cn;
    	 else
    	 	 temp_data = 0xffff - temp_cn + cticks_5ms;
    }
}

//-应该是uS级的延时
void TWI_Delay(void)
{
		UINT32 i=120;			//这里可以优化速度 ，经测试最低到5还能写入 		 150
											//-i=15时 延时2.411uS
											//-i=36时 延时5.035uS
		while(i--);
}

//-空指令
//-只有当这个指令是跳转后的第1条指令时才会需要2个系统周期，否则只需要1个周期
//-每个FCLK执行一条指令
//-STM32有三级流水线，指令周期不定的，arm给出的是1.25MIPS/Mhz，一个平均执行速度。
//-就是1Mhz的频率，每秒钟可以执行1.25M指令。72M，那么就是72*1.25
//-asm("NOP");		//-主频56M,那么每秒执行56*1.25M条指令,那么一条指令1/(56*1.25) uS
void NOP_Delay(UINT16 i)  //-一次周期是0.9825uS,但只是大概值,不能确定
{
		while(i--)
		{
				asm("NOP");
				asm("NOP");		//-采用流线方式,这里的延时是不可靠的不是简单的叠加
				asm("NOP");
				asm("NOP");
				asm("NOP");
				asm("NOP");
        asm("NOP");
				asm("NOP");

        asm("NOP");
				asm("NOP");
				asm("NOP");
				asm("NOP");
				asm("NOP");

		}
}


/***********************************************/
/* clock proc used in soft_1ms_int             */
/***********************************************/
/*===========================================================================*/
void Clock_Process(void)
{
/* for it is used in 1ms short ,it must disable interrupt in main_loop and other place */

    // other place needn't call the function,for REG_Surplus_Time changed only in 1ms_int.
    if(m_msec<1000) return;
    do
    {
        m_msec=m_msec-1000;
        m_sec  =m_sec+1;
    }
    while(m_msec>999);

    if(m_sec<60)
    {
        return;
    }
    else
    {
        /* not consider clock_process not run 1 times in 60s */
        m_sec=m_sec-60;
        m_min=m_min+1;
        //-测试增加
        //-led_display_start = 0x55;
        //-cticks_s_page = 0;
        //-led_display_new = 0xa5;	//-暂时启动动画
			  //-led_display_y = 0;
    }

    if(m_min<60)
    {
        return;
    }
    else
    {
        /* not consider clock_process not run 1 times in 60s */
        m_min=m_min-60;
        m_hour  =m_hour  +1;
    }

    if(m_hour<24)
    {
        return;
    }
    else
    {
        /* not consider clock_process not run 1 times in 60s */
        m_hour=m_hour-24;
        m_date=m_date+1;
    }


    if(m_date<29)
    {
        return;
    }
    else
    {
        if(m_date==29)
        {
            if(m_month==2)
            {
                if(((m_year) % 4)!=0)
                {
                    m_date=1;
                    m_month=3;
                }
            }
        }
        else
        {
            if(m_date==30)
            {
                if(m_month==2)
                {
                    m_date=1;
                    m_month=3;
                }
            }
            else
            {
                if(m_date==31)
                {
                    if((m_month==4)||(m_month==6)||(m_month==9)||(m_month==11))
                    {
                        m_date=1;
                        m_month=m_month+1;
                    }
                }
                else  // REG_Date==32
                {
                    m_date=1;
                    m_month=m_month+1;
                    if(m_month>12)
                    {
                        m_month=1;
                        m_year =m_year+1;
                    }
                }
            }
        }
    }
}


BYTE Judge_Time_In_MainLoop(WORD start_time,WORD judge_value)
{
    UL time_value1;
    UL time_value2;

    time_value2=(UL)start_time+judge_value;

    if(cticks_5ms<start_time)
    {
        time_value1=cticks_5ms+0x10000;
    }
    else
    {
        time_value1=cticks_5ms;
    }

    if(time_value1>time_value2)
        return YES;
    else
        return NO;
}

BYTE Judge_LongTime_In_MainLoop(WORD start_time,WORD judge_value)	//-长延时程序
{
    UL time_value1;
    UL time_value2;

    time_value2=(UL)start_time+judge_value;

    if(Time_2048ms_Counter<start_time)
    {
        time_value1=Time_2048ms_Counter+0x10000;	//-小于的实际上顶多转了一圈
    }
    else
    {
        time_value1=Time_2048ms_Counter;	//-就是以他为时间轴的
    }

    if(time_value1>time_value2)
        return YES;
    else
        return NO;
}

BYTE Judge_STime_In_MainLoop(DWORD start_time,WORD judge_value)
{
    UL time_value1;
    UL time_value2;

    time_value2=(UL)start_time+judge_value;

    time_value1=cticks_s;


    if(time_value1>time_value2)
        return YES;
    else
        return NO;
}













