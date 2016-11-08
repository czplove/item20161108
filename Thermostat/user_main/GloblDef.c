/*
 不包含自己的头文件
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
BYTE	port_deal_flag[4]; 		//-值为0可以处理;值为0xaa表示内容还没有发送出去不可以处理
BYTE	UART3_TO_UART2_FLAG;

UL		samp_data_ch0_average;
UL		samp_data_ch1_average;
UL		samp_data_ch2_average;
UL		samp_data_ch3_average;
UL		samp_data_ch4_average;



/*struct L6_Ram
{								//start		len
	int16  samp_data_ram_base[4][128];			//0x0000	0x600  乘通道系数的结果。
	int16  samp_data_cf_ram_base[4][128];			//0x0600	0x600
	Uint16 samp_data_power_ram_base[128];			//0x0c00	0x080
	int16  samp_data_yc_ram_base[13][128];			//0x0c80	0x240  end 0x0fef
}L6Ram;
*/

BYTE	samp_data_pt;

//-1	CO2
//-2  温度
//-3  湿度
//-4  PM2.5
//-5  噪声
WORD    port_send_sense_data[20];			//-里面是存储的所有可能的传感器数据,第一个元素作为标志位,为0x55时说明有数值更新了需要上送;为0时也是刚更新的数值,只是没有达到
																			//-上送要求


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

UINT32 Device_communication_cn =0;		//-用以记录系统通讯情况

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
UINT8  m_week  =0;  //-日历中星期天表示的是0 
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


UINT16  cticks_ms; //-1ms时间基准计数器

UINT16  Time_2048ms_Counter;
UINT16  cticks_5ms;    //-5ms时间基准计数器
UINT16  cticks_500ms;
UINT16  cticks_ms_pwm;
UINT16  cticks_test;		//-测试传感器时间

UINT32  cticks_s;

UINT16  cticks_SLEEP_serial;    //-伪任务休眠计数器	位15是标识位 0 数据无效	1 数据有效	低15位为计数值(mS级)
UINT32  TEST_Sensor_wait_time;
UINT16  Sensor_data_wait_time;

///const unsigned char version_number[80] ={"DSA LINE NETGATE  ver 1.0"};

//-光照手势传感器使用的临时全局变量,测试用
UINT16	i2c1_alsdata_16;
UINT16  i2c1_psdata_16;
UINT8		i2c1_newdata_flag;		//-0 表示没有新数据;1 表示有新数据
UINT8		i2c1_psinit_flag;		  //-0 表示初始化成功;1 表示失败
UINT8		i2c1_psint_cn;		    //-记录中断持续时间
UINT8		i2c1_psint_flag;			//-0 表示中断信号无效;1 表示中断信号存在,开始进入中断后置1,直到软件清除为止
UINT8		i2c1_psint_flag_FX;
UINT8		i2c1_bus_error_cn;		//-总线出错累加都一定数量之后重新初始化总线

UINT16	i2c2_alsdata_16;
UINT16  i2c2_psdata_16;
UINT8		i2c2_newdata_flag;	  //-0 表示没有新数据;1 表示有新数据
UINT8		i2c2_psinit_flag;		  //-0 表示初始化成功;1 表示失败
UINT8		i2c2_psint_cn;		    //-记录中断持续时间
UINT8		i2c2_psint_flag;			//-0 表示中断信号无效;1 表示中断信号存在,开始进入中断后置1,直到软件清除为止
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
UINT8		ps_flag;		//-挥动手势的情况	0 没有挥动;1 由上到下;2 由下到上
UINT32		ps1_int_time;
UINT32		ps2_int_time;
UINT16		VCNL_poll_wait_time;
//-ADI新手势
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


//-点阵屏显示临时变量
UINT8		led_display_flag;			//-作为一页内容,显示的方式标志
UINT8		led_display_y;        //-显示屏显示的行号最大值,就是在动画中的显示变量
UINT8		led_display_cn;        //-显示进入中断的次数
UINT8		led_display_cn_f;
UINT8		led_display_new;			//-显示是否有新数据需要准备 0x55表示现在准备新数据;	0xaa表示数据准备好了; 0xa5表示处于动画过程中.
UINT16  led_display_data;			//-一页最多显示三位数据,所以最大是999
UINT8		led_display_num;				//-两个缓冲区轮流显示:值1 2是两个常规静态显示区,5 6是通讯显示图片
UINT8		led_display_txpic_flag;				//-0x55 表示显示通讯下发的图片 0xaa 表示图片已经刷过了,主机可以继续刷新;手势结束刷图,恢复正常显示
UINT8		led_display_txpic_num;				//-刷固定图片的偏移量
UINT16	cartoon_end_wait_time;		//-记录一个动画开始的时间以便确定什么时候结束
UINT32	led_display_data1[16];	//-保存需要显示数据的内容
UINT32	led_display_data2[16];	//-保存需要显示数据的内容
UINT32	led_display_data3[16];	//-保存需要显示数据的内容,专门显示动画效果的,最终数据区
UINT32	led_display_data4[16];	//-保存需要显示数据的内容,专门显示动画效果的,动态过程数据区
UINT32	led_display_data5[16];	//-通讯显示
UINT32	led_display_data6[16];
UINT32	led_display_data_temp[16];	//-临时保存数据的全局变量
UINT8		led_data_x;							//-表示在整个数据点中的坐标位置
UINT8		led_data_y;
UINT32	led_display_data_round[32];		//-一个全局变量用于旋转效果图
UINT32	led_display_data_round1[32];		//-作为旋转效果图的一个中间变量

UINT8		led_display_page;		//-记录目前显示的是哪一页
UINT8		led_display_page_end;		//-记录目前系统可以显示的最大页号
UINT8		led_display_start;		//-值为0x55说明现在处于刷屏状态,0xaa 表示熄灭状态,0表示彻底熄灭
UINT8   cticks_s_page;        //-没有触发条件延时一段时间后,熄灭屏幕
UINT8		led_display_long;			//-这是一个屏保时间,首先实现屏保计时,然后实现掉电保存
UINT16	led_display_end_time;	//-点阵屏熄灭之后的时长

UINT8		ps_flag_led;			//-挥动手势的情况	0 没有挥动;1 由上到下;2 由下到上,用于存储标志,供点阵屏使用,而不需要影响总ps_flag标志
UINT8		ps_flag_led_end;	//-记录最后一次挥动的手势
UINT8   led_display_move_pt;		//-左右移动时计数用的,相当于一个时间基准

UINT8		led_display_data_flag;		//-表示测量的数据是否是最新的:0 没有数据,需要等待更新;1 更新数据开始计时;2 数据有效;3 数据失效开始计时

//-测试观察用全局变量
UINT8	temp_data01_8;
UINT8	temp_data02_8;
UINT8	temp_data03_8;
UINT8	temp_data04_8;

UINT16	temp_data01;
UINT16	temp_data02;
UINT16	temp_data03;
UINT16	temp_data04;


UINT16		watch_data[512];		//-通用观察数据用数组
UINT16		watch_data_pt;

UINT8		watch_cfgdata;
UINT16		watch_cfgdata_16;

//-触摸按键
/*
BIN5		BIN4		BIN3		BIN2		BIN1		GUARD		Shield
CS3			CS1			CS0			CS4			CS5			CS2			CS15
D5      D4      D0      D1      D3			保护
*/
UINT8		CSKEY_cfgDATA;		//-读出的寄存器数据
UINT8		CSKEY_DATA;				//-形象的键值


//-by cheng CO2
UINT16 		Value_CO2_Buffer[10];
UINT8		  CO2_Counter=0;
char      ReadBuf_CO2[30];
int       Received_Over_time_co2,Received_Over_Flag_co2=0;
UINT8		  send_end_flag_co2;			//-值为1 表示发送结束了等待应答;值为0 已经应答不再等待
UINT8     Received_pt_co2=0;

UINT32    cticks_s_CO2;
UINT16    co2_data;
UINT16    co2_data_old;
UINT16    co2_renew_wait_time;
UINT16    CO2_poll_wait_time;
UINT16    CO2_data_flag;		//-表示现在PM2.5所处的等级



//-by cheng pm2.5
char      ReadBuf_pm[30];
char 			Received_Over_Flag_pm;
UINT16 		Value_PM_Buffer[10];
UINT8 		PM_Counter=0; //-记录有几个有效数值了,待求平均数据
char      RcvStatus_pm;
UINT8     Received_pt_pm=0;

UINT16    pm_data;
UINT16    pm_data_old;
UINT16    pm_renew_wait_time;
UINT16    pm_data_flag;		//-表示现在PM2.5所处的等级


//-by cheng 温湿度
UINT8 		SHT3X_status;
UINT8 		SHT3X_err_flag;
UINT16 		temperature_data;
UINT8 		temperature_data_flag;  //-0表示正数 1表示负数
UINT16 		temperature_data_x10;
UINT16 		humidity_data;
UINT16 		NTC_data;
UINT16 		temperature_data2;
UINT16 		humidity_data2;
UINT16 		temperature_data_out;
UINT32    temp_data_out_time;
UINT8     temp_data_out_flag;

//-呼吸灯
//-char      interver;//pwm全局变量
UINT8     rgb_num;				//-位0 代表GREEN,位1 代表RED,位2 代表BLUE;各位1表示亮灯 0表示灭灯
UINT8		  ps_flag_pwm;		//-挥动手势的情况	0 没有挥动;1 由上到下;2 由下到上,用于存储标志,供PWM使用,而不需要影响总ps_flag标志
UINT32    cticks_ms_pwm_loop;	//-从灭到下一个灭,这样一个周期的长度
UINT32    cticks_ms_pwm_R;
UINT32    cticks_ms_pwm_G;
UINT32    cticks_ms_pwm_B;
UINT32    cticks_ms_pwm_4;
UINT16    GREEN_pwmval;
UINT16    RED_pwmval;
UINT16    BLUE_pwmval;
UINT16    GREEN_pwmval_pt;		//-偏移量
UINT16    RED_pwmval_pt;
UINT16    BLUE_pwmval_pt;
UINT16    white_pwmval_pt;
UINT16    GREEN_pwmval_num;		//-一个占空比占有的时长
UINT16    RED_pwmval_num;
UINT16    BLUE_pwmval_num;
UINT16    white_pwmval_num;
UINT8     HRL_RUN_ONOFF;	//-表示是否开环境指示灯
UINT8     HRL_RUN_flag;		//-跑马灯运行标志 0 不运行可以显示加网状态;0x55 处于启动状态;0xaa 显示环境状态
UINT8     HRL_pt;		//-跑马灯指针
UINT8     HRL_pt_start;
UINT8     HRL_pt_end;
UINT8     HRL_color_pt;		//-0 1 2 分别代表一个颜色
UINT16    HRL_pt_time00;
UINT16    HRL_pt_time01;
//-运用在彩灯上时
UINT8     HL_flag;		//-彩灯标志,用于表示彩灯的运行模式:0 关闭状态;1 彩灯状态
UINT16    HL_run_time;		//-彩灯运行时间计时
UINT8     HL_new_value_flag;		//-0 表示占空比没有变化;1 有灯的占空比发生了变化
UINT16    HL_GREEN_pwmval;
UINT16    HL_RED_pwmval;
UINT16    HL_BLUE_pwmval;
UINT8     HL_step;		//-控制炫彩灯运行的步奏
UINT8     HL_ld_R[4];		//-记录了每种颜色的亮度值(0~255)
UINT8     HL_ld_G[4];
UINT8     HL_ld_B[4];
UINT8     HL_ld_R_user[4];		//-用户记录了每种颜色的亮度值(0~255)
UINT8     HL_ld_G_user[4];
UINT8     HL_ld_B_user[4];
UINT8     HL_ld_brightness;		//-保存调光值

//-跑马灯
UINT8     RunLed_PollNum;
UINT8     RunLed_stata_num;		//-记录了目前所处的显示页点阵屏
UINT8     RunLed_stata_flag;		//-指示现在传感器监测到的环境灯

//-噪声
UINT16 		ADC_ConvertedValue[maxbuffer];
UINT16 		ADC_ConvertedValue_TEMP[maxbuffer];
UINT16	  samp_data_ram_base[ADC_Channel_num][maxbuffer/ADC_Channel_num];			//0x0000	0x600  乘通道系数的结果。
UINT8 		Start_thansfer=0;
UINT8 		ADC_Conv_flag;
UINT16 		Noise_Value;
UINT16 		Noise_Value_flag;
UINT32 		ADC_Converte_cn;		//-转换的计数次数

//-天气
UINT8 Weather_flag;		//-0 表示没有天气数据;1晴，2阴，3多云，4雨，5雪

//-风扇
UINT8 		FAN_RUN_flag;		//-风扇运行标志 0 停止,1 运行
UINT8 		FAN_RUN_wait_flag;
UINT16    FAN_RUN_wait_time;	//-最近一次启动风扇的时间


//-全局使用标志,暂时定义
UINT8     sys_err_flag;		//-数据代表错误类型,多个一起错误现在不考虑
//-1 触摸按键总线出错
//-2 上手势传感器出错
//-3 下手势传感器出错

//-测试用数组,存储点阵屏位置信息
UINT16    test_pt;
UINT16    test_cn[8];
UINT16    test_cn_wait_time;
UINT32    test_SIN_data[16];


//-模拟EEPROM 使用FLASH
UINT32 EEP_Data;
UINT8  EEP_Data_flag;		//-0x55说明数据有修改,需要写入FLASH



float float_data;

//-三轴加速度计
UINT16    ADXL_X_data_flag;	//-0表示数据为正数,1 表示数据为负数
UINT16    ADXL_Y_data_flag;
UINT16    ADXL_Z_data_flag;
UINT16    ADXL_X_data;		//-为绝对值
UINT16    ADXL_Y_data;
UINT16    ADXL_Z_data;

UINT8 		ADXL_TAP_it_flag;			//-值为0 没有中断;值为1 处于中断中
UINT8 		ADXL_TAP_it_SOURCE;		//-记录中断源值
UINT16		ADXL_TAP_wait_time;		//-这个时间用于防抖动

//-自检标识位
UINT8 		STM32_UP_selfT_flag1;		//-每一位代表一个器件的好坏:0 正常,1 异常.位0 三轴加速度
UINT8 		STM32_UP_error_flag1;		//-说明外设永久损坏需要重新上电或者更换器件
UINT8 		STM32_UP_selfT_cn1;
UINT8 		STM32_UP_selfT_cn2;
UINT8 		STM32_UP_selfT_cn3;
UINT8 		STM32_shedeng_selfT_flag1;		//-射灯板自检状态

//-通讯开始
UINT8 		UART0_transmit_flag;
UINT16	  UART0_start_tx_time;

//-串口1
UINT8 		UART1_transmit_flag;		//-值为YES表示可以发送 值为NO表示不可以发送
UINT8 		UART1_transmit_control;	//-不同的值代表不同的发送内容

UINT8 		UART1_renew_flag;			//-0 表示可以通过串口更新数据库数据,0x55 表示不可以通过串口更新数据
UINT16 		UART1_renew_wait_time;

UINT8 		UART1_led_disdata00;
UINT8 		UART1_led_disdata01;
UINT8 		UART1_led_disdata02;

UINT8 		UART1_sloop_flag;
UINT16 		UART1_sloop_wait_time;

//-语音播报
UINT8 		voice_flag;				//-0 表示不播报;0x55 播报


//-zigbee加网标识
UINT8 		zigbee_flag;
UINT16		zigbee_wait_time;		//-允许加网的时间延时


//-VOC
UINT16 		VOC_data;
UINT16    VOC_data_old;
UINT16    VOC_renew_wait_time;
UINT16    VOC_data_flag;
UINT16    voc_rd_flag;



//-语音ES705全局变量定义
__IO uint32_t irq_flag = 0;
__IO uint32_t f_rec  = 0;
__IO uint8_t	rx1_cnt = 0;
uint8_t uart1_rx_buff[100];
  //-后期增加全局标志位,为了合并串口
UINT8 		es705_mode;     //-现在语音芯片所处的模式
UINT8 		es705_training_status;   //-学习模式时所处的状态
UINT8 		es705_training_count;    //-学习模式时成功学习的次数
UINT8 		es705_TO_UART2_FLAG;     //-这个标志位决定了STM32通过串口2向网关7620发送的内容
UINT16 		es705_event_type;        //-语音唤醒的类型

int       es705_mode_file;
UINT8 		es705_training_flag;  //-说明es705处于哪个状态
UINT16    es705_training_wait_time;   //-一次命令进入学习状态,最多5Min
UINT16    es705_msleep_time;

//-iap升级
UINT16    IAP_ack_hang;
UINT16    IAP_ack_version;

//-触摸按键串口2通讯
UINT16    Touch_wait_time;
UINT8 		Touch_Count2;
UINT8 		Touch_Count2_pt;

//-低功耗STOP模式控制
UINT8 		STOP_status;    //-表示目前处于哪个状态
UINT16    STOP_wait_time;
UINT8 		STOP_status2;
UINT16    STOP_wait_time2;

//-蜂鸣器
UINT8 		beep_status;    //-表示蜂鸣器状态
UINT16    beep_wait_time;
UINT8 		beep_onoff;
UINT8 		Motor_onoff;

//-系统参数
UINT8 		SYS_WORK_MODE;		//-工作模式
UINT8 		SYS_LOCK_FLAG;
UINT8 		SYS_power_FLAG;   //-系统电源标志
UINT32    SYS_power_wait_time;

//-菜单状态
UINT8 		Standby_status;    //-待机状态记录
UINT8 		RUN_ONOFF_status; //-开关机情况
UINT8 		RUN_status;    //-记录运行状态
UINT8 		RUN_status_flag;    //-表示是否切换了工作模式 0x55切换了
UINT16    RUN_status_wait_time;   //-当切换到新模式的时候,延时一段时间有效,防止反复切换
UINT8 		menu_key_status;  //-记录对应菜单可用的按键值 风速 减 菜单 加 关机/开机
UINT8 		menu_wind_status;			//-表示风扇的运行情况,位7表示是否值有更新
UINT8 		menu_set_tt_flag;    //-值为0说明没有修改,值为1说明处于修改中
UINT16 		menu_set_tt_volue;   //-当前显示的有效值
UINT16 		menu_set_tt_max;    //-菜单设置的制冷温度值,,实际值比最大值还大就需要制冷了
UINT16 		menu_set_tt_min;    //-菜单设置的制热温度值,,实际值比最小值还小就需要制热了
UINT16 		menu_set_tt_max_eep;    //-菜单设置的制热温度值
UINT16 		menu_set_tt_min_eep;    //-菜单设置的制冷温度值
UINT16 		menu_set_tt_max_f;    //-菜单设置的制热温度值
UINT16 		menu_set_tt_min_f;    //-菜单设置的制冷温度值
UINT16 		auto_set_tt_max_eep;    //-菜单设置的制热温度值
UINT16 		auto_set_tt_min_eep;    //-菜单设置的制冷温度值
UINT8 		menu_set_tt_more;    //-菜单设置的温度值
UINT8 		menu_set_test_flag;   //-表示是否进入测试模式
UINT16 		menu_set_tt_simu;
UINT8 		menu_set_tt_CorF;
UINT8 		SET_Modify_flag;

UINT8 		menu_set_tt_swing;    //-菜单设置的一级摇摆区域值
UINT8 		menu_set_tt_swing_pt;
UINT8 		menu_set_tt_diff;    //-菜单设置的二级摇摆区域值
UINT8 		menu_set_tt_diff_pt;
UINT8 		menu_set_tt_third;
UINT8 		menu_set_tt_third_pt;
UINT8 		menu_set_tt_cal;    //-温度校准


//-出口判断
UINT8 		E_out_flag;   //-紧急制热标志
UINT8 		E_start_flag; //-用于记录紧急制热是否启动
UINT8 		out_flag;   //-表示出口是否需要更新
UINT16 		out_status; //-表示出口目前的输出值
UINT16 		out_status_new; //-表示出口新数据等待更新
UINT8 		out_hold_flag;
UINT8 		 out_onoff_flag;
UINT16    out_onoff_wait_time;

UINT32    OUT_HEAT_one_time;
UINT32    OUT_HEAT_two_time;
UINT32    OUT_COOL_one_time;
UINT32    OUT_COOL_two_time;
UINT8    OUT_HEAT_one_flag;   //-任何一个时间基准都是有有效性的
UINT8    OUT_HEAT_two_flag;
UINT8    OUT_COOL_one_flag;
UINT8    OUT_COOL_two_flag;

UINT8    OUT_W1orAUX_flag;
UINT32    OUT_W1orAUX_time;
UINT8    OUT_Y1_flag;
UINT32    OUT_Y1_time;
UINT8    OUT_Y2_flag;
UINT32    OUT_Y2_time;

PROGRAMME_DAY_CH  programme_day[3*7];   //-每天3张表,每周7天,总共21张表,依次制热,制冷,自动三种模式
UINT8     now_programme_flag;    //-0 表示人为就地操作失效 0x55 人为有效中,编程失效
UINT16    now_programme_time;   //-记录最后一次修改的时间
UINT8     now_programme_redata;

UINT8     OUT_emergency_hot;


UINT32    SET_send_one_time;
UINT8    SET_send_one_flag;
UINT32    SET_send_two_time;
UINT8    SET_send_two_flag;






