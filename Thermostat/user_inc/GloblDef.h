


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
	int16  samp_data_ram_base[4][128];			//0x0000	0x600  乘通道系数的结果。
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



extern  UINT16  cticks_ms;    //-时间基准计数器
extern  UINT16  Time_2048ms_Counter;
extern  UINT16  cticks_5ms;    //-时间基准计数器
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


//-光照手势传感器使用的临时全局变量,测试用
extern UINT16	  i2c1_alsdata_16;
extern UINT16  	i2c1_psdata_16;
extern UINT8		i2c1_newdata_flag;		//-0 表示没有新数据;1 表示有新数据
extern UINT8		i2c1_psinit_flag;		  //-0 表示初始化成功;1 表示失败
extern UINT8		i2c1_psint_cn;		    //-记录中断持续时间
extern UINT8		i2c1_psint_flag;			//-0 表示中断信号无效;1 表示中断信号存在,开始进入中断后置1,直到软件清除为止
extern UINT8		i2c1_psint_flag_FX;
extern UINT8		i2c1_bus_error_cn;

extern UINT16	  i2c2_alsdata_16;
extern UINT16  	i2c2_psdata_16;
extern UINT8		i2c2_newdata_flag;	  //-0 表示没有新数据;1 表示有新数据
extern UINT8		i2c2_psinit_flag;		  //-0 表示初始化成功;1 表示失败
extern UINT8		i2c2_psint_cn;		    //-记录中断持续时间
extern UINT8		i2c2_psint_flag;			//-0 表示中断信号无效;1 表示中断信号存在,开始进入中断后置1,直到软件清除为止
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
//-ADI新手势
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

//-点阵屏显示临时变量
extern UINT8		led_display_flag;			//-切换显示页
extern UINT8		led_display_y;
extern UINT8		led_display_cn;        //-显示进入中断的次数
extern UINT8		led_display_cn_f;
extern UINT8		led_display_new;			//-显示是否有新数据需要准备0x55 表示现在准备新数据	0xaa 表示数据准备好了
extern UINT16  	led_display_data;			//-一页最多显示三位数据,所以最大是999
extern UINT8		led_display_num;				//-两个缓冲区轮流显示
extern UINT8		led_display_txpic_flag;				//-0x55 表示显示通讯下发的图片
extern UINT8		led_display_txpic_num;
extern UINT16	  cartoon_end_wait_time;
extern UINT32		led_display_data1[16];	//-保存需要显示数据的内容
extern UINT32		led_display_data2[16];	//-保存需要显示数据的内容
extern UINT32		led_display_data3[16];	//-保存需要显示数据的内容
extern UINT32		led_display_data4[16];	//-保存需要显示数据的内容
extern UINT32		led_display_data5[16];	//-保存需要显示数据的内容
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

//-字库数组定义常数
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


//-触摸按键
/*
BIN5		BIN4		BIN3		BIN2		BIN1		GUARD
CS3			CS1			CS0			CS4			CS5			CS2
D5      D4      D0      D1      D3			保护
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
extern UINT8	   send_end_flag_co2;			//-值为1 表示发送结束了等待应答;值为0 已经应答不再等待
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

//-by cheng 温湿度
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
extern UINT16    GREEN_pwmval_num;		//-一个占空比占有的时长
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
#define PWM_Period_Value        4000	//-更新事件的有效值
#define PWM_step_cn             (375*2)  	//-经过多少步由最小变为最大
//-运用在彩灯上时
extern UINT8     HL_flag;		//-彩灯标志,用于表示彩灯的运行模式:0 呼吸状态;1 彩灯状态
extern UINT16    HL_run_time;
extern UINT8     HL_new_value_flag;		//-0 表示占空比没有变化;1 有灯的占空比发生了变化
extern UINT16    HL_GREEN_pwmval;
extern UINT16    HL_RED_pwmval;
extern UINT16    HL_BLUE_pwmval;
extern UINT8     HL_step;
extern UINT8     HL_ld_R[4];		//-记录了每种颜色的亮度值(0~255)
extern UINT8     HL_ld_G[4];
extern UINT8     HL_ld_B[4];
extern UINT8     HL_ld_R_user[4];		//-用户记录了每种颜色的亮度值(0~255)
extern UINT8     HL_ld_G_user[4];
extern UINT8     HL_ld_B_user[4];
extern UINT8     HL_ld_brightness;


//-跑马灯
extern UINT8     RunLed_PollNum;
extern UINT8     RunLed_stata_num;
extern UINT8     RunLed_stata_flag;

//-噪声
#define maxbuffer 		1026
#define maxcounter 		1024
#define base 					1723
#define ADC_Channel_num 					3			//-系统中ADC的采样频道数
extern UINT16 	ADC_ConvertedValue[maxbuffer];
extern UINT16 	ADC_ConvertedValue_TEMP[maxbuffer];
extern UINT16	  samp_data_ram_base[ADC_Channel_num][maxbuffer/ADC_Channel_num];
extern UINT8 		Start_thansfer;
extern UINT8 		ADC_Conv_flag;
extern UINT16 	Noise_Value;
extern UINT16 	Noise_Value_flag;
extern UINT32 	ADC_Converte_cn;

//-天气
extern UINT8 Weather_flag;

//-风扇
extern UINT8 		 FAN_RUN_flag;		//-风扇运行标志 0 停止,1 运行
extern UINT8 		 FAN_RUN_wait_flag;
extern UINT16    FAN_RUN_wait_time;	//-最近一次启动风扇的时间

//-全局使用标志,暂时定义
extern UINT8     sys_err_flag;

//-测试用数组,存储点阵屏位置信息
extern UINT16    test_pt;
extern UINT16    test_cn[8];
extern UINT16    test_cn_wait_time;
extern UINT32    test_SIN_data[16];

//-模拟EEPROM 使用FLASH
extern UINT32 EEP_Data;
extern UINT8  EEP_Data_flag;

extern float float_data;

//-三轴加速度计
extern UINT16    ADXL_X_data_flag;	//-0表示数据为正数,1 表示数据为负数
extern UINT16    ADXL_Y_data_flag;
extern UINT16    ADXL_Z_data_flag;
extern UINT16    ADXL_X_data;		//-为绝对值
extern UINT16    ADXL_Y_data;
extern UINT16    ADXL_Z_data;

extern UINT8 		 ADXL_TAP_it_flag;
extern UINT8 		 ADXL_TAP_it_SOURCE;
extern UINT16		 ADXL_TAP_wait_time;

//-自检标识位
extern UINT8 		STM32_UP_selfT_flag1;
extern UINT8 		STM32_UP_error_flag1;
extern UINT8 		STM32_UP_selfT_cn1;
extern UINT8 		STM32_UP_selfT_cn2;
extern UINT8 		STM32_UP_selfT_cn3;
extern UINT8 		STM32_shedeng_selfT_flag1;

//-通讯开始
extern UINT8 		UART0_transmit_flag;
extern UINT16	  UART0_start_tx_time;

//-串口1
extern UINT8 		UART1_transmit_flag;		//-值为YES表示可以发送 值为NO表示不可以发送
extern UINT8 		UART1_transmit_control;	//-不同的值代表不同的发送内容
extern UINT8 		UART1_renew_flag;			//-0 表示可以通过串口更新数据库数据,0x55 表示不可以通过串口更新数据
extern UINT16 	UART1_renew_wait_time;

extern UINT8 		UART1_led_disdata00;
extern UINT8 		UART1_led_disdata01;
extern UINT8 		UART1_led_disdata02;

extern UINT8 		UART1_sloop_flag;
extern UINT16 	UART1_sloop_wait_time;

//-语音播报
extern UINT8 		voice_flag;

//-zigbee加网标识
extern UINT8 		zigbee_flag;
extern UINT16		zigbee_wait_time;

//-VOC
extern UINT16 	 VOC_data;
extern UINT16    VOC_data_old;
extern UINT16    VOC_renew_wait_time;
extern UINT16    VOC_data_flag;
extern UINT16    voc_rd_flag;


//-语音ES705全局变量定义
extern __IO uint32_t f_rec;
extern __IO uint32_t irq_flag;

//-后期增加全局标志位,为了合并串口
extern UINT8 		es705_mode;
extern UINT8 		es705_training_status;   //-学习模式时所处的状态
extern UINT8 		es705_training_count;   //-学习模式时成功学习的次数
extern UINT8 		es705_TO_UART2_FLAG;
extern UINT16		es705_event_type;

extern int     es705_mode_file;
extern UINT8 		es705_training_flag;  //-说明es705处于哪个状态
extern UINT16    es705_training_wait_time;
extern UINT16    es705_msleep_time;

//-iap升级
extern UINT16    IAP_ack_hang;
extern UINT16    IAP_ack_version;

//-触摸按键串口2通讯
extern UINT16    Touch_wait_time;
extern UINT8 		 Touch_Count2;
extern UINT8 		 Touch_Count2_pt;

//-低功耗STOP模式控制
extern UINT8 		 STOP_status;    //-表示目前处于哪个状态
extern UINT16    STOP_wait_time;
extern UINT8 		 STOP_status2;
extern UINT16    STOP_wait_time2;

//-蜂鸣器
extern UINT8 		beep_status;    //-表示蜂鸣器状态
extern UINT16    beep_wait_time;
extern UINT8 		beep_onoff;
extern UINT8 		Motor_onoff;

//-系统参数
extern UINT8 		SYS_WORK_MODE;		//-工作模式
extern UINT8 		SYS_LOCK_FLAG;
extern UINT8 		SYS_power_FLAG;
extern UINT32   SYS_power_wait_time;


//-菜单状态
extern UINT8 		Standby_status;    //-记录菜单状态
extern UINT8 		RUN_ONOFF_status;
extern UINT8 		RUN_status;
extern UINT8 		RUN_status_flag;
extern UINT16    RUN_status_wait_time;
extern UINT8 		menu_key_status;  //-记录对应菜单可用的按键值
extern UINT8 		menu_wind_status;
extern UINT8 		menu_set_tt_flag;
extern UINT16 		menu_set_tt_volue;
extern UINT16 		menu_set_tt_max;    //-菜单设置的制冷温度值
extern UINT16 		menu_set_tt_min;    //-菜单设置的制热温度值
extern UINT16 		menu_set_tt_max_eep;    //-菜单设置的制冷温度值,数据值扩大了100倍
extern UINT16 		menu_set_tt_min_eep;    //-菜单设置的制热温度值
extern UINT16 		menu_set_tt_max_f;    //-菜单设置的制冷温度值
extern UINT16 		menu_set_tt_min_f;    //-菜单设置的制热温度值
extern UINT16 		auto_set_tt_max_eep;    //-菜单设置的制冷温度值
extern UINT16 		auto_set_tt_min_eep;    //-菜单设置的制热温度值
extern UINT8 		menu_set_tt_more;    //-菜单设置的严重变化值
extern UINT8 		menu_set_test_flag;
extern UINT16 		menu_set_tt_simu;     //-为了调试模拟的实际温度,测试动作逻辑
extern UINT8 		menu_set_tt_CorF;
extern UINT8 		SET_Modify_flag;

extern UINT8 		menu_set_tt_swing;    //-菜单设置的一级摇摆区域值
extern UINT8 		menu_set_tt_swing_pt;
extern UINT8 		menu_set_tt_diff;    //-菜单设置的二级摇摆区域值
extern UINT8 		menu_set_tt_diff_pt;
extern UINT8 		menu_set_tt_third;
extern UINT8 		menu_set_tt_third_pt;
extern UINT8 		menu_set_tt_cal;

//-出口判断
extern UINT8 		E_out_flag;
extern UINT8 		E_start_flag;
extern UINT8 		out_flag;   //-表示出口是否需要更新
extern UINT16 		out_status; //-表示出口目前的输出值
extern UINT16 		out_status_new;
extern UINT8 		out_hold_flag;
extern UINT8 		 out_onoff_flag;
extern UINT16    out_onoff_wait_time;

extern UINT32    OUT_HEAT_one_time;
extern UINT32    OUT_HEAT_two_time;
extern UINT32    OUT_COOL_one_time;
extern UINT32    OUT_COOL_two_time;
extern UINT8    OUT_HEAT_one_flag;   //-任何一个时间基准都是有有效性的
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
extern UINT8     now_programme_flag;    //-0 表示人为就地操作失效 0x55 人为有效中,编程失效
extern UINT16    now_programme_time;
extern UINT8     now_programme_redata;

extern UINT8     OUT_emergency_hot;

extern UINT32    SET_send_one_time;
extern UINT8    SET_send_one_flag;
extern UINT32    SET_send_two_time;
extern UINT8    SET_send_two_flag;


#endif // __GloblDef_H__


