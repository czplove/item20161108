#ifndef _comuse_h
#define _comuse_h

/*
//Function
extern void Initial_CPUCOM(void);
extern void Initial_CAN(BYTE PortNo);
extern void init_port();
extern void init_all_port();
extern void port_send_begin_no_monitor(BYTE the_port_no);
extern void port_send_begin();

//extern void Ex_transmit_yk_cmd_unit();
//extern void Ex_transmit_yk_reply_unit();
extern void Ex_pipe_unit();
extern void Ex_pipe_port();

//应用 core  前提已释放 temp_loop,temp_loop1,temp_int,temp_lp_int
extern void core_update_DC();
extern void core_update_YC();
extern void core_update_YX();
extern void core_update_YM();
extern void core_insert_SOE();
extern void core_insert_SOECOS();
extern void core_get_yx_set_unit();
extern void core_get_yc_set_unit();
extern void core_get_bh_bank_report(BYTE the_port);
// by x.zhao
extern BYTE Ex_Produce_Transmit_Info(void);
extern BYTE Ex_YK_CDTBB_ObjectNo_To_UnitAddr(void);



extern void Read_Time_From_Dallas(void);
extern void  Write_Time_To_Dallas(void);

extern void Store_Rcd_Info_Myself(void);
extern void Store_Rcd_Info_System(void);

extern void  Judge_P554_CAN_Reset(void);



extern void BCH_Calculate(void);
*/
extern BYTE     Judge_Time_In_MainLoop(WORD start_time,WORD judge_value);
extern BYTE     Judge_Time_In_OtherInt(WORD start_time,WORD judge_value);
extern BYTE Judge_LongTime_In_MainLoop(WORD start_time,WORD judge_value);
extern BYTE Judge_LongTime_In_OtherInt(WORD start_time,WORD judge_value);
extern void Host_LowLevelDelay(UINT16 milliseconds);
extern void TWI_Delay(void);
extern void NOP_Delay(UINT16 i);

extern void Clock_Process(void);


#define power   0x01
#define up      0x02
#define menu    0x03
#define down    0x04
#define wind    0x05

///////////////////////////////////////////////////////////////////////////////
extern void UART2_Rx_Deal(void);
extern void RTC_Configuration(void);
extern uint16_t read_bkp_eep(uint16_t BKP_DR);
extern int MONI_EEPROM_read(void);
extern void BEEP_Off(void);
extern void MotorStatus(u8 MOTOR);
extern void TouchLedStatus(u8 Wind,u8 Down,u8 Menu,u8 Up,u8 Power);
extern void TurnOffDisLED(void);
extern u8 write_bkp_eep(uint16_t BKP_DR, uint16_t Data);
extern BYTE Judge_STime_In_MainLoop(DWORD start_time,WORD judge_value);
extern void MONI_EEPROM_write(void);
extern void RCC_Configuration(void);
extern void GPIO_Configuration(void);
extern void NVIC_Configuration(void);
extern void EXTI_Configuration(void);
extern void uart1_config(void);
extern void uart2_config(void);
extern void uart3_config(void);
extern void adc_config(void);
extern void commonIT_Config(void);
extern void AutoWakeupConfigure(void);
extern void BEEP_On(void);
extern void RTC_init(void);
extern void SHT3X_init(void);
extern void sys_init(void);
extern void menu_init(void);
extern void iwdg_init(void);
extern void TouchValueHandle(u8 *Buffer);
extern void LED_refurbish(void);
extern void uart1_Main(void);
extern void uart2_Main(void);
extern void uart3_Main(void);
extern void sys_mode_judge(void);
extern void SHT3X_ReadMeasurementBuffer(u16 *temp, u16 *humi);
extern void sec_to_calendar(u32 TimeVar);
extern void sys_delay_judge(void);
extern void uart1_to_EXTI(void);
extern void uart2_to_EXTI(void);
extern void uart3_to_EXTI(void);
extern void SYSCLKConfig_STOP(void);
extern void Time_Adjust(void);
extern void EXTI_to_uart1(void);
extern void EXTI_to_uart2(void);
extern void EXTI_to_uart3(void);
extern void led_display_hang_it(void);
extern void UART1_Rx_Deal(void);
extern void UART1_Tx_Deal(void);
extern void get_ntc_adc_value(void);
extern void LED_renew(void);
extern void KEY_read(void);
extern void Display(u8 x, u32 y);
///////////////////////////////////////////////////////////////////////////////

#endif /* _comuse_h */
