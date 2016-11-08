#include "user_conf.h"

/*
SHT3x-DIS

normal temperature and humidity range of 5 �C
60 ��C and 20 �C 80 %RH

���������� ��Ҫ����
*/

//-�ڲ�ʹ�õ��ٶ���
#define  INT8U  char
#define  uint8  unsigned char
#define  int8   char

typedef unsigned char   u8t;      ///< range: 0 .. 255
typedef signed char     i8t;      ///< range: -128 .. +127

typedef unsigned short    u16t;     ///< range: 0 .. 65535
typedef signed short      i16t;     ///< range: -32768 .. +32767

typedef unsigned long   u32t;     ///< range: 0 .. 4'294'967'295
typedef signed long     i32t;     ///< range: -2'147'483'648 .. +2'147'483'647

//-typedef signed long  ft;       ///< range: +-1.18E-38 .. +-3.39E+38
typedef float  ft;
typedef double          dt;       ///< range:            .. +-1.79E+308

//-int       Value;
//-ft        temperature; // temperature [�C]
//-ft        humidity;    // relative humidity [%RH]

typedef enum{
  CMD_READ_SERIALNBR  = 0x3780, // read serial number
  CMD_READ_STATUS     = 0xF32D, // read status register
	CMD_CLEAR_STATUS    = 0x3041, // clear status register
	CMD_HEATER_ENABLE   = 0x306D, // enabled heater
	CMD_HEATER_DISABLE  = 0x3066, // disable heater
  CMD_SOFT_RESET      = 0x30A2, // soft reset
	CMD_MEAS_CLOCKSTR_H = 0x2C06, // meas. clock stretching, high rep.
	CMD_MEAS_CLOCKSTR_M = 0x2C0D, // meas. clock stretching, medium rep.
	CMD_MEAS_CLOCKSTR_L = 0x2C10, // meas. clock stretching, low rep.
	CMD_MEAS_POLLING_H  = 0x2400, // meas. no clock stretching, high rep.
	CMD_MEAS_POLLING_M  = 0x240B, // meas. no clock stretching, medium rep.
	CMD_MEAS_POLLING_L  = 0x2416, // meas. no clock stretching, low rep.
	CMD_MEAS_PERI_05_H  = 0x2032, // meas. periodic 0.5 mps, high rep.
	CMD_MEAS_PERI_05_M  = 0x2024, // meas. periodic 0.5 mps, medium rep.
	CMD_MEAS_PERI_05_L  = 0x202F, // meas. periodic 0.5 mps, low rep.
	CMD_MEAS_PERI_1_H   = 0x2130, // meas. periodic 1 mps, high rep.
	CMD_MEAS_PERI_1_M   = 0x2126, // meas. periodic 1 mps, medium rep.
	CMD_MEAS_PERI_1_L   = 0x212D, // meas. periodic 1 mps, low rep.
	CMD_MEAS_PERI_2_H   = 0x2236, // meas. periodic 2 mps, high rep.
	CMD_MEAS_PERI_2_M   = 0x2220, // meas. periodic 2 mps, medium rep.
	CMD_MEAS_PERI_2_L   = 0x222B, // meas. periodic 2 mps, low rep.
	CMD_MEAS_PERI_4_H   = 0x2334, // meas. periodic 4 mps, high rep.
	CMD_MEAS_PERI_4_M   = 0x2322, // meas. periodic 4 mps, medium rep.
	CMD_MEAS_PERI_4_L   = 0x2329, // meas. periodic 4 mps, low rep.
	CMD_MEAS_PERI_10_H  = 0x2737, // meas. periodic 10 mps, high rep.
	CMD_MEAS_PERI_10_M  = 0x2721, // meas. periodic 10 mps, medium rep.
	CMD_MEAS_PERI_10_L  = 0x272A, // meas. periodic 10 mps, low rep.
	CMD_FETCH_DATA      = 0xE000, // readout measurements for periodic mode
	CMD_R_AL_LIM_LS     = 0xE102, // read alert limits, low set
	CMD_R_AL_LIM_LC     = 0xE109, // read alert limits, low clear
	CMD_R_AL_LIM_HS     = 0xE11F, // read alert limits, high set
	CMD_R_AL_LIM_HC     = 0xE114, // read alert limits, high clear
	CMD_W_AL_LIM_LS     = 0x6100, // write alert limits, low set
	CMD_W_AL_LIM_LC     = 0x610B, // write alert limits, low clear
	CMD_W_AL_LIM_HS     = 0x611D, // write alert limits, high set
	CMD_W_AL_LIM_HC     = 0x6116, // write alert limits, high clear
  CMD_NO_SLEEP        = 0x303E,
}etCommands;

typedef enum{
  NO_ERROR       = 0x00, // no error
  ACK_ERROR      = 0x01, // no acknowledgment error
  CHECKSUM_ERROR = 0x02, // checksum mismatch error
  TIMEOUT_ERROR  = 0x04, // timeout error
  PARM_ERROR     = 0x80, // parameter out of range error
}etError;




//-C��CRC8У���Դ���룺
//   CRC8У��
//Polynomial 0x31  (X^8 + X^2 + X^1 + 1)
//Initialization 0xFF
//Final XOR 0x00
//-Examples CRC (0xBEEF) = 0x92

unsigned char SHT3X_CRC8(unsigned char *data,unsigned char datalen)
{
  unsigned char init_CRC = 0xFF;
  unsigned char genPoly = 0x31;

  for (char i=0; i<datalen; i++)
  {
    init_CRC ^= data[i];

    for(char j = 0; j<8; j++)
    {
        if(init_CRC & 0x80 )
            init_CRC = (init_CRC << 1) ^ genPoly;
        else
            init_CRC <<= 1;
    }
  }
  return init_CRC;
}




int SHT3X_StartPeriodicMeasurment(etRepeatab  repeatab,etFrequency freq)		//-��ʼ���ڲ���
{
	  struct i2c_client client;
	  u8 WriteBuffer_temp[2];
	  int res;

	  client.addr = 0x88;                   // ���ӻ���ַд�����ݰ�pa12201001�ĵ�ַ����0x3C
    client.flags = 0;
    client.num = 2;		//-Ӳ���˿ں�1,I2C1

	  // use depending on the required repeatability and frequency,
	  // the corresponding command
		switch(repeatab)
		{
			case REPEATAB_LOW: // low repeatability
				switch(freq)
				{
					case FREQUENCY_HZ5:  // low repeatability,  0.5 Hz
						WriteBuffer_temp[0] = CMD_MEAS_PERI_05_L >> 8;
						WriteBuffer_temp[1] = CMD_MEAS_PERI_05_L & 0xff;
						 break;
					case FREQUENCY_1HZ:  // low repeatability,  1.0 Hz
						WriteBuffer_temp[0] = CMD_MEAS_PERI_1_L >> 8;
						WriteBuffer_temp[1] = CMD_MEAS_PERI_1_L & 0xff;
					  break;
					case FREQUENCY_2HZ:  // low repeatability,  2.0 Hz
						WriteBuffer_temp[0] = CMD_MEAS_PERI_2_L >> 8;
						WriteBuffer_temp[1] = CMD_MEAS_PERI_2_L & 0xff;
					  break;
					case FREQUENCY_4HZ:  // low repeatability,  4.0 Hz
						WriteBuffer_temp[0] = CMD_MEAS_PERI_4_L >> 8;
						WriteBuffer_temp[1] = CMD_MEAS_PERI_4_L & 0xff;
						break;
					case FREQUENCY_10HZ: // low repeatability, 10.0 Hz
						WriteBuffer_temp[0] = CMD_MEAS_PERI_10_L >> 8;
						WriteBuffer_temp[1] = CMD_MEAS_PERI_10_L & 0xff;
						break;
					default:
						 //-PARM_ERROR;
						break;
				}
			  break;

			case REPEATAB_MEDIUM: // medium repeatability
				switch(freq)
				{
					case FREQUENCY_HZ5:  // medium repeatability,  0.5 Hz
						WriteBuffer_temp[0] = CMD_MEAS_PERI_05_M >> 8;
						WriteBuffer_temp[1] = CMD_MEAS_PERI_05_M & 0xff;
					  break;
					case FREQUENCY_1HZ:  // medium repeatability,  1.0 Hz
						WriteBuffer_temp[0] = CMD_MEAS_PERI_1_M >> 8;
						WriteBuffer_temp[1] = CMD_MEAS_PERI_1_M & 0xff;
						break;
					case FREQUENCY_2HZ:  // medium repeatability,  2.0 Hz
						WriteBuffer_temp[0] = CMD_MEAS_PERI_2_M >> 8;
						WriteBuffer_temp[1] = CMD_MEAS_PERI_2_M & 0xff;
						break;
					case FREQUENCY_4HZ:  // medium repeatability,  4.0 Hz
						WriteBuffer_temp[0] = CMD_MEAS_PERI_4_M >> 8;
						WriteBuffer_temp[1] = CMD_MEAS_PERI_4_M & 0xff;
						break;
					case FREQUENCY_10HZ: // medium repeatability, 10.0 Hz
						WriteBuffer_temp[0] = CMD_MEAS_PERI_10_M >> 8;
						WriteBuffer_temp[1] = CMD_MEAS_PERI_10_M & 0xff;
						break;
					default:
						 //-PARM_ERROR;
             break;
				}
			  break;

			case REPEATAB_HIGH: // high repeatability
				switch(freq)
				{
					case FREQUENCY_HZ5:  // high repeatability,  0.5 Hz
						WriteBuffer_temp[0] = CMD_MEAS_PERI_05_H >> 8;
						WriteBuffer_temp[1] = CMD_MEAS_PERI_05_H & 0xff;
						break;
					case FREQUENCY_1HZ:  // high repeatability,  1.0 Hz
						WriteBuffer_temp[0] = CMD_MEAS_PERI_1_H >> 8;
						WriteBuffer_temp[1] = CMD_MEAS_PERI_1_H & 0xff;
						break;
					case FREQUENCY_2HZ:  // high repeatability,  2.0 Hz
						WriteBuffer_temp[0] = CMD_MEAS_PERI_2_H >> 8;
						WriteBuffer_temp[1] = CMD_MEAS_PERI_2_H & 0xff;
						break;
					case FREQUENCY_4HZ:  // high repeatability,  4.0 Hz
						WriteBuffer_temp[0] = CMD_MEAS_PERI_4_H >> 8;
						WriteBuffer_temp[1] = CMD_MEAS_PERI_4_H & 0xff;
						break;
					case FREQUENCY_10HZ: // high repeatability, 10.0 Hz
						WriteBuffer_temp[0] = CMD_MEAS_PERI_10_H >> 8;
						WriteBuffer_temp[1] = CMD_MEAS_PERI_10_H & 0xff;
						break;
					default:
						//-PARM_ERROR;
						break;
				}
			  break;
			default:
				   //-PARM_ERROR;
				   break;
		}

		res = i2c_write_reg_Buffer(&client, WriteBuffer_temp, 2);
    if(res <= 0)
		{//-��ֵ�ɹ��Ļ��ͼ����ж�
 			 return res;
		}
    return res;
}

//-ft SHT3X_temp_date;
u16t SHT3X_CalcTemperature(u16t rawValue)   //-�õ��Ľ����������ʮ�����޷�����
{
  ft temp_date;
  u16t temp_date2;
//==============================================================================
  // calculate temperature [�C]
  // T = -45 + 175 * rawValue / (2^16-1)
  //-return 175 * (ft)rawValue / 65535 - 45;
  temp_date = (ft)rawValue / (ft)65535;
  //-temp_date2 = (u16t)((ft)1750 * temp_date - 450);   //-������ʾ�¶�,�����Ǽ�ȥ��ƫ�Ƶ��¶�
  if((ft)1750 * temp_date >= 450 + menu_set_tt_cal * 10)
  {
    temperature_data_flag = 0;
    temp_date2 = (u16t)((ft)1750 * temp_date - 450 - menu_set_tt_cal * 10);
  }
  else
  {
    temperature_data_flag = 1;
    temp_date2 = (u16t)(450 + menu_set_tt_cal * 10 - (ft)1750 * temp_date);
  }
  //-SHT3X_temp_date = temp_date2;
  return temp_date2;
}

u16t SHT3X_CalcHumidity(u16t rawValue)
{
	ft temp_date;
	u16t temp_date2;
//==============================================================================
  // calculate relative humidity [%RH]
  // RH = rawValue / (2^16-1) * 100
  //-return 100 * (ft)rawValue / 65535;
  temp_date = (ft)rawValue / 65535;
  temp_date2 = (u16t)(temp_date * 100);
  return temp_date2;
}

void SHT3X_ReadMeasurementBuffer(u16t *temp, u16t *humi)	//-ͨ��I2C�ӿ�ֱ�Ӳ�ѯ����������
{
//==============================================================================
    struct i2c_client client;
	  u8 WriteBuffer_temp[2];
	  u8 ReadBuffer_temp[10];
	  int res;
	  UINT16 t_data,humi_data;
	  UINT16 		temp_data;
	  //-etError  error;        // error code
		u16t     rawValueTemp; // temperature raw value from sensor
	  u16t     rawValueHumi; // humidity raw value from sensor


	  client.addr = 0x88;                   // ���ӻ���ַд�����ݰ�pa12201001�ĵ�ַ����0x3C
    client.flags = 0;
    client.num = 2;		//-Ӳ���˿ں�1,I2C1

    SHT3X_err_flag++;

	  WriteBuffer_temp[0] = CMD_FETCH_DATA >> 8;
	  WriteBuffer_temp[1] = CMD_FETCH_DATA & 0xff;

	  res = i2c_write_reg_Buffer(&client, WriteBuffer_temp, 2);
    if(res <= 0)
		{//-��ֵ�ɹ��Ļ��ͼ����ж�
 			 return;
		}


		//-SHT3X_StartWriteAccess();
	  char T_MSB=0,T_LSB=0,H_MSB=0,H_LSB=0,T_CRC=0,H_CRC=0;
		// if no error, read measurements
	  //-SHT3X_WriteCommand(CMD_FETCH_DATA);

	  res = i2c_read_data_Buffer(&client, 6, ReadBuffer_temp);
    if(res <= 0)
		{//-��ֵ�ɹ��Ļ��ͼ����ж�
 			 return;
		}

    //-������
    SHT3X_err_flag = 0;

	  T_MSB=ReadBuffer_temp[0];
	  T_LSB=ReadBuffer_temp[1];
	  T_CRC=ReadBuffer_temp[2];
	  H_MSB=ReadBuffer_temp[3];
	  H_LSB=ReadBuffer_temp[4];
	  H_CRC=ReadBuffer_temp[5];
	  rawValueTemp= (T_MSB << 8) | T_LSB;
	  rawValueHumi= (H_MSB << 8) | H_LSB;

	  temp_data =  SHT3X_CRC8(&ReadBuffer_temp[0],2);
	  if(temp_data == T_CRC)
	  {
	  	 t_data = SHT3X_CalcTemperature(rawValueTemp);
	  	 //-t_data = SHT3X_CalcTemperature(16477);		//-ģ���-10��
		   //-if(t_data >= 0x8000)	//-�����¼�����޷�����,�����Ǹ�������������Ҫ��Ӧ�ó����д���
		   //  t_data = 0 - t_data;
		   *temp = t_data;

		   port_send_sense_data[2] = t_data;  //-����ոն�ȡ���¶�,�������ʱʱ�������ݷ���,���������������Ʋ�����,��ô�����в�������
       
       if(t_data%10 > 4)  //-��������޸��˳�����ֵΪ0��bug
          temp_data = (t_data/10) * 10 + 5;
       else
          temp_data = (t_data/10) * 10;
	  }


	  //-����ͨ��ֵ,ΪͨѶ��
          if((temperature_data != temp_data) && (UART1_transmit_control != 4))
          {
          	 //-port_send_sense_data[2] = temperature_data;

             UART1_transmit_control = 4;  //-��ֵ�仯���Զ��ϴ���������
             UART1_transmit_flag=YES;
          }
          
	  temp_data =  SHT3X_CRC8(&ReadBuffer_temp[3],2);
	  if(temp_data == H_CRC)
	  {
	  	 humi_data = SHT3X_CalcHumidity(rawValueHumi);
		   //-if(humi_data >= 0x8000)
		   //-  humi_data = 0 - humi_data;
		   *humi = humi_data;

		   //-port_send_sense_data[3] = humi_data;
	  }

	  //-����ͨ��ֵ,ΪͨѶ��
          if(humi_data != port_send_sense_data[3])
          {
          	 port_send_sense_data[3] = humi_data;

             //-UART1_transmit_control = 4;
             //-UART1_transmit_flag=YES;
          }
          //-else
          //-	 temp_data = port_send_sense_data[3] - humi_data;

          //-if(temp_data > 30)
          //-{
          //-	 port_send_sense_data[0] = 0x55;
          //-}

}



//-��һ����ʪ������
void SHT3X_StartPeriodicMeasurment2(etRepeatab  repeatab,etFrequency freq)		//-��ʼ���ڲ���
{
	  struct i2c_client client;
	  u8 WriteBuffer_temp[2];
	  int res;

	  client.addr = 0x88;                   // ���ӻ���ַд�����ݰ�pa12201001�ĵ�ַ����0x3C
    client.flags = 0;
    client.num = 2;		//-Ӳ���˿ں�1,I2C1

	  // use depending on the required repeatability and frequency,
	  // the corresponding command
		switch(repeatab)
		{
			case REPEATAB_LOW: // low repeatability
				switch(freq)
				{
					case FREQUENCY_HZ5:  // low repeatability,  0.5 Hz
						WriteBuffer_temp[0] = CMD_MEAS_PERI_05_L >> 8;
						WriteBuffer_temp[1] = CMD_MEAS_PERI_05_L & 0xff;
						 break;
					case FREQUENCY_1HZ:  // low repeatability,  1.0 Hz
						WriteBuffer_temp[0] = CMD_MEAS_PERI_1_L >> 8;
						WriteBuffer_temp[1] = CMD_MEAS_PERI_1_L & 0xff;
					  break;
					case FREQUENCY_2HZ:  // low repeatability,  2.0 Hz
						WriteBuffer_temp[0] = CMD_MEAS_PERI_2_L >> 8;
						WriteBuffer_temp[1] = CMD_MEAS_PERI_2_L & 0xff;
					  break;
					case FREQUENCY_4HZ:  // low repeatability,  4.0 Hz
						WriteBuffer_temp[0] = CMD_MEAS_PERI_4_L >> 8;
						WriteBuffer_temp[1] = CMD_MEAS_PERI_4_L & 0xff;
						break;
					case FREQUENCY_10HZ: // low repeatability, 10.0 Hz
						WriteBuffer_temp[0] = CMD_MEAS_PERI_10_L >> 8;
						WriteBuffer_temp[1] = CMD_MEAS_PERI_10_L & 0xff;
						break;
					default:
						 //-PARM_ERROR;
						break;
				}
			  break;

			case REPEATAB_MEDIUM: // medium repeatability
				switch(freq)
				{
					case FREQUENCY_HZ5:  // medium repeatability,  0.5 Hz
						WriteBuffer_temp[0] = CMD_MEAS_PERI_05_M >> 8;
						WriteBuffer_temp[1] = CMD_MEAS_PERI_05_M & 0xff;
					  break;
					case FREQUENCY_1HZ:  // medium repeatability,  1.0 Hz
						WriteBuffer_temp[0] = CMD_MEAS_PERI_1_M >> 8;
						WriteBuffer_temp[1] = CMD_MEAS_PERI_1_M & 0xff;
						break;
					case FREQUENCY_2HZ:  // medium repeatability,  2.0 Hz
						WriteBuffer_temp[0] = CMD_MEAS_PERI_2_M >> 8;
						WriteBuffer_temp[1] = CMD_MEAS_PERI_2_M & 0xff;
						break;
					case FREQUENCY_4HZ:  // medium repeatability,  4.0 Hz
						WriteBuffer_temp[0] = CMD_MEAS_PERI_4_M >> 8;
						WriteBuffer_temp[1] = CMD_MEAS_PERI_4_M & 0xff;
						break;
					case FREQUENCY_10HZ: // medium repeatability, 10.0 Hz
						WriteBuffer_temp[0] = CMD_MEAS_PERI_10_M >> 8;
						WriteBuffer_temp[1] = CMD_MEAS_PERI_10_M & 0xff;
						break;
					default:
						 //-PARM_ERROR;
             break;
				}
			  break;

			case REPEATAB_HIGH: // high repeatability
				switch(freq)
				{
					case FREQUENCY_HZ5:  // high repeatability,  0.5 Hz
						WriteBuffer_temp[0] = CMD_MEAS_PERI_05_H >> 8;
						WriteBuffer_temp[1] = CMD_MEAS_PERI_05_H & 0xff;
						break;
					case FREQUENCY_1HZ:  // high repeatability,  1.0 Hz
						WriteBuffer_temp[0] = CMD_MEAS_PERI_1_H >> 8;
						WriteBuffer_temp[1] = CMD_MEAS_PERI_1_H & 0xff;
						break;
					case FREQUENCY_2HZ:  // high repeatability,  2.0 Hz
						WriteBuffer_temp[0] = CMD_MEAS_PERI_2_H >> 8;
						WriteBuffer_temp[1] = CMD_MEAS_PERI_2_H & 0xff;
						break;
					case FREQUENCY_4HZ:  // high repeatability,  4.0 Hz
						WriteBuffer_temp[0] = CMD_MEAS_PERI_4_H >> 8;
						WriteBuffer_temp[1] = CMD_MEAS_PERI_4_H & 0xff;
						break;
					case FREQUENCY_10HZ: // high repeatability, 10.0 Hz
						WriteBuffer_temp[0] = CMD_MEAS_PERI_10_H >> 8;
						WriteBuffer_temp[1] = CMD_MEAS_PERI_10_H & 0xff;
						break;
					default:
						//-PARM_ERROR;
						break;
				}
			  break;
			default:
				   //-PARM_ERROR;
				   break;
		}

		res = i2c_write_reg_Buffer(&client, WriteBuffer_temp, 2);
    if(res <= 0)
		{//-��ֵ�ɹ��Ļ��ͼ����ж�
 			 return;
		}
}


void SHT3X_ReadMeasurementBuffer2(u16t *temp, u16t *humi)	//-ͨ��I2C�ӿ�ֱ�Ӳ�ѯ����������
{
//==============================================================================
    struct i2c_client client;
	  u8 WriteBuffer_temp[2];
	  u8 ReadBuffer_temp[10];
	  int res;
	  ft t_data,humi_data;
	  UINT16 		temp_data;

	  client.addr = 0x88;                   // ���ӻ���ַд�����ݰ�pa12201001�ĵ�ַ����0x3C
    client.flags = 0;
    client.num = 2;		//-Ӳ���˿ں�1,I2C1

	  //-etError  error;        // error code
		u16t     rawValueTemp; // temperature raw value from sensor
	  u16t     rawValueHumi; // humidity raw value from sensor


	  WriteBuffer_temp[0] = CMD_FETCH_DATA >> 8;
	  WriteBuffer_temp[1] = CMD_FETCH_DATA & 0xff;

	  res = i2c_write_reg_Buffer(&client, WriteBuffer_temp, 2);
    if(res <= 0)
		{//-��ֵ�ɹ��Ļ��ͼ����ж�
 			 return;
		}


		//-SHT3X_StartWriteAccess();
	  char T_MSB=0,T_LSB=0,H_MSB=0,H_LSB=0,T_CRC=0,H_CRC=0;
		// if no error, read measurements
	  //-SHT3X_WriteCommand(CMD_FETCH_DATA);

	  //-client.addr = 0x88;                   // ���ӻ���ַд�����ݰ�pa12201001�ĵ�ַ����0x3C
    //-client.flags = 0;
    //-client.num = 2;		//-Ӳ���˿ں�1,I2C1

	  res = i2c_read_data_Buffer(&client, 6, ReadBuffer_temp);
    if(res <= 0)
		{//-��ֵ�ɹ��Ļ��ͼ����ж�
 			 return;
		}

	  T_MSB=ReadBuffer_temp[0];
	  T_LSB=ReadBuffer_temp[1];
	  T_CRC=ReadBuffer_temp[2];
	  H_MSB=ReadBuffer_temp[3];
	  H_LSB=ReadBuffer_temp[4];
	  H_CRC=ReadBuffer_temp[5];
	  rawValueTemp= (T_MSB << 8) | T_LSB;
	  rawValueHumi= (H_MSB << 8) | H_LSB;

	  temp_data =  SHT3X_CRC8(&ReadBuffer_temp[0],2);
	  if(temp_data == T_CRC)
	  {
	  	 t_data = SHT3X_CalcTemperature(rawValueTemp);
		   if(t_data >= 0x80000000)
		  	  t_data = 0 - t_data;
		   *temp = (u16t)t_data;

		   port_send_sense_data[6] = (u16t)t_data;
	  }


	  //-����ͨ��ֵ,ΪͨѶ��
          //-if(temperature_data >= port_send_sense_data[2])
          //-{
          //-	 temp_data = temperature_data - port_send_sense_data[2];
          //-}
          //-else
          //-	 temp_data = port_send_sense_data[2] - temperature_data;

          //-if(temp_data > 30)
          //-{
          //-	 port_send_sense_data[0] = 0x55;
          //-}
	  temp_data =  SHT3X_CRC8(&ReadBuffer_temp[3],2);
	  if(temp_data == H_CRC)
	  {
	  	 humi_data = SHT3X_CalcHumidity(rawValueHumi);
		   if(humi_data >= 0x80000000)
		  	  humi_data = 0 - humi_data;
		   *humi = (u16t)humi_data;

		   //-port_send_sense_data[3] = humi_data;
	  }

	  //-����ͨ��ֵ,ΪͨѶ��
          //-if(humi_data >= port_send_sense_data[3])
          //-{
          //-	 temp_data = humi_data - port_send_sense_data[3];
          //-}
          //-else
          //-	 temp_data = port_send_sense_data[3] - humi_data;

          //-if(temp_data > 30)
          //-{
          //-	 port_send_sense_data[0] = 0x55;
          //-}

}

////////////////////////////////////////////////////////////////////////////////
//-�ضȱ� -10 �ȵ� 105 �� (�� --> С)
const unsigned short v_table[116]=
{
   4095,4000,4500,4400,4300,4200,4100,4000,3900,3800,   //-10 ----  -1
   3700,3600,3500,3400,3300,3250,3150,3050,2950,2850,//0 ----  9
   2750,2650,2550,2450,2350,2250,2150,2950,2850,2750,//10 ----  19
   2650,2450,2353,2250,2140,2040,1960,1860,1790,1710,//20 ----  29
   1620,1550,1450,1250,1150,1050,950,850,750,650,//30 ----  39
   3183,3224,3265,3304,3344,3382,3420,3457,3493,3529,//40 ----  49
   3564,3599,3632,3665,3698,3729,3760,3791,3820,3849,//50 ----  59
   3878,3905,3932,3959,3985,4010,4034,4058,4082,4105,//60 ----  69
   4127,4149,4170,4191,4211,4231,4250,4269,4287,4304,//70 ----  79
   4322,4339,4355,4371,4387,4402,4417,4431,4445,4459,//80 ----  89
   4472,4485,4497,4510,4522,4533,4545,4556,4566,4577,//90 ----  99
	 4587,4597,4607,4616,4625,300										 //100 ----  105
};

/*
��������:
���¶�ϵ������������(PTC)�͸��¶�ϵ������������(NTC)��
���¶�ϵ������������(PTC)���¶�Խ��ʱ����ֵԽ�󣬸��¶�ϵ������������(NTC)���¶�Խ��ʱ����ֵԽ��

NTC����ֵ�ɽ��Ʊ�ʾΪ:
Rt = RT *EXP(Bn*(1/T-1/T0)
ʽ��RT��RT0�ֱ�Ϊ�¶�T��T0ʱ�ĵ���ֵ��BnΪ���ϳ���.�մɾ������������¶ȱ仯��ʹ�����ʷ����仯�������ɰ뵼�����Ծ�����.

Ҫȷ������������Χ���¶ȣ������Խ���Steinhart-Hart��ʽ:T=1/(A0+A1(lnRT)+A3(lnRT3))��ʵ�֡�
���У�TΪ�����¶�;RTΪ�����������¶�Tʱ����ֵ;�� A0��A1��A3�����������������������ṩ�ĳ�����

*/
UINT16 NTC_CalcTemperature(UINT16 v)	//-���¶�ϵ����������������¶�
{
	 UINT8 x,y;
   UINT16 v_big,v_small,v_step;

   if (v<100) return 0xFFFF; // ���]����̖�r�@ʾ-99.9�e�`

   for (x=0;x<115;x++) // ���
   {
      if (v >= v_table[x]) // �ҳ�늉��^��
      {
         v_small = v_table[x]; // �^�� �߶�
         v_big = v_table[x-1]; //�^�� �Ͷ�
         v_step = (v_big - v_small)/10; // �х^�򼚷ֳ�10��
         for (y=0;y<10;y++) // ���ֱ��^
         {
            if (v > v_big) // �ó��Y��
            {
               v = (x-1)*10+y;// ����x*10 ����������, y ��С������
               if ((x-1)<10) // ���10�r��ؓ�ض�
               {
               	  v = 100 - v;
                  v |= 0x8000; // ����̖ؓ��ӛ
               }
               else
               {
                  v -= 100; // 0 ������
               }
                   return v;
            }
            v_big -= v_step;
         }
      }
   }
  return v;

}

/////////////////////////////////////////



/*
STM32�ڲ��¶� ת������ֵΪ�¶�
ADCת�������Ժ󣬶�ȡADC_DR�Ĵ����еĽ����ת���¶�ֵ���㹫ʽ���£�
        V25 - VSENSE
T(��) = ------------ + 25
          Avg_Slope
V25�� �¶ȴ�������25��ʱ �������ѹ������ֵ1.43 V��
VSENSE���¶ȴ������ĵ�ǰ�����ѹ����ADC_DR �Ĵ����еĽ��ADC_ConvertedValue֮���ת����ϵΪ��
          ADC_ConvertedValue * Vdd
VSENSE = --------------------------
          Vdd_convert_value(0xFFF)
Avg_Slope���¶ȴ����������ѹ���¶ȵĹ�������������ֵ4.3 mV/�档
//Converted Temperature
Vtemp_sensor = ADC_ConvertedValue * Vdd / Vdd_convert_value;
Current_Temp = (V25 - Vtemp_sensor)/Avg_Slope + 25;
*/
void get_ntc_adc_value(void) 	//-��ȡ��Ҫ�ĵ��ADC��ֵ
{
   UINT16 TEMP;

   TEMP = samp_data_ram_base[1][2];
   NTC_data = NTC_CalcTemperature(TEMP);

   watch_data[0]=(u16t)((1.43-samp_data_ram_base[2][2]*3.3/4096)*1000/4.35 + 25);
}


void SHT3X_init(void)
{
   int i;

  i = SHT3X_StartPeriodicMeasurment(REPEATAB_HIGH, FREQUENCY_1HZ);  /* ��ʪ�ȴ�����IIC�ٶ����� */
  if(i <= 0)
  {
    SHT3X_status = 0;
  }
  else
    SHT3X_status = 0x55;
}
