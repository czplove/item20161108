#ifndef __COMMON_H__
#define __COMMON_H__

#define YES       0xaa
#define NO        0x55

//-#define BYTE      unsigned char
//-#define WORD      unsigned short
#define UL        unsigned long

#define int16     short
#define Uint16    unsigned short

#define UINT8      unsigned char
#define UINT16     unsigned short
#define UINT32     unsigned long


#ifndef TRUE
#define  TRUE	   1    //-��Сд��Ӧ�ĺ��岻ͬ
#endif
#ifndef FALSE
#define  FALSE   0
#endif

//-typedef unsigned long  DWORD;
//-typedef short          bool;

//-#define BOOL    bool
#define true	1
#define false	0

#define LOBYTE(wValue)    ((BYTE)(wValue))
#define HIBYTE(wValue)    ((BYTE)((wValue) >> 8))

#define  Success  1
#define  Error    0

///////////////////////////////////////////////////////////////////////////////
#define MAX_MON_BUF_SIZE  			1024		// Max Monitor Buffer Size


///////////////////////////////////////////////////////////////////////////////
//-���ϵͳ��ͬ�������ı�������
//-#define I2C_HARDWARE_FLAG			0			//-���ʹ��STM32�����Ӳ��I2C,��ô�����Ӳ��I2C��־ֵ�͸�1
#define I2C_SOFTWARE_FLAG			1			//-
#define SYS_MODE_STOP			    1			//-����ϵͳģʽ��ѡ��
#define SYS_iwatch_DOG			  1			//-ѡ���Ƿ����ö������Ź�

///////////////////////////////////////////////////////////////////////////////

//-iap������

//-#define BANK5_WRITE_START_ADDR  ((UINT32)0x0807D800)
//-#define BANK5_WRITE_END_ADDR    ((UINT32)0x0807DFFF)
//-#define BANK4_WRITE_START_ADDR  ((UINT32)0x0807E000)
//-#define BANK4_WRITE_END_ADDR    ((UINT32)0x0807E7FF)
//-#define BANK3_WRITE_START_ADDR  ((UINT32)0x0807E800)
//-#define BANK3_WRITE_END_ADDR    ((UINT32)0x0807EFFF)

//-#define APP_CONFIG_ADDR 		(BANK3_WRITE_START_ADDR + 0x000)	//���õ�ַ,˵�����г����ѡ��
//-#define APP_CONFIG1_ADDR 		(BANK4_WRITE_START_ADDR + 0x000)	//���õ�ַ,˵���������������Ƿ���OK
//-#define APP_CONFIG2_ADDR 		(BANK4_WRITE_START_ADDR + 0x004)	//-��¼��������Ч����Ĵ�С,16λ����Ϊ��λ
//-#define APP_CONFIG3_ADDR 		(BANK5_WRITE_START_ADDR + 0x000)  //-��¼ϵͳ����汾��

#define STM32_SYS_VERSION_NUM     0x0001

//-STM32ϵͳ����
#define ADC1_DR_Address    ((u32)0x4001244C)
#define USART1_DR_Address    ((u32)0x40013804)
#define USART2_DR_Address    ((u32)0x40004404)
#define USART3_DR_Address    ((u32)0x40004804)


///////////////////////////////////////////////////////////////////////////////
#define PROGRAMME_DAY_dot_NUM    6

typedef struct{ WORD MIN;    // moni EXINT0 hold triggle level,not reset, so can't come into short again
                //-BYTE MIN;    // as above
                BYTE MIN_VOULE;    // as above
                BYTE MAX_VOULE;    // as above
              }PROGRAMME_TIME_CH;

typedef struct{ BYTE  SET_status;   //-ֵΪ0��ʾ�����Ѿ��洢,ֵΪ0x55��ʾֵ��Ҫд��EEP
                PROGRAMME_TIME_CH day_dot[PROGRAMME_DAY_dot_NUM];
              }PROGRAMME_DAY_CH;

#endif // __COMMON_H__


