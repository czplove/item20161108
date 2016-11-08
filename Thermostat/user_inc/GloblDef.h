


#ifndef __GloblDef_H__
#define __GloblDef_H__

extern WORD    port_recv_pt[4];
extern WORD    port_recv_dl[4];
extern WORD    port_send_pt[4];
extern WORD    port_send_dl[4];
extern WORD    port_send_len[4];

extern BYTE	port_recv[4][256];
extern BYTE	port_send[4][256];
extern BYTE	UART3_port_send[256];
extern BYTE	port_deal_buf[4][256];
extern BYTE	port_report[256];
extern BYTE	port_deal_flag[4];
extern BYTE	UART3_TO_UART2_FLAG;

extern UL		samp_data_ch0_average;
extern UL		samp_data_ch1_average;
extern UL		samp_data_ch2_average;
extern UL		samp_data_ch3_average;
extern UL		samp_data_ch4_average;



/*extern struct L6_Ram
{								//start		len
	int16  samp_data_ram_base[4][128];			//0x0000	0x600  ��ͨ��ϵ���Ľ����
	int16  samp_data_cf_ram_base[4][128];			//0x0600	0x600
	Uint16 samp_data_power_ram_base[128];			//0x0c00	0x080
	int16  samp_data_yc_ram_base[13][128];			//0x0c80	0x240  end 0x0fef
}L6Ram;
*/

extern BYTE	samp_data_pt;

extern WORD    port_send_sense_data[20];

/*extern struct  NET_PORT_VAR{
Uint16	net_port_recv_pt[UIP_CONNS];
Uint16   net_port_recv_dl[UIP_CONNS];
UINT8    net_port_recv[UIP_CONNS][2048];
Uint16	net_port_send_pt[UIP_CONNS];
Uint16   net_port_send_len[UIP_CONNS];
UINT8    net_port_send[UIP_CONNS][2048];
UINT8    net_port_deal_buf[UIP_CONNS][512];
		    }net_port_var;*/

//////////////////////////////////////////////////////////

#define NU_SUCCESS				0

//#define one_net
#define syn_clock		/*--IRIG-B's time from rs485 and net's date*/
//#define ex_syn_all 	/*only from IRIG-B */
//#define timeover_syn_e/*if timeover for syn_clock from net then auto sent clock to can*/

#define SERIAL_BUS_IDLE_TIME  4

/* Device Stste Definition */
extern UINT32 DeviceState;

#define indication_syn_clock_e     0x00000001   // Synchronize Clock Enable
#define indication_syn_clock_err   0x00000002   // Synchronize Clock Error
#define indication_net_run         0x00000004   // Net is running
#define indication_change_ip       0x00000008
#define indication_net_timeover    0x00000010
#define indication_syn_clock_ok    0x00000020	// Synchronized by IRIG-B
#define indication_sys_need_send   0x00000040
#define indication_version         0x00000080
#define indication_mon_serial0_rt  0x00000100	// Monitor UART0
#define indication_mon_serial1_rt  0x00000200	// Monitor UART1
#define indication_mon_tcp0_rt     0x00000400	// Monitor TCP0
#define indication_mon_tcp1_rt     0x00000800	// Monitor TCP1
#define indication_mon_232_rt      0x00001000	// Monitor 232
#define indication_stop_mon        0x00002000	// Stop Monitor



extern UINT32 Device_communication_cn;

/*  */
extern BYTE  serial_bus_recv_state;

/* System Date&Time */
extern  UINT16   m_year;
extern  BYTE     m_month;
extern  BYTE     m_week;
extern  BYTE     m_date;
extern  BYTE     m_hour;
extern  BYTE     m_min;
extern  BYTE     m_sec;
extern  UINT16   m_msec;

extern  UINT16  REG_year;
extern  UINT8  REG_month;
extern  UINT8  REG_week;
extern  UINT8  REG_date;
extern  UINT8  REG_hour;
extern  UINT8  REG_min;
extern  UINT8  REG_sec;
extern  UINT16 REG_msec;

/* User Monitor Buffer */
//-#define MAX_MON_BUF_SIZE  			1024		// Max Monitor Buffer Size
extern UINT8  monitor_buffer[MAX_MON_BUF_SIZE];
extern UINT16 monitor_in_pos,monitor_out_pos,monitor_out_pos_t;

/* UART TX/RX Buffer */
//-extern UART_BUFFER mUART_TX_BUFFER[NO_UART_CPU];
//extern UART_BUFFER mUART_RX_BUFFER[NO_UART_CPU];

/* UART Device Entry&Entry Pointer */
//extern UART_DEVICE_ENTRY  uart_device_task[NO_UART_CPU];
//extern UART_DEVICE_ENTRY  *fp_uart_device[NO_UART_CPU];

/* UART Channel x Initialize Function Pointer Array */
//extern int (*fp_uart_device_init[NO_UART_CPU])(UART_DEVICE_ENTRY *);

/* */
extern BYTE  serial_idle_counter[3];

/* 103 Protocol Device Self Address */
extern UINT16 net_u103_self_addr;

/* Index Protocol Type for TCP or UDP */
extern UINT8  protocal_type;



extern  UINT16  cticks_ms;    //-ʱ���׼������
extern  UINT16  Time_2048ms_Counter;
extern  UINT16  cticks_5ms;    //-ʱ���׼������
extern  UINT16  cticks_500ms;
extern  UINT32  cticks_s;
extern  UINT16  cticks_ms_pwm;
extern  UINT16  cticks_test;

extern  UINT16  cticks_SLEEP_serial;
extern  UINT32  TEST_Sensor_wait_time;
extern  UINT16  Sensor_data_wait_time;

#define SYS_TIME_OUT_NUM   0xfffffff0



//common function
extern void memcopy_to_udpbuf(BYTE *src,short len);


//-�������ƴ�����ʹ�õ���ʱȫ�ֱ���,������
extern UINT16	  i2c1_alsdata_16;
extern UINT16  	i2c1_psdata_16;
extern UINT8		i2c1_newdata_flag;		//-0 ��ʾû��������;1 ��ʾ��������
extern UINT8		i2c1_psinit_flag;		  //-0 ��ʾ��ʼ���ɹ�;1 ��ʾʧ��
extern UINT8		i2c1_psint_cn;		    //-��¼�жϳ���ʱ��
extern UINT8		i2c1_psint_flag;			//-0 ��ʾ�ж��ź���Ч;1 ��ʾ�ж��źŴ���,��ʼ�����жϺ���1,ֱ��������Ϊֹ
extern UINT8		i2c1_psint_flag_FX;
extern UINT8		i2c1_bus_error_cn;

extern UINT16	  i2c2_alsdata_16;
extern UINT16  	i2c2_psdata_16;
extern UINT8		i2c2_newdata_flag;	  //-0 ��ʾû��������;1 ��ʾ��������
extern UINT8		i2c2_psinit_flag;		  //-0 ��ʾ��ʼ���ɹ�;1 ��ʾʧ��
extern UINT8		i2c2_psint_cn;		    //-��¼�жϳ���ʱ��
extern UINT8		i2c2_psint_flag;			//-0 ��ʾ�ж��ź���Ч;1 ��ʾ�ж��źŴ���,��ʼ�����жϺ���1,ֱ��������Ϊֹ
extern UINT8		i2c2_psint_flag_FX;
extern UINT8		i2c2_bus_error_cn;
extern UINT8		i2c3_bus_error_cn;
extern UINT8		i2c4_bus_error_cn;
extern UINT8		i2c5_bus_error_cn;

extern UINT16  i2c3_psdata_16;
extern UINT16  i2c4_psdata_16;
extern UINT16  als_data;
extern UINT16  als_data2;
extern UINT16  als_data3;
extern UINT16  als_data4;
extern UINT8		ps_flag;
extern UINT32		ps1_int_time;
extern UINT32		ps2_int_time;
extern UINT16		VCNL_poll_wait_time;
//-ADI������
extern UINT16  ADI_CH1_OFFSET;
extern UINT16  ADI_CH2_OFFSET;
extern UINT16  ADI_CH3_OFFSET;
extern UINT16  ADI_CH4_OFFSET;
extern UINT16  ADI_DATA_BUFFER_OUT;
extern UINT16  ADI_DSAMPLE_TIME;
extern UINT16  ADI_DEC_MODE;
extern UINT16  ADI_INT_STATUS;
extern UINT32 	ADI_X1;
extern UINT32 	ADI_X2;
extern UINT32 	ADI_Y1;
extern UINT32 	ADI_Y2;
extern UINT16  ADI_PS_Value1;

//-��������ʾ��ʱ����
extern UINT8		led_display_flag;			//-�л���ʾҳ
extern UINT8		led_display_y;
extern UINT8		led_display_cn;        //-��ʾ�����жϵĴ���
extern UINT8		led_display_cn_f;
extern UINT8		led_display_new;			//-��ʾ�Ƿ�����������Ҫ׼��0x55 ��ʾ����׼��������	0xaa ��ʾ����׼������
extern UINT16  	led_display_data;			//-һҳ�����ʾ��λ����,���������999
extern UINT8		led_display_num;				//-����������������ʾ
extern UINT8		led_display_txpic_flag;				//-0x55 ��ʾ��ʾͨѶ�·���ͼƬ
extern UINT8		led_display_txpic_num;
extern UINT16	  cartoon_end_wait_time;
extern UINT32		led_display_data1[16];	//-������Ҫ��ʾ���ݵ�����
extern UINT32		led_display_data2[16];	//-������Ҫ��ʾ���ݵ�����
extern UINT32		led_display_data3[16];	//-������Ҫ��ʾ���ݵ�����
extern UINT32		led_display_data4[16];	//-������Ҫ��ʾ���ݵ�����
extern UINT32		led_display_data5[16];	//-������Ҫ��ʾ���ݵ�����
extern UINT32		led_display_data6[16];
extern UINT32	  led_display_data_temp[16];
extern UINT8		led_data_x;
extern UINT8		led_data_y;
extern UINT32		led_display_data_round[32];
extern UINT32		led_display_data_round1[32];
extern UINT32 	led_display_blink0[16];
extern UINT32 	led_display_blink1[16];


extern UINT8		led_display_page;
extern UINT8		led_display_page_end;
extern UINT8		led_display_start;
extern UINT8    cticks_s_page;
extern UINT8		ps_flag_led;
extern UINT8		ps_flag_led_end;
extern UINT8		led_display_long;
extern UINT16	  led_display_end_time;
extern UINT8    led_display_move_pt;
extern UINT8		led_display_data_flag;

//-�ֿ����鶨�峣��
extern UINT8 ZIKU[][16];
extern UINT8 ZIKU_CO2[][3];
extern UINT8 ZIKU_PM25[][3];
extern UINT8 ZIKU_TEMP[][3];
extern UINT8 ZIKU_humidity[][3];
extern UINT8 ZIKU_TIME[][3];
extern UINT8 ZIKU_WIFI[][3];
extern UINT8 ZIKU_cloudy[][3];
extern UINT8 ZIKU_sunshine[][3];
extern UINT8 ZIKU_lightning[][3];
extern UINT8 ZIKU_rain[][3];
extern UINT8 ZIKU_WIFIOFF[][3];
extern UINT8 ZIKU_DROOM[][3];
extern UINT8 ZIKU_HOOMIN[][3];
extern UINT8 ZIKU_HOMEOUT[][3];
extern UINT8 ZIKU_yawp[][3];
extern UINT8 ZIKU_laugh[][3];
extern UINT8 ZIKU_cry[][3];
extern UINT8 ZIKU_user[][3];
extern UINT8 ZIKU_FF[][3];

extern UINT8 ZIKU_SIN[][3];
extern UINT8 ZIKU_XIAO[][7];

extern UINT8	temp_data01_8;
extern UINT8	temp_data02_8;
extern UINT8	temp_data03_8;
extern UINT8	temp_data04_8;

extern UINT16	temp_data01;
extern UINT16	temp_data02;
extern UINT16	temp_data03;
extern UINT16	temp_data04;

extern UINT16		watch_data[512];
extern UINT16		watch_data_pt;

extern UINT8		watch_cfgdata;
extern UINT16		watch_cfgdata_16;


//-��������
/*
BIN5		BIN4		BIN3		BIN2		BIN1		GUARD
CS3			CS1			CS0			CS4			CS5			CS2
D5      D4      D0      D1      D3			����
*/
#define KEY_BIN1         0x20
#define KEY_BIN2         0x10
#define KEY_BIN3         0x01
#define KEY_BIN4         0x02
#define KEY_BIN5         0x08
#define KEY_GUARD        0x04
#define KEY_MASK         0x3F

extern UINT8		CSKEY_cfgDATA;
extern UINT8		CSKEY_DATA;

//-by cheng CO2
extern UINT16 	 Value_CO2_Buffer[10];
extern UINT8		 CO2_Counter;
extern char      ReadBuf_CO2[30];
extern int       Received_Over_time_co2,Received_Over_Flag_co2;
extern UINT8	   send_end_flag_co2;			//-ֵΪ1 ��ʾ���ͽ����˵ȴ�Ӧ��;ֵΪ0 �Ѿ�Ӧ���ٵȴ�
extern UINT8     Received_pt_co2;
extern UINT32    cticks_s_CO2;
extern UINT16    co2_data;
extern UINT16    co2_data_old;
extern UINT16    co2_renew_wait_time;
extern UINT16    CO2_poll_wait_time;
extern UINT16    CO2_data_flag;


//-by cheng pm2.5
extern char    		ReadBuf_pm[30];
extern char 	 		Received_Over_Flag_pm;
extern UINT16 		Value_PM_Buffer[10];
extern UINT8 			PM_Counter;
extern char    		RcvStatus_pm;
extern UINT8     	Received_pt_pm;
extern UINT16 		pm_data;
extern UINT16     pm_data_old;
extern UINT16     pm_renew_wait_time;
extern UINT16     pm_data_flag;

//-by cheng ��ʪ��
extern UINT8 		  SHT3X_status;
extern UINT8 		  SHT3X_err_flag;
extern UINT16 		temperature_data;
extern UINT8 		  temperature_data_flag;
extern UINT16 		temperature_data_x10;
extern UINT16 		humidity_data;
extern UINT16 		NTC_data;
extern UINT16 		temperature_data2;
extern UINT16 		humidity_data2;
extern UINT16 		temperature_data_out;
extern UINT32    temp_data_out_time;
extern UINT8     temp_data_out_flag;

//-by cheng PWM
extern UINT8    rgb_num;
extern UINT8		ps_flag_pwm;
extern UINT32   cticks_ms_pwm_loop;
extern UINT32  cticks_ms_pwm_R;
extern UINT32  cticks_ms_pwm_G;
extern UINT32  cticks_ms_pwm_B;
extern UINT32  cticks_ms_pwm_4;
extern UINT16    GREEN_pwmval;
extern UINT16    RED_pwmval;
extern UINT16    BLUE_pwmval;
extern UINT16    GREEN_pwmval_pt;
extern UINT16    RED_pwmval_pt;
extern UINT16    BLUE_pwmval_pt;
extern UINT16    white_pwmval_pt;
extern UINT16    GREEN_pwmval_num;		//-һ��ռ�ձ�ռ�е�ʱ��
extern UINT16    RED_pwmval_num;
extern UINT16    BLUE_pwmval_num;
extern UINT16    white_pwmval_num;
extern UINT8     HRL_RUN_ONOFF;
extern UINT8     HRL_RUN_flag;
extern UINT8     HRL_pt;
extern UINT8     HRL_pt_start;
extern UINT8     HRL_pt_end;
extern UINT8     HRL_color_pt;
extern UINT16    HRL_pt_time00;
extern UINT16    HRL_pt_time01;
#define PWM_Period_Value        4000	//-�����¼�����Чֵ
#define PWM_step_cn             (375*2)  	//-�������ٲ�����С��Ϊ���
//-�����ڲʵ���ʱ
extern UINT8     HL_flag;		//-�ʵƱ�־,���ڱ�ʾ�ʵƵ�����ģʽ:0 ����״̬;1 �ʵ�״̬
extern UINT16    HL_run_time;
extern UINT8     HL_new_value_flag;		//-0 ��ʾռ�ձ�û�б仯;1 �еƵ�ռ�ձȷ����˱仯
extern UINT16    HL_GREEN_pwmval;
extern UINT16    HL_RED_pwmval;
extern UINT16    HL_BLUE_pwmval;
extern UINT8     HL_step;
extern UINT8     HL_ld_R[4];		//-��¼��ÿ����ɫ������ֵ(0~255)
extern UINT8     HL_ld_G[4];
extern UINT8     HL_ld_B[4];
extern UINT8     HL_ld_R_user[4];		//-�û���¼��ÿ����ɫ������ֵ(0~255)
extern UINT8     HL_ld_G_user[4];
extern UINT8     HL_ld_B_user[4];
extern UINT8     HL_ld_brightness;


//-�����
extern UINT8     RunLed_PollNum;
extern UINT8     RunLed_stata_num;
extern UINT8     RunLed_stata_flag;

//-����
#define maxbuffer 		1026
#define maxcounter 		1024
#define base 					1723
#define ADC_Channel_num 					3			//-ϵͳ��ADC�Ĳ���Ƶ����
extern UINT16 	ADC_ConvertedValue[maxbuffer];
extern UINT16 	ADC_ConvertedValue_TEMP[maxbuffer];
extern UINT16	  samp_data_ram_base[ADC_Channel_num][maxbuffer/ADC_Channel_num];
extern UINT8 		Start_thansfer;
extern UINT8 		ADC_Conv_flag;
extern UINT16 	Noise_Value;
extern UINT16 	Noise_Value_flag;
extern UINT32 	ADC_Converte_cn;

//-����
extern UINT8 Weather_flag;

//-����
extern UINT8 		 FAN_RUN_flag;		//-�������б�־ 0 ֹͣ,1 ����
extern UINT8 		 FAN_RUN_wait_flag;
extern UINT16    FAN_RUN_wait_time;	//-���һ���������ȵ�ʱ��

//-ȫ��ʹ�ñ�־,��ʱ����
extern UINT8     sys_err_flag;

//-����������,�洢������λ����Ϣ
extern UINT16    test_pt;
extern UINT16    test_cn[8];
extern UINT16    test_cn_wait_time;
extern UINT32    test_SIN_data[16];

//-ģ��EEPROM ʹ��FLASH
extern UINT32 EEP_Data;
extern UINT8  EEP_Data_flag;

extern float float_data;

//-������ٶȼ�
extern UINT16    ADXL_X_data_flag;	//-0��ʾ����Ϊ����,1 ��ʾ����Ϊ����
extern UINT16    ADXL_Y_data_flag;
extern UINT16    ADXL_Z_data_flag;
extern UINT16    ADXL_X_data;		//-Ϊ����ֵ
extern UINT16    ADXL_Y_data;
extern UINT16    ADXL_Z_data;

extern UINT8 		 ADXL_TAP_it_flag;
extern UINT8 		 ADXL_TAP_it_SOURCE;
extern UINT16		 ADXL_TAP_wait_time;

//-�Լ��ʶλ
extern UINT8 		STM32_UP_selfT_flag1;
extern UINT8 		STM32_UP_error_flag1;
extern UINT8 		STM32_UP_selfT_cn1;
extern UINT8 		STM32_UP_selfT_cn2;
extern UINT8 		STM32_UP_selfT_cn3;
extern UINT8 		STM32_shedeng_selfT_flag1;

//-ͨѶ��ʼ
extern UINT8 		UART0_transmit_flag;
extern UINT16	  UART0_start_tx_time;

//-����1
extern UINT8 		UART1_transmit_flag;		//-ֵΪYES��ʾ���Է��� ֵΪNO��ʾ�����Է���
extern UINT8 		UART1_transmit_control;	//-��ͬ��ֵ����ͬ�ķ�������
extern UINT8 		UART1_renew_flag;			//-0 ��ʾ����ͨ�����ڸ������ݿ�����,0x55 ��ʾ������ͨ�����ڸ�������
extern UINT16 	UART1_renew_wait_time;

extern UINT8 		UART1_led_disdata00;
extern UINT8 		UART1_led_disdata01;
extern UINT8 		UART1_led_disdata02;

extern UINT8 		UART1_sloop_flag;
extern UINT16 	UART1_sloop_wait_time;

//-��������
extern UINT8 		voice_flag;

//-zigbee������ʶ
extern UINT8 		zigbee_flag;
extern UINT16		zigbee_wait_time;

//-VOC
extern UINT16 	 VOC_data;
extern UINT16    VOC_data_old;
extern UINT16    VOC_renew_wait_time;
extern UINT16    VOC_data_flag;
extern UINT16    voc_rd_flag;


//-����ES705ȫ�ֱ�������
extern __IO uint32_t f_rec;
extern __IO uint32_t irq_flag;

//-��������ȫ�ֱ�־λ,Ϊ�˺ϲ�����
extern UINT8 		es705_mode;
extern UINT8 		es705_training_status;   //-ѧϰģʽʱ������״̬
extern UINT8 		es705_training_count;   //-ѧϰģʽʱ�ɹ�ѧϰ�Ĵ���
extern UINT8 		es705_TO_UART2_FLAG;
extern UINT16		es705_event_type;

extern int     es705_mode_file;
extern UINT8 		es705_training_flag;  //-˵��es705�����ĸ�״̬
extern UINT16    es705_training_wait_time;
extern UINT16    es705_msleep_time;

//-iap����
extern UINT16    IAP_ack_hang;
extern UINT16    IAP_ack_version;

//-������������2ͨѶ
extern UINT16    Touch_wait_time;
extern UINT8 		 Touch_Count2;
extern UINT8 		 Touch_Count2_pt;

//-�͹���STOPģʽ����
extern UINT8 		 STOP_status;    //-��ʾĿǰ�����ĸ�״̬
extern UINT16    STOP_wait_time;
extern UINT8 		 STOP_status2;
extern UINT16    STOP_wait_time2;

//-������
extern UINT8 		beep_status;    //-��ʾ������״̬
extern UINT16    beep_wait_time;
extern UINT8 		beep_onoff;
extern UINT8 		Motor_onoff;

//-ϵͳ����
extern UINT8 		SYS_WORK_MODE;		//-����ģʽ
extern UINT8 		SYS_LOCK_FLAG;
extern UINT8 		SYS_power_FLAG;
extern UINT32   SYS_power_wait_time;


//-�˵�״̬
extern UINT8 		Standby_status;    //-��¼�˵�״̬
extern UINT8 		RUN_ONOFF_status;
extern UINT8 		RUN_status;
extern UINT8 		RUN_status_flag;
extern UINT16    RUN_status_wait_time;
extern UINT8 		menu_key_status;  //-��¼��Ӧ�˵����õİ���ֵ
extern UINT8 		menu_wind_status;
extern UINT8 		menu_set_tt_flag;
extern UINT16 		menu_set_tt_volue;
extern UINT16 		menu_set_tt_max;    //-�˵����õ������¶�ֵ
extern UINT16 		menu_set_tt_min;    //-�˵����õ������¶�ֵ
extern UINT16 		menu_set_tt_max_eep;    //-�˵����õ������¶�ֵ,����ֵ������100��
extern UINT16 		menu_set_tt_min_eep;    //-�˵����õ������¶�ֵ
extern UINT16 		menu_set_tt_max_f;    //-�˵����õ������¶�ֵ
extern UINT16 		menu_set_tt_min_f;    //-�˵����õ������¶�ֵ
extern UINT16 		auto_set_tt_max_eep;    //-�˵����õ������¶�ֵ
extern UINT16 		auto_set_tt_min_eep;    //-�˵����õ������¶�ֵ
extern UINT8 		menu_set_tt_more;    //-�˵����õ����ر仯ֵ
extern UINT8 		menu_set_test_flag;
extern UINT16 		menu_set_tt_simu;     //-Ϊ�˵���ģ���ʵ���¶�,���Զ����߼�
extern UINT8 		menu_set_tt_CorF;
extern UINT8 		SET_Modify_flag;

extern UINT8 		menu_set_tt_swing;    //-�˵����õ�һ��ҡ������ֵ
extern UINT8 		menu_set_tt_swing_pt;
extern UINT8 		menu_set_tt_diff;    //-�˵����õĶ���ҡ������ֵ
extern UINT8 		menu_set_tt_diff_pt;
extern UINT8 		menu_set_tt_third;
extern UINT8 		menu_set_tt_third_pt;
extern UINT8 		menu_set_tt_cal;

//-�����ж�
extern UINT8 		E_out_flag;
extern UINT8 		E_start_flag;
extern UINT8 		out_flag;   //-��ʾ�����Ƿ���Ҫ����
extern UINT16 		out_status; //-��ʾ����Ŀǰ�����ֵ
extern UINT16 		out_status_new;
extern UINT8 		out_hold_flag;
extern UINT8 		 out_onoff_flag;
extern UINT16    out_onoff_wait_time;

extern UINT32    OUT_HEAT_one_time;
extern UINT32    OUT_HEAT_two_time;
extern UINT32    OUT_COOL_one_time;
extern UINT32    OUT_COOL_two_time;
extern UINT8    OUT_HEAT_one_flag;   //-�κ�һ��ʱ���׼��������Ч�Ե�
extern UINT8    OUT_HEAT_two_flag;
extern UINT8    OUT_COOL_one_flag;
extern UINT8    OUT_COOL_two_flag;

extern UINT8    OUT_W1orAUX_flag;
extern UINT32    OUT_W1orAUX_time;
extern UINT8    OUT_Y1_flag;
extern UINT32    OUT_Y1_time;
extern UINT8    OUT_Y2_flag;
extern UINT32    OUT_Y2_time;


extern PROGRAMME_DAY_CH  programme_day[3*7];
extern UINT8     now_programme_flag;    //-0 ��ʾ��Ϊ�͵ز���ʧЧ 0x55 ��Ϊ��Ч��,���ʧЧ
extern UINT16    now_programme_time;
extern UINT8     now_programme_redata;

extern UINT8     OUT_emergency_hot;

extern UINT32    SET_send_one_time;
extern UINT8    SET_send_one_flag;
extern UINT32    SET_send_two_time;
extern UINT8    SET_send_two_flag;


#endif // __GloblDef_H__


