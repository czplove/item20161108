/*
 �������Լ���ͷ�ļ�
 */
#include "stm32f10x.h"
#include "integer.h"
#include "user_conf.h"



WORD    port_recv_pt[4];
WORD    port_recv_dl[4];
WORD    port_send_pt[4];
WORD    port_send_dl[4];
WORD    port_send_len[4];

BYTE	port_recv[4][256];
BYTE	port_send[4][256]={1,2,3,4,5};
BYTE	UART3_port_send[256];
BYTE	port_deal_buf[4][256];
BYTE	port_report[256];
BYTE	port_deal_flag[4]; 		//-ֵΪ0���Դ���;ֵΪ0xaa��ʾ���ݻ�û�з��ͳ�ȥ�����Դ���
BYTE	UART3_TO_UART2_FLAG;

UL		samp_data_ch0_average;
UL		samp_data_ch1_average;
UL		samp_data_ch2_average;
UL		samp_data_ch3_average;
UL		samp_data_ch4_average;



/*struct L6_Ram
{								//start		len
	int16  samp_data_ram_base[4][128];			//0x0000	0x600  ��ͨ��ϵ���Ľ����
	int16  samp_data_cf_ram_base[4][128];			//0x0600	0x600
	Uint16 samp_data_power_ram_base[128];			//0x0c00	0x080
	int16  samp_data_yc_ram_base[13][128];			//0x0c80	0x240  end 0x0fef
}L6Ram;
*/

BYTE	samp_data_pt;

//-1	CO2
//-2  �¶�
//-3  ʪ��
//-4  PM2.5
//-5  ����
WORD    port_send_sense_data[20];			//-�����Ǵ洢�����п��ܵĴ���������,��һ��Ԫ����Ϊ��־λ,Ϊ0x55ʱ˵������ֵ��������Ҫ����;Ϊ0ʱҲ�Ǹո��µ���ֵ,ֻ��û�дﵽ
																			//-����Ҫ��


/*struct  NET_PORT_VAR{
Uint16	net_port_recv_pt[UIP_CONNS];
Uint16   net_port_recv_dl[UIP_CONNS];
UINT8    net_port_recv[UIP_CONNS][2048];
Uint16	net_port_send_pt[UIP_CONNS];
Uint16   net_port_send_len[UIP_CONNS];
UINT8    net_port_send[UIP_CONNS][2048];
UINT8    net_port_deal_buf[UIP_CONNS][512];
		    }net_port_var;*/


////////////////////////////////////////////////////////////
/* Monitor Buffer and Position Marker */
UINT8  monitor_buffer[MAX_MON_BUF_SIZE];
UINT16 monitor_in_pos = 0, monitor_out_pos = 0, monitor_out_pos_t = 0;

/* UART Channel x TX/RX Buffer Array */
//-UART_BUFFER mUART_TX_BUFFER[NO_UART_CPU];
//UART_BUFFER mUART_RX_BUFFER[NO_UART_CPU];

/* UART Device x Entry Array and Corresponding Pointer Array */
//UART_DEVICE_ENTRY  uart_device_task[NO_UART_CPU];
//UART_DEVICE_ENTRY   *fp_uart_device[NO_UART_CPU];

/* UART Channel x Initialize Function Pointer Array */
//-int (*fp_uart_device_init[NO_UART_CPU])(UART_DEVICE_ENTRY *);

/* Device Stste Definition */
UINT32 DeviceState =0;

UINT32 Device_communication_cn =0;		//-���Լ�¼ϵͳͨѶ���

/*
 * UART receive state, bit x for UART channel x
 * ->When byte was received to UART buffer, but not processed, set.
 * ->When byte was received to UART buffer, and processed, cleared.
 * ->When byte was not received to UART buffer, cleared.
*/
BYTE    serial_bus_recv_state=0;

/* Define Serial Port Idle Time->
 * ->When in receive state, clear serial_idle_counter when byte received
 * ->When in receive state, self-added serial_idle_counter when byte not received
 * ->When serial_idle_counter less than SERIAL_BUS_IDLE_TIME, one frame not ended
 */
BYTE    serial_idle_counter[3]={0,0,0};

/* Index Protocol Type for TCP or UDP */
BYTE  protocal_type;	// 1 for UDP, 0 for TCP

/* System date&time */
UINT16  m_year  =0;
UINT8  m_month =0;
UINT8  m_week  =0;  //-�������������ʾ����0 
UINT8  m_date  =0;
UINT8  m_hour  =0;
UINT8  m_min   =0;
UINT8  m_sec   =0;
UINT16 m_msec  =0;

UINT16  REG_year  =2016;
UINT8  REG_month =4;
UINT8  REG_week  =0;
UINT8  REG_date  =26;
UINT8  REG_hour  =10;
UINT8  REG_min   =21;
UINT8  REG_sec   =0;
UINT16 REG_msec  =0;


UINT16  cticks_ms; //-1msʱ���׼������

UINT16  Time_2048ms_Counter;
UINT16  cticks_5ms;    //-5msʱ���׼������
UINT16  cticks_500ms;
UINT16  cticks_ms_pwm;
UINT16  cticks_test;		//-���Դ�����ʱ��

UINT32  cticks_s;

UINT16  cticks_SLEEP_serial;    //-α�������߼�����	λ15�Ǳ�ʶλ 0 ������Ч	1 ������Ч	��15λΪ����ֵ(mS��)
UINT32  TEST_Sensor_wait_time;
UINT16  Sensor_data_wait_time;

///const unsigned char version_number[80] ={"DSA LINE NETGATE  ver 1.0"};

//-�������ƴ�����ʹ�õ���ʱȫ�ֱ���,������
UINT16	i2c1_alsdata_16;
UINT16  i2c1_psdata_16;
UINT8		i2c1_newdata_flag;		//-0 ��ʾû��������;1 ��ʾ��������
UINT8		i2c1_psinit_flag;		  //-0 ��ʾ��ʼ���ɹ�;1 ��ʾʧ��
UINT8		i2c1_psint_cn;		    //-��¼�жϳ���ʱ��
UINT8		i2c1_psint_flag;			//-0 ��ʾ�ж��ź���Ч;1 ��ʾ�ж��źŴ���,��ʼ�����жϺ���1,ֱ��������Ϊֹ
UINT8		i2c1_psint_flag_FX;
UINT8		i2c1_bus_error_cn;		//-���߳����ۼӶ�һ������֮�����³�ʼ������

UINT16	i2c2_alsdata_16;
UINT16  i2c2_psdata_16;
UINT8		i2c2_newdata_flag;	  //-0 ��ʾû��������;1 ��ʾ��������
UINT8		i2c2_psinit_flag;		  //-0 ��ʾ��ʼ���ɹ�;1 ��ʾʧ��
UINT8		i2c2_psint_cn;		    //-��¼�жϳ���ʱ��
UINT8		i2c2_psint_flag;			//-0 ��ʾ�ж��ź���Ч;1 ��ʾ�ж��źŴ���,��ʼ�����жϺ���1,ֱ��������Ϊֹ
UINT8		i2c2_psint_flag_FX;
UINT8		i2c2_bus_error_cn;
UINT8		i2c3_bus_error_cn;
UINT8		i2c4_bus_error_cn;
UINT8		i2c5_bus_error_cn;

UINT16  i2c3_psdata_16;
UINT16  i2c4_psdata_16;
UINT16  als_data;
UINT16  als_data2;
UINT16  als_data3;
UINT16  als_data4;
UINT8		ps_flag;		//-�Ӷ����Ƶ����	0 û�лӶ�;1 ���ϵ���;2 ���µ���
UINT32		ps1_int_time;
UINT32		ps2_int_time;
UINT16		VCNL_poll_wait_time;
//-ADI������
UINT16  ADI_CH1_OFFSET;
UINT16  ADI_CH2_OFFSET;
UINT16  ADI_CH3_OFFSET;
UINT16  ADI_CH4_OFFSET;
UINT16  ADI_DATA_BUFFER_OUT;
UINT16  ADI_DSAMPLE_TIME;
UINT16  ADI_DEC_MODE;
UINT16  ADI_INT_STATUS;
UINT32 	ADI_X1 = 0;
UINT32 	ADI_X2 = 0;
UINT32 	ADI_Y1 = 0;
UINT32 	ADI_Y2 = 0;
UINT16  ADI_PS_Value1;


//-��������ʾ��ʱ����
UINT8		led_display_flag;			//-��Ϊһҳ����,��ʾ�ķ�ʽ��־
UINT8		led_display_y;        //-��ʾ����ʾ���к����ֵ,�����ڶ����е���ʾ����
UINT8		led_display_cn;        //-��ʾ�����жϵĴ���
UINT8		led_display_cn_f;
UINT8		led_display_new;			//-��ʾ�Ƿ�����������Ҫ׼�� 0x55��ʾ����׼��������;	0xaa��ʾ����׼������; 0xa5��ʾ���ڶ���������.
UINT16  led_display_data;			//-һҳ�����ʾ��λ����,���������999
UINT8		led_display_num;				//-����������������ʾ:ֵ1 2���������澲̬��ʾ��,5 6��ͨѶ��ʾͼƬ
UINT8		led_display_txpic_flag;				//-0x55 ��ʾ��ʾͨѶ�·���ͼƬ 0xaa ��ʾͼƬ�Ѿ�ˢ����,�������Լ���ˢ��;���ƽ���ˢͼ,�ָ�������ʾ
UINT8		led_display_txpic_num;				//-ˢ�̶�ͼƬ��ƫ����
UINT16	cartoon_end_wait_time;		//-��¼һ��������ʼ��ʱ���Ա�ȷ��ʲôʱ�����
UINT32	led_display_data1[16];	//-������Ҫ��ʾ���ݵ�����
UINT32	led_display_data2[16];	//-������Ҫ��ʾ���ݵ�����
UINT32	led_display_data3[16];	//-������Ҫ��ʾ���ݵ�����,ר����ʾ����Ч����,����������
UINT32	led_display_data4[16];	//-������Ҫ��ʾ���ݵ�����,ר����ʾ����Ч����,��̬����������
UINT32	led_display_data5[16];	//-ͨѶ��ʾ
UINT32	led_display_data6[16];
UINT32	led_display_data_temp[16];	//-��ʱ�������ݵ�ȫ�ֱ���
UINT8		led_data_x;							//-��ʾ���������ݵ��е�����λ��
UINT8		led_data_y;
UINT32	led_display_data_round[32];		//-һ��ȫ�ֱ���������תЧ��ͼ
UINT32	led_display_data_round1[32];		//-��Ϊ��תЧ��ͼ��һ���м����

UINT8		led_display_page;		//-��¼Ŀǰ��ʾ������һҳ
UINT8		led_display_page_end;		//-��¼Ŀǰϵͳ������ʾ�����ҳ��
UINT8		led_display_start;		//-ֵΪ0x55˵�����ڴ���ˢ��״̬,0xaa ��ʾϨ��״̬,0��ʾ����Ϩ��
UINT8   cticks_s_page;        //-û�д���������ʱһ��ʱ���,Ϩ����Ļ
UINT8		led_display_long;			//-����һ������ʱ��,����ʵ��������ʱ,Ȼ��ʵ�ֵ��籣��
UINT16	led_display_end_time;	//-������Ϩ��֮���ʱ��

UINT8		ps_flag_led;			//-�Ӷ����Ƶ����	0 û�лӶ�;1 ���ϵ���;2 ���µ���,���ڴ洢��־,��������ʹ��,������ҪӰ����ps_flag��־
UINT8		ps_flag_led_end;	//-��¼���һ�λӶ�������
UINT8   led_display_move_pt;		//-�����ƶ�ʱ�����õ�,�൱��һ��ʱ���׼

UINT8		led_display_data_flag;		//-��ʾ�����������Ƿ������µ�:0 û������,��Ҫ�ȴ�����;1 �������ݿ�ʼ��ʱ;2 ������Ч;3 ����ʧЧ��ʼ��ʱ

//-���Թ۲���ȫ�ֱ���
UINT8	temp_data01_8;
UINT8	temp_data02_8;
UINT8	temp_data03_8;
UINT8	temp_data04_8;

UINT16	temp_data01;
UINT16	temp_data02;
UINT16	temp_data03;
UINT16	temp_data04;


UINT16		watch_data[512];		//-ͨ�ù۲�����������
UINT16		watch_data_pt;

UINT8		watch_cfgdata;
UINT16		watch_cfgdata_16;

//-��������
/*
BIN5		BIN4		BIN3		BIN2		BIN1		GUARD		Shield
CS3			CS1			CS0			CS4			CS5			CS2			CS15
D5      D4      D0      D1      D3			����
*/
UINT8		CSKEY_cfgDATA;		//-�����ļĴ�������
UINT8		CSKEY_DATA;				//-����ļ�ֵ


//-by cheng CO2
UINT16 		Value_CO2_Buffer[10];
UINT8		  CO2_Counter=0;
char      ReadBuf_CO2[30];
int       Received_Over_time_co2,Received_Over_Flag_co2=0;
UINT8		  send_end_flag_co2;			//-ֵΪ1 ��ʾ���ͽ����˵ȴ�Ӧ��;ֵΪ0 �Ѿ�Ӧ���ٵȴ�
UINT8     Received_pt_co2=0;

UINT32    cticks_s_CO2;
UINT16    co2_data;
UINT16    co2_data_old;
UINT16    co2_renew_wait_time;
UINT16    CO2_poll_wait_time;
UINT16    CO2_data_flag;		//-��ʾ����PM2.5�����ĵȼ�



//-by cheng pm2.5
char      ReadBuf_pm[30];
char 			Received_Over_Flag_pm;
UINT16 		Value_PM_Buffer[10];
UINT8 		PM_Counter=0; //-��¼�м�����Ч��ֵ��,����ƽ������
char      RcvStatus_pm;
UINT8     Received_pt_pm=0;

UINT16    pm_data;
UINT16    pm_data_old;
UINT16    pm_renew_wait_time;
UINT16    pm_data_flag;		//-��ʾ����PM2.5�����ĵȼ�


//-by cheng ��ʪ��
UINT8 		SHT3X_status;
UINT8 		SHT3X_err_flag;
UINT16 		temperature_data;
UINT8 		temperature_data_flag;  //-0��ʾ���� 1��ʾ����
UINT16 		temperature_data_x10;
UINT16 		humidity_data;
UINT16 		NTC_data;
UINT16 		temperature_data2;
UINT16 		humidity_data2;
UINT16 		temperature_data_out;
UINT32    temp_data_out_time;
UINT8     temp_data_out_flag;

//-������
//-char      interver;//pwmȫ�ֱ���
UINT8     rgb_num;				//-λ0 ����GREEN,λ1 ����RED,λ2 ����BLUE;��λ1��ʾ���� 0��ʾ���
UINT8		  ps_flag_pwm;		//-�Ӷ����Ƶ����	0 û�лӶ�;1 ���ϵ���;2 ���µ���,���ڴ洢��־,��PWMʹ��,������ҪӰ����ps_flag��־
UINT32    cticks_ms_pwm_loop;	//-������һ����,����һ�����ڵĳ���
UINT32    cticks_ms_pwm_R;
UINT32    cticks_ms_pwm_G;
UINT32    cticks_ms_pwm_B;
UINT32    cticks_ms_pwm_4;
UINT16    GREEN_pwmval;
UINT16    RED_pwmval;
UINT16    BLUE_pwmval;
UINT16    GREEN_pwmval_pt;		//-ƫ����
UINT16    RED_pwmval_pt;
UINT16    BLUE_pwmval_pt;
UINT16    white_pwmval_pt;
UINT16    GREEN_pwmval_num;		//-һ��ռ�ձ�ռ�е�ʱ��
UINT16    RED_pwmval_num;
UINT16    BLUE_pwmval_num;
UINT16    white_pwmval_num;
UINT8     HRL_RUN_ONOFF;	//-��ʾ�Ƿ񿪻���ָʾ��
UINT8     HRL_RUN_flag;		//-��������б�־ 0 �����п�����ʾ����״̬;0x55 ��������״̬;0xaa ��ʾ����״̬
UINT8     HRL_pt;		//-�����ָ��
UINT8     HRL_pt_start;
UINT8     HRL_pt_end;
UINT8     HRL_color_pt;		//-0 1 2 �ֱ����һ����ɫ
UINT16    HRL_pt_time00;
UINT16    HRL_pt_time01;
//-�����ڲʵ���ʱ
UINT8     HL_flag;		//-�ʵƱ�־,���ڱ�ʾ�ʵƵ�����ģʽ:0 �ر�״̬;1 �ʵ�״̬
UINT16    HL_run_time;		//-�ʵ�����ʱ���ʱ
UINT8     HL_new_value_flag;		//-0 ��ʾռ�ձ�û�б仯;1 �еƵ�ռ�ձȷ����˱仯
UINT16    HL_GREEN_pwmval;
UINT16    HL_RED_pwmval;
UINT16    HL_BLUE_pwmval;
UINT8     HL_step;		//-�����Ųʵ����еĲ���
UINT8     HL_ld_R[4];		//-��¼��ÿ����ɫ������ֵ(0~255)
UINT8     HL_ld_G[4];
UINT8     HL_ld_B[4];
UINT8     HL_ld_R_user[4];		//-�û���¼��ÿ����ɫ������ֵ(0~255)
UINT8     HL_ld_G_user[4];
UINT8     HL_ld_B_user[4];
UINT8     HL_ld_brightness;		//-�������ֵ

//-�����
UINT8     RunLed_PollNum;
UINT8     RunLed_stata_num;		//-��¼��Ŀǰ��������ʾҳ������
UINT8     RunLed_stata_flag;		//-ָʾ���ڴ�������⵽�Ļ�����

//-����
UINT16 		ADC_ConvertedValue[maxbuffer];
UINT16 		ADC_ConvertedValue_TEMP[maxbuffer];
UINT16	  samp_data_ram_base[ADC_Channel_num][maxbuffer/ADC_Channel_num];			//0x0000	0x600  ��ͨ��ϵ���Ľ����
UINT8 		Start_thansfer=0;
UINT8 		ADC_Conv_flag;
UINT16 		Noise_Value;
UINT16 		Noise_Value_flag;
UINT32 		ADC_Converte_cn;		//-ת���ļ�������

//-����
UINT8 Weather_flag;		//-0 ��ʾû����������;1�磬2����3���ƣ�4�꣬5ѩ

//-����
UINT8 		FAN_RUN_flag;		//-�������б�־ 0 ֹͣ,1 ����
UINT8 		FAN_RUN_wait_flag;
UINT16    FAN_RUN_wait_time;	//-���һ���������ȵ�ʱ��


//-ȫ��ʹ�ñ�־,��ʱ����
UINT8     sys_err_flag;		//-���ݴ����������,���һ��������ڲ�����
//-1 �����������߳���
//-2 �����ƴ���������
//-3 �����ƴ���������

//-����������,�洢������λ����Ϣ
UINT16    test_pt;
UINT16    test_cn[8];
UINT16    test_cn_wait_time;
UINT32    test_SIN_data[16];


//-ģ��EEPROM ʹ��FLASH
UINT32 EEP_Data;
UINT8  EEP_Data_flag;		//-0x55˵���������޸�,��Ҫд��FLASH



float float_data;

//-������ٶȼ�
UINT16    ADXL_X_data_flag;	//-0��ʾ����Ϊ����,1 ��ʾ����Ϊ����
UINT16    ADXL_Y_data_flag;
UINT16    ADXL_Z_data_flag;
UINT16    ADXL_X_data;		//-Ϊ����ֵ
UINT16    ADXL_Y_data;
UINT16    ADXL_Z_data;

UINT8 		ADXL_TAP_it_flag;			//-ֵΪ0 û���ж�;ֵΪ1 �����ж���
UINT8 		ADXL_TAP_it_SOURCE;		//-��¼�ж�Դֵ
UINT16		ADXL_TAP_wait_time;		//-���ʱ�����ڷ�����

//-�Լ��ʶλ
UINT8 		STM32_UP_selfT_flag1;		//-ÿһλ����һ�������ĺû�:0 ����,1 �쳣.λ0 ������ٶ�
UINT8 		STM32_UP_error_flag1;		//-˵��������������Ҫ�����ϵ���߸�������
UINT8 		STM32_UP_selfT_cn1;
UINT8 		STM32_UP_selfT_cn2;
UINT8 		STM32_UP_selfT_cn3;
UINT8 		STM32_shedeng_selfT_flag1;		//-��ư��Լ�״̬

//-ͨѶ��ʼ
UINT8 		UART0_transmit_flag;
UINT16	  UART0_start_tx_time;

//-����1
UINT8 		UART1_transmit_flag;		//-ֵΪYES��ʾ���Է��� ֵΪNO��ʾ�����Է���
UINT8 		UART1_transmit_control;	//-��ͬ��ֵ����ͬ�ķ�������

UINT8 		UART1_renew_flag;			//-0 ��ʾ����ͨ�����ڸ������ݿ�����,0x55 ��ʾ������ͨ�����ڸ�������
UINT16 		UART1_renew_wait_time;

UINT8 		UART1_led_disdata00;
UINT8 		UART1_led_disdata01;
UINT8 		UART1_led_disdata02;

UINT8 		UART1_sloop_flag;
UINT16 		UART1_sloop_wait_time;

//-��������
UINT8 		voice_flag;				//-0 ��ʾ������;0x55 ����


//-zigbee������ʶ
UINT8 		zigbee_flag;
UINT16		zigbee_wait_time;		//-���������ʱ����ʱ


//-VOC
UINT16 		VOC_data;
UINT16    VOC_data_old;
UINT16    VOC_renew_wait_time;
UINT16    VOC_data_flag;
UINT16    voc_rd_flag;



//-����ES705ȫ�ֱ�������
__IO uint32_t irq_flag = 0;
__IO uint32_t f_rec  = 0;
__IO uint8_t	rx1_cnt = 0;
uint8_t uart1_rx_buff[100];
  //-��������ȫ�ֱ�־λ,Ϊ�˺ϲ�����
UINT8 		es705_mode;     //-��������оƬ������ģʽ
UINT8 		es705_training_status;   //-ѧϰģʽʱ������״̬
UINT8 		es705_training_count;    //-ѧϰģʽʱ�ɹ�ѧϰ�Ĵ���
UINT8 		es705_TO_UART2_FLAG;     //-�����־λ������STM32ͨ������2������7620���͵�����
UINT16 		es705_event_type;        //-�������ѵ�����

int       es705_mode_file;
UINT8 		es705_training_flag;  //-˵��es705�����ĸ�״̬
UINT16    es705_training_wait_time;   //-һ���������ѧϰ״̬,���5Min
UINT16    es705_msleep_time;

//-iap����
UINT16    IAP_ack_hang;
UINT16    IAP_ack_version;

//-������������2ͨѶ
UINT16    Touch_wait_time;
UINT8 		Touch_Count2;
UINT8 		Touch_Count2_pt;

//-�͹���STOPģʽ����
UINT8 		STOP_status;    //-��ʾĿǰ�����ĸ�״̬
UINT16    STOP_wait_time;
UINT8 		STOP_status2;
UINT16    STOP_wait_time2;

//-������
UINT8 		beep_status;    //-��ʾ������״̬
UINT16    beep_wait_time;
UINT8 		beep_onoff;
UINT8 		Motor_onoff;

//-ϵͳ����
UINT8 		SYS_WORK_MODE;		//-����ģʽ
UINT8 		SYS_LOCK_FLAG;
UINT8 		SYS_power_FLAG;   //-ϵͳ��Դ��־
UINT32    SYS_power_wait_time;

//-�˵�״̬
UINT8 		Standby_status;    //-����״̬��¼
UINT8 		RUN_ONOFF_status; //-���ػ����
UINT8 		RUN_status;    //-��¼����״̬
UINT8 		RUN_status_flag;    //-��ʾ�Ƿ��л��˹���ģʽ 0x55�л���
UINT16    RUN_status_wait_time;   //-���л�����ģʽ��ʱ��,��ʱһ��ʱ����Ч,��ֹ�����л�
UINT8 		menu_key_status;  //-��¼��Ӧ�˵����õİ���ֵ ���� �� �˵� �� �ػ�/����
UINT8 		menu_wind_status;			//-��ʾ���ȵ��������,λ7��ʾ�Ƿ�ֵ�и���
UINT8 		menu_set_tt_flag;    //-ֵΪ0˵��û���޸�,ֵΪ1˵�������޸���
UINT16 		menu_set_tt_volue;   //-��ǰ��ʾ����Чֵ
UINT16 		menu_set_tt_max;    //-�˵����õ������¶�ֵ,,ʵ��ֵ�����ֵ�������Ҫ������
UINT16 		menu_set_tt_min;    //-�˵����õ������¶�ֵ,,ʵ��ֵ����Сֵ��С����Ҫ������
UINT16 		menu_set_tt_max_eep;    //-�˵����õ������¶�ֵ
UINT16 		menu_set_tt_min_eep;    //-�˵����õ������¶�ֵ
UINT16 		menu_set_tt_max_f;    //-�˵����õ������¶�ֵ
UINT16 		menu_set_tt_min_f;    //-�˵����õ������¶�ֵ
UINT16 		auto_set_tt_max_eep;    //-�˵����õ������¶�ֵ
UINT16 		auto_set_tt_min_eep;    //-�˵����õ������¶�ֵ
UINT8 		menu_set_tt_more;    //-�˵����õ��¶�ֵ
UINT8 		menu_set_test_flag;   //-��ʾ�Ƿ�������ģʽ
UINT16 		menu_set_tt_simu;
UINT8 		menu_set_tt_CorF;
UINT8 		SET_Modify_flag;

UINT8 		menu_set_tt_swing;    //-�˵����õ�һ��ҡ������ֵ
UINT8 		menu_set_tt_swing_pt;
UINT8 		menu_set_tt_diff;    //-�˵����õĶ���ҡ������ֵ
UINT8 		menu_set_tt_diff_pt;
UINT8 		menu_set_tt_third;
UINT8 		menu_set_tt_third_pt;
UINT8 		menu_set_tt_cal;    //-�¶�У׼


//-�����ж�
UINT8 		E_out_flag;   //-�������ȱ�־
UINT8 		E_start_flag; //-���ڼ�¼���������Ƿ�����
UINT8 		out_flag;   //-��ʾ�����Ƿ���Ҫ����
UINT16 		out_status; //-��ʾ����Ŀǰ�����ֵ
UINT16 		out_status_new; //-��ʾ���������ݵȴ�����
UINT8 		out_hold_flag;
UINT8 		 out_onoff_flag;
UINT16    out_onoff_wait_time;

UINT32    OUT_HEAT_one_time;
UINT32    OUT_HEAT_two_time;
UINT32    OUT_COOL_one_time;
UINT32    OUT_COOL_two_time;
UINT8    OUT_HEAT_one_flag;   //-�κ�һ��ʱ���׼��������Ч�Ե�
UINT8    OUT_HEAT_two_flag;
UINT8    OUT_COOL_one_flag;
UINT8    OUT_COOL_two_flag;

UINT8    OUT_W1orAUX_flag;
UINT32    OUT_W1orAUX_time;
UINT8    OUT_Y1_flag;
UINT32    OUT_Y1_time;
UINT8    OUT_Y2_flag;
UINT32    OUT_Y2_time;

PROGRAMME_DAY_CH  programme_day[3*7];   //-ÿ��3�ű�,ÿ��7��,�ܹ�21�ű�,��������,����,�Զ�����ģʽ
UINT8     now_programme_flag;    //-0 ��ʾ��Ϊ�͵ز���ʧЧ 0x55 ��Ϊ��Ч��,���ʧЧ
UINT16    now_programme_time;   //-��¼���һ���޸ĵ�ʱ��
UINT8     now_programme_redata;

UINT8     OUT_emergency_hot;


UINT32    SET_send_one_time;
UINT8    SET_send_one_flag;
UINT32    SET_send_two_time;
UINT8    SET_send_two_flag;






