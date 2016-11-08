//-简单的实现FLASH的读写,不搞复杂的逻辑,这个尽量少用
#include "user_conf.h"
//-#include "eeprom.h"


/*
以32位为最小单位进行读写操作,实际中需要几个字节,就取几个字节.

?读写的时候是否可以继续进行其它操作
?是否可以被中断打断
?擦除一页的时间是多长

如果有一个定值需要使用FLASH 进行保存,那么可以设置一个标志位判断是否需要修改定值
否则上电直接读FLASH中的值就行

大容量产品:
FLASH 0x08000000;到0x0807FFFF;
对于大容量产品，其被划分为 256 页，每页 2K 字节。注意，小容量和中容量产品则每页只有 1K 字节。
中容量产品:	64K flash
FLASH 0x08000000;到0x0800FFFF;
注意，小容量和中容量产品则每页只有 1K 字节。

//-大容量256K flash
//-模拟FLASH定为16K,0x4000.每页2K,0x800.
//-0x0807C000;到0x0807FFFF;为设定的模拟FLASH空间.
中容量64K flash
模拟FLASH定为24K,0x6000.每页1K,0x400.
*/


/*
0x08000000 ~ 0x0800FFFF
注意，小容量和中容量产品则每页只有 1K 字节。

在执行闪存写操作时，任何对闪存的读操作都会锁住总线，在
写操作完成后读操作才能正确地进行；既在进行写或擦除操作
时，不能进行代码或数据的读取操作。所以在每次操作之前，
我们都要等待上一次操作完成这次操作才能开始。
*/
#define FLASH_PAGE_SIZE    ((uint16_t)0x400)

#define BANK24_WRITE_START_ADDR  ((UINT32)0x0800A000)
#define BANK24_WRITE_END_ADDR    ((UINT32)0x0800A3FF)
#define BANK23_WRITE_START_ADDR  ((UINT32)0x0800A400)
#define BANK23_WRITE_END_ADDR    ((UINT32)0x0800A7FF)
#define BANK22_WRITE_START_ADDR  ((UINT32)0x0800A800)
#define BANK22_WRITE_END_ADDR    ((UINT32)0x0800ABFF)
#define BANK21_WRITE_START_ADDR  ((UINT32)0x0800AC00)
#define BANK21_WRITE_END_ADDR    ((UINT32)0x0800AFFF)

#define BANK20_WRITE_START_ADDR  ((UINT32)0x0800B000)
#define BANK20_WRITE_END_ADDR    ((UINT32)0x0800B3FF)
#define BANK19_WRITE_START_ADDR  ((UINT32)0x0800B400)
#define BANK19_WRITE_END_ADDR    ((UINT32)0x0800B7FF)
#define BANK18_WRITE_START_ADDR  ((UINT32)0x0800B800)
#define BANK18_WRITE_END_ADDR    ((UINT32)0x0800BBFF)
#define BANK17_WRITE_START_ADDR  ((UINT32)0x0800BC00)
#define BANK17_WRITE_END_ADDR    ((UINT32)0x0800BFFF)

#define BANK16_WRITE_START_ADDR  ((UINT32)0x0800C000)
#define BANK16_WRITE_END_ADDR    ((UINT32)0x0800C3FF)
#define BANK15_WRITE_START_ADDR  ((UINT32)0x0800C400)
#define BANK15_WRITE_END_ADDR    ((UINT32)0x0800C7FF)
#define BANK14_WRITE_START_ADDR  ((UINT32)0x0800C800)
#define BANK14_WRITE_END_ADDR    ((UINT32)0x0800CBFF)
#define BANK13_WRITE_START_ADDR  ((UINT32)0x0800CC00)
#define BANK13_WRITE_END_ADDR    ((UINT32)0x0800CFFF)

#define BANK12_WRITE_START_ADDR  ((UINT32)0x0800D000)
#define BANK12_WRITE_END_ADDR    ((UINT32)0x0800D3FF)
#define BANK11_WRITE_START_ADDR  ((UINT32)0x0800D400)
#define BANK11_WRITE_END_ADDR    ((UINT32)0x0800D7FF)
#define BANK10_WRITE_START_ADDR  ((UINT32)0x0800D800)
#define BANK10_WRITE_END_ADDR    ((UINT32)0x0800DBFF)
#define BANK9_WRITE_START_ADDR  ((UINT32)0x0800DC00)
#define BANK9_WRITE_END_ADDR    ((UINT32)0x0800DFFF)

#define BANK8_WRITE_START_ADDR  ((UINT32)0x0800E000)
#define BANK8_WRITE_END_ADDR    ((UINT32)0x0800E3FF)
#define BANK7_WRITE_START_ADDR  ((UINT32)0x0800E400)
#define BANK7_WRITE_END_ADDR    ((UINT32)0x0800E7FF)
#define BANK6_WRITE_START_ADDR  ((UINT32)0x0800E800)
#define BANK6_WRITE_END_ADDR    ((UINT32)0x0800EBFF)
#define BANK5_WRITE_START_ADDR  ((UINT32)0x0800EC00)
#define BANK5_WRITE_END_ADDR    ((UINT32)0x0800EFFF)

#define BANK4_WRITE_START_ADDR  ((UINT32)0x0800F000)
#define BANK4_WRITE_END_ADDR    ((UINT32)0x0800F3FF)
#define BANK3_WRITE_START_ADDR  ((UINT32)0x0800F400)
#define BANK3_WRITE_END_ADDR    ((UINT32)0x0800F7FF)
#define BANK2_WRITE_START_ADDR  ((UINT32)0x0800F800)
#define BANK2_WRITE_END_ADDR    ((UINT32)0x0800FBFF)
#define BANK1_WRITE_START_ADDR  ((UINT32)0x0800FC00)
#define BANK1_WRITE_END_ADDR    ((UINT32)0x0800FFFF)



#define led_display_long_EEP		(BANK1_WRITE_START_ADDR + 0x000)		//-0x004
//-#define led_display_long_EEP		(BANK1_WRITE_START_ADDR + 0x004)

#define voice_flag_EEP		(BANK2_WRITE_START_ADDR + 0x000)		//-0x004

FLASH_Status FLASHStatus;// = FLASH_COMPLETE;

UINT32 Data = 0x3210ABCD;

void MONI_EEPROM_init(void)
{
	 //uint16_t result = 0;


	/* Unlock the Flash Program Erase controller */
	FLASH_Unlock();		//-第一步

	/* Clear All pending flags */
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);		//-这个是后来加的,必要性需要考虑

  /* Erase the FLASH pages */
  //-for(EraseCounter = 0; (EraseCounter < NbrOfPage) && (FLASHStatus == FLASH_COMPLETE); EraseCounter++)
  //{
    FLASHStatus = FLASH_ErasePage(BANK1_WRITE_START_ADDR);//- + (FLASH_PAGE_SIZE * EraseCounter));		//-以页为单位擦除
  //}

  //-while((Address < BANK1_WRITE_END_ADDR) && (FLASHStatus == FLASH_COMPLETE))
  //{
    FLASHStatus = FLASH_ProgramWord(BANK1_WRITE_START_ADDR, Data);		//-32 位字写入函数，其他分别为 16 位半字写入和用户选择字节写入函数
    //-Address = Address + 4;
  //}

  //-EEP_Data = (*(UINT32*) BANK1_WRITE_START_ADDR);

		/* EEPROM Init */
		//-EE_Init();

		/* Write to EEPROM */
		//-result = memcpy_to_eeprom_with_checksum(TITLE_KEY, title, TITLE_SIZE);
		//-result = memcpy_to_eeprom_with_checksum(POINT_KEY, &point, sizeof(point));

		/* Read from EEPROM */
		//-result = memcpy_from_eeprom_with_checksum(title, TITLE_KEY, TITLE_SIZE);
		//-result = memcpy_from_eeprom_with_checksum(&point, POINT_KEY, sizeof(point));

		FLASH_Lock();		//-结束修改
}

int MONI_EEPROM_read(void)		//-从模拟FLASH中读出定值,供程序使用.
{
	 UINT32 temp_data;
	 UINT8	i,j;
	 //-int res;

	 for(i=0;i<21;i++)
	 {
		 	for(j=0;j<128;j++)
		  {
		 			temp_data = (*(UINT32*) (BANK1_WRITE_START_ADDR - i*0x400 + j*4));
		 			if((temp_data & 0xffff0000) < 0x05A00000)
		 			{//-数据点有效
		 					programme_day[i].day_dot[j].MIN = temp_data >> 16;
              programme_day[i].day_dot[j].MIN_VOULE = (temp_data & 0x0000ff00) >> 8;
              programme_day[i].day_dot[j].MAX_VOULE = (temp_data & 0x000000ff);
		 			}
		 			else
		 				break;
		  }
		  programme_day[i].SET_status = 0;
	 }

	 return 0;
}

UINT8 STM32_FLASH_write_word(uint32_t Address, uint32_t Data)
{
	UINT32 temp_data;
	UINT8	ret;

	ret = 0;
///////////////////////////////////////////////////////////////////////////////
	/* Unlock the Flash Program Erase controller */
	FLASH_Unlock();		//-第一步

	/* Clear All pending flags */
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);		//-这个是后来加的,必要性需要考虑

  /* Erase the FLASH pages */
  //-for(EraseCounter = 0; (EraseCounter < NbrOfPage) && (FLASHStatus == FLASH_COMPLETE); EraseCounter++)
  //{
  //-擦除是以页为单位的,这里仅仅首地址的时候才擦除,其它情况下
  if(((Address & 0x3ff) == 0) && (Address >= BANK24_WRITE_START_ADDR))	//-每页的起始地址进行擦除
    FLASHStatus = FLASH_ErasePage(Address);//- + (FLASH_PAGE_SIZE * EraseCounter));		//-以页为单位擦除
  //}

  //-while((Address < BANK1_WRITE_END_ADDR) && (FLASHStatus == FLASH_COMPLETE))
  //{
    FLASHStatus = FLASH_ProgramWord(Address, Data);		//-32 位字写入函数，其他分别为 16 位半字写入和用户选择字节写入函数
    //-Address = Address + 4;
  //}

  temp_data = (*(UINT32*) Address);

  if(temp_data != Data)		//-修改不正确了,会继续修改直到正确为止
  	ret = 1;

		FLASH_Lock();		//-结束修改
///////////////////////////////////////////////////////////////////////////////
	return ret;
}

void MONI_EEPROM_write(void)
{
	 UINT32 temp_data;
	 UINT8	i,j,k;


	for(i=0;i<21;i++)
	{
			if(programme_day[i].SET_status == 0x55)
			{
				for(j=0;j<128;j++)
				{
					if((programme_day[i].day_dot[j].MIN <= 1440) && (programme_day[i].day_dot[j].MIN > 0))
					{

						temp_data = programme_day[i].day_dot[j].MIN << 16;
						temp_data = temp_data + (programme_day[i].day_dot[j].MIN_VOULE << 8) + programme_day[i].day_dot[j].MAX_VOULE;

						k = STM32_FLASH_write_word((BANK1_WRITE_START_ADDR - i*0x400 + j*4),temp_data);
						if(k == 1)
							break;
					}
					else
					{
							programme_day[i].SET_status = 0;
							break;
					}
				}
			}
	}

}

void MONI_EEPROM_write1(void)
{
	 UINT32 temp_data;


	/* Unlock the Flash Program Erase controller */
	FLASH_Unlock();		//-第一步

	/* Clear All pending flags */
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);		//-这个是后来加的,必要性需要考虑

  /* Erase the FLASH pages */
  //-for(EraseCounter = 0; (EraseCounter < NbrOfPage) && (FLASHStatus == FLASH_COMPLETE); EraseCounter++)
  //{
    FLASHStatus = FLASH_ErasePage(BANK2_WRITE_START_ADDR);//- + (FLASH_PAGE_SIZE * EraseCounter));		//-以页为单位擦除
  //}

  //-while((Address < BANK1_WRITE_END_ADDR) && (FLASHStatus == FLASH_COMPLETE))
  //{
    FLASHStatus = FLASH_ProgramWord(voice_flag_EEP, voice_flag);		//-32 位字写入函数，其他分别为 16 位半字写入和用户选择字节写入函数
    //-Address = Address + 4;
  //}

  temp_data = (*(UINT32*) voice_flag_EEP);

  if(temp_data == voice_flag)		//-修改不正确了,会继续修改直到正确为止
  	EEP_Data_flag = 0;			//-这里需要修改,不对


		FLASH_Lock();		//-结束修改
}

/*
把特定的位置定义为特定的变量,然后上电后直接读取变量值进行调用.当需要修改时,首先把变量值记录下来.
然后在主循环中查询,一直等到屏幕熄灭后开始写入FLASH,平时调用还使用RAM中的数值,重新上电后才从FLASH
中读取.
一个数据占用两位,紧接着的一位数据是上个数据的取反,这样可以验证数据是否填写正确,可以简单的读出原数据
后保证数据正确了,就可以,目前不考虑FLASH损坏的情况.

正常情况下通讯随时可以修改RAM里面的数值,并且这个数据是有效的,一但屏幕熄灭之后这个数值就会写入到FLASH
中进行永久保存,如果本身就是熄灭状态,就会立即进行修改保存.

定义的变量:
UINT8		led_display_long;			UINT32 led_display_long_EEP;
*/
void MONI_EEPROM_sub(void)
{
	 MONI_EEPROM_write();
	 if(EEP_Data_flag == 0) //-只有上一个修改成功了才有必要修改下一个
	 {
	 	  EEP_Data_flag = 0x55;
	 		MONI_EEPROM_write1();
	 }
}
























































