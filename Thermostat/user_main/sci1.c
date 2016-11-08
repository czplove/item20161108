/*
接收:使用一个常规逻辑,开辟一个512字节的缓冲区,然后循环存储
发送:也使用512字节的空间保存内容,然后置一个发送标志,直到内容发送出去为止

串口3 作为射灯板和7620之间通讯透传口,现在决定不进行任何处理,仅仅双向透传
7620发来的需要传递到射灯板的东西,直接让串口3传送
STM32传出的数据,直接在串口3中处理,不传递任何数据,仅仅置标志位让串口2自己组织数据向7620发送

2016/4/9 15:45:27
1.设置制热温度
*/
#include "user_conf.h"




#define   MOD_TC_WAIT_TIME_VALUE                   250

#define   MOD_TC_polling_ID_FRAME                  0x01
#define   MOD_TC_polling_data_FRAME                0x02
#define   MOD_TC_LED_display_FRAME                 0x03
#define   MOD_TC_receive_data_FRAME                0x04
#define   MOD_TC_receive_set_FRAME                 0x05
#define   MOD_TC_SET_screen_FRAME									 0x06
#define   MOD_TC_mode_set_FRAME										 0x07
#define   MOD_TC_chage_page_FRAME									 0x08
#define   MOD_TC_chage_pic_FRAME									 0x09
#define   MOD_TC_SET_loop_FRAME		  							 0x0A
#define   MOD_TC_send_state_FRAME		  						 0x0B
#define   MOD_TC_re_Weather_FRAME		  						 0x0C
#define   MOD_TC_re_show_FRAME		  						   0x0D
#define   MOD_TC_polling_state_FRAME		  				 0x0E
#define   MOD_TC_polling_PM25_FRAME		  				   0x0F
#define   MOD_TC_end_PM25_FRAME		  	    			   0x10
#define   MOD_TC_polling_hl_FRAME		  	    			 0x11
#define   MOD_TC_SET_hl_time_FRAME		  	    		 0x13
#define   MOD_TC_send_leddis_flag_FRAME		     		 0x14
#define   MOD_TC_SET_voice_flag_FRAME		     		   0x15
#define   MOD_TC_polling_voice_flag_FRAME		   	   0x16
#define   MOD_TC_re_gateway_flag_FRAME		    	   0x17

#define   MOD_TC_send_voice_mode_FRAME	      	   0x1B

#define   MOD_TC_set_hl_set_FRAME	     	           0x21

#define   MOD_TC_7620_inquire_upSDB_FRAME          0x22
#define   MOD_TC_7620_inquire_update_FRAME	       0x23
#define   MOD_TC_7620_inquire_version_FRAME	       0x24



#define   MOD_TC_control_HRL_FRAME		  	    	   0x40
#define   MOD_TC_leddis_end_FRAME	  	  	    	   0x41
#define   MOD_TC_send_HRL_mode_FRAME	  	     	   0x42
#define   MOD_TC_control_FUN_FRAME	    	     	   0x43
#define   MOD_TC_update_status_FRAME	    	     	 0x44


//-定义的常数表格,每个表增加一个成员0,这样就可以对应从1开始
BYTE swing_constant[] = {0,2,5,8,10,12,15,18,20};
BYTE diff_constant[] = {0,10,15,20,25,30};
BYTE third_constant[] = {0,25,30,35,40,45,50};

//-为协议处理临时定义的变量
BYTE  MOD_TC_rxd_head_flag;
BYTE  MOD_TC_rec_OK;
BYTE  MOD_TC_rec_frame_type;
WORD  MOD_TC_rxd_wait_time;
BYTE  MOD_TC_transmit_flag;
BYTE  MOD_TC_transmit_control;
WORD  MOD_TC_transmit_wait_time;



////////////////////////////////////////////////////////////////////////////////
/*
例子1:查询传感器设备（ID）
7620发送：
0xaa 0x55 0x05 0x02 0x01 CRC
STM32回应：
0xaa 0x55 0x0b 0x02 0x01 0x01 0x02 0x03 0x04 0x05 0x06 CRC






*/
unsigned int MOD_TC_CRC16(unsigned char *MOD_TC_CRC16_start,unsigned char MOD_TC_CRC16_bytes)    //*x为指向每行前5个数据的指针
{	//-计算冗余校验CRC
unsigned int bx, cx, i, j;

    bx = 0xffff;
    cx = 0xa001;
    for(i=0;i<MOD_TC_CRC16_bytes;i++)
     {
      bx=bx^MOD_TC_CRC16_start[i];
      for(j=0;j<8;j++)
       {
        if ((bx&0x0001)==1)
         {
          bx=bx>>1;
          bx=bx&0x7fff;
          bx=bx^cx;
         }
        else
         {
          bx=bx>>1;
          bx=bx&0x7fff;
         }
       }
     }
    return(bx);
}

void MOD_TC_clear_port_report_deal(BYTE date_len)
{
	 WORD i,temp_loop;

	 //-len = port_report[2] + 2;
	 temp_loop = port_recv_dl[0];

	 for(i=0;i<date_len;i++)
	 {
	 	  if(temp_loop > 0)
	 	  	temp_loop = temp_loop - 1;
	 	  else
	 	  	temp_loop = 255;
	    port_recv[0][temp_loop] = 0;
	 }


	 for(i=0;i<date_len;i++)
	   port_report[i] = 0;
}

void MOD_TC_normal_respond(BYTE* temp_data)
{
    WORD the_ram_ax;

	  port_send[0][0] = 0xaa;		//-两字节包头
	  port_send[0][1] = 0x55;

	  port_send[0][2] = 0x01;		//-整包长： 1个字节，从包头开始，到CRC16之前的数据。

    port_send[0][3] = 0x06;
	  port_send[0][4] = temp_data[0];		//-功能码:01h 传感器主动上报

	  //-有效数据
	  //-这里的有效数据又是以块为单位的"数据长度+设备ID+功能码+数据描述+数据位"
	  port_send[0][5] = temp_data[1];		//-数据长度
    port_send[0][6] = temp_data[2];
    port_send[0][7] = temp_data[3];

	  //-CRC16
	  the_ram_ax=MOD_TC_CRC16(&port_send[0][0],8);
	  port_send[0][8] =LOBYTE(the_ram_ax);
	  port_send[0][9] =HIBYTE(the_ram_ax);	//-直到这里所有的报文内容都已经准备好了

	  //-发送长度
	  port_send_len[0] = 10;
	  port_deal_flag[0] = 0xaa;

	  //-cticks_500ms = 0;
}

void MOD_TC_10normal_respond(BYTE *data_pt)
{
    WORD the_ram_ax;

	  port_send[0][0] = 0xaa;		//-两字节包头
	  port_send[0][1] = 0x55;

	  port_send[0][2] = 0x01;		//-整包长： 1个字节，从包头开始，到CRC16之前的数据。

    port_send[0][3] = 0x10;

	  port_send[0][4] = data_pt[0];		//-功能码:01h 传感器主动上报

	  //-有效数据
	  //-这里的有效数据又是以块为单位的"数据长度+设备ID+功能码+数据描述+数据位"
	  port_send[0][5] = data_pt[1];		//-数据长度

    port_send[0][6] = data_pt[2];
    port_send[0][7] = data_pt[3];

	  //-CRC16
	  the_ram_ax=MOD_TC_CRC16(&port_send[0][0],8);
	  port_send[0][8] =LOBYTE(the_ram_ax);
	  port_send[0][9] =HIBYTE(the_ram_ax);	//-直到这里所有的报文内容都已经准备好了

	  //-发送长度
	  port_send_len[0] = 10;
	  port_deal_flag[0] = 0xaa;

	  //-cticks_500ms = 0;
}


//-下面是主动发送的报文
void MOD_TC_ask_join_cmd(void)		//-接收到查询器件状态命令之后,直接返回状态值
{
	  WORD the_ram_ax;

	  //-如果数据成功接收到之后就应答一次
			  port_send[0][0] = 0xaa;		//-两字节包头
			  port_send[0][1] = 0x55;

			  port_send[0][2] = 0x01;		//-整包长： 1个字节，从包头开始，到CRC16之前的数据。

			  port_send[0][3] = 0x41;		//-功能码:01h 传感器主动上报

			  //-有效数据
		    port_send[0][4] = 0x40;		//-数据长度

			  port_send[0][5] = 0x00;		//-数据长度
			  port_send[0][6] = 0x00;		//-数据长度
			  port_send[0][7] = 0x01;

			  //-CRC16
			  the_ram_ax=MOD_TC_CRC16(&port_send[0][0],8);
			  port_send[0][8] =LOBYTE(the_ram_ax);
			  port_send[0][9] =HIBYTE(the_ram_ax);	//-直到这里所有的报文内容都已经准备好了

			  //-发送长度
			  port_send_len[0] = 8 + 2;
			  port_deal_flag[0] = 0xaa;

}

void MOD_TC_ask_move_cmd(void)		//-接收到查询器件状态命令之后,直接返回状态值
{
	  WORD the_ram_ax;

	  //-如果数据成功接收到之后就应答一次
			  port_send[0][0] = 0xaa;		//-两字节包头
			  port_send[0][1] = 0x55;

			  port_send[0][2] = 0x01;		//-整包长： 1个字节，从包头开始，到CRC16之前的数据。

			  port_send[0][3] = 0x42;		//-功能码:01h 传感器主动上报

			  //-有效数据
		    port_send[0][4] = 0x40;		//-数据长度

			  port_send[0][5] = 0x01;		//-数据长度
			  port_send[0][6] = 0x00;		//-数据长度
			  port_send[0][7] = 0x01;

			  //-CRC16
			  the_ram_ax=MOD_TC_CRC16(&port_send[0][0],8);
			  port_send[0][8] =LOBYTE(the_ram_ax);
			  port_send[0][9] =HIBYTE(the_ram_ax);	//-直到这里所有的报文内容都已经准备好了

			  //-发送长度
			  port_send_len[0] = 8 + 2;
			  port_deal_flag[0] = 0xaa;

}

void MOD_TC_ask_program_cmd(void)		//-接收到查询器件状态命令之后,直接返回状态值
{
	  WORD the_ram_ax;

	  //-如果数据成功接收到之后就应答一次
			  port_send[0][0] = 0xaa;		//-两字节包头
			  port_send[0][1] = 0x55;

			  port_send[0][2] = 0x01;		//-整包长： 1个字节，从包头开始，到CRC16之前的数据。

			  port_send[0][3] = 0x43;		//-功能码:01h 传感器主动上报

			  //-有效数据
		    port_send[0][4] = 0x40;		//-数据长度

			  port_send[0][5] = 0x20;		//-数据长度
			  port_send[0][6] = 0x00;		//-数据长度
			  port_send[0][7] = 0x54;

			  //-CRC16
			  the_ram_ax=MOD_TC_CRC16(&port_send[0][0],8);
			  port_send[0][8] =LOBYTE(the_ram_ax);
			  port_send[0][9] =HIBYTE(the_ram_ax);	//-直到这里所有的报文内容都已经准备好了

			  //-发送长度
			  port_send_len[0] = 8 + 2;
			  port_deal_flag[0] = 0xaa;

}

void MOD_TC_send_status_data(void)		//-主动上送状态变化值
{
	  WORD the_ram_ax;

	  //-如果数据成功接收到之后就应答一次
			  port_send[0][0] = 0xaa;		//-两字节包头
			  port_send[0][1] = 0x55;

			  port_send[0][2] = 0x01;		//-整包长： 1个字节，从包头开始，到CRC16之前的数据。

			  port_send[0][3] = 0x43;		//-功能码:01h 传感器主动上报

			  //-有效数据
		    port_send[0][4] = 0x18;		//-字节数 = 2 * Zigbee读写寄存器个数

        //-运行状态,MM->00:关闭，MM->01：打开
			  port_send[0][5] = 0x00;		//-数据Hi
        //-if(RUN_status == 0)
        //-  port_send[0][6] = 0x00;		//-数据Li
        //-else
          port_send[0][6] = RUN_ONOFF_status;		//-数据Li
        
        //-风机模式,MM->01自动，MM->02：打开
        port_send[0][7] = 0x00;
        if((menu_wind_status & 0x01) == 1)
          port_send[0][8] = 0x02;
        else
          port_send[0][8] = 0x01;
        
        //-温标
        port_send[0][9] = 0x00;        
        port_send[0][10] = menu_set_tt_CorF;
        
        //-设备类型
        port_send[0][11] = 0x00;        
        port_send[0][12] = SYS_WORK_MODE;
        
        //-运行模式,MM->01制热，MM->02：制冷，MM->03：自动
			  port_send[0][13] = 0x00;
        port_send[0][14] = RUN_status;
        
        //-温度
        port_send[0][15] = menu_set_tt_min_eep >> 8;   //-制热
        port_send[0][16] = menu_set_tt_min_eep & 0xff;   //-制冷

        if(temperature_data_flag)
          the_ram_ax = 0 - port_send_sense_data[2];
        else
          the_ram_ax = port_send_sense_data[2];
        port_send[0][17] = the_ram_ax >> 8;
        port_send[0][18] = the_ram_ax;

        //-湿度
        port_send[0][19] = 0;
        port_send[0][20] = humidity_data & 0xff;

        //-自动制热 
        port_send[0][21] = auto_set_tt_min_eep >> 8;   //-制热
        port_send[0][22] = auto_set_tt_min_eep & 0xff;   //-制冷 

        //-锁定
        port_send[0][23] = 0;
        port_send[0][24] = SYS_LOCK_FLAG;
        
        //-制冷温度
        port_send[0][25] = menu_set_tt_max_eep >> 8;   //-制热
        port_send[0][26] = menu_set_tt_max_eep & 0xff;   //-制冷
        
        //-自动制冷温度
        port_send[0][27] = auto_set_tt_max_eep >> 8;   //-制热
        port_send[0][28] = auto_set_tt_max_eep & 0xff;   //-制冷

        if(now_programme_flag == 1)
        {
            if(RUN_status == 1)
            {//-制热
              the_ram_ax = menu_set_tt_min * 10;
              port_send[0][15] = the_ram_ax >> 8;   //-制热
              port_send[0][16] = the_ram_ax & 0xff;   //-制冷
            }
            else if(RUN_status == 2)
            {//-制冷
              the_ram_ax = menu_set_tt_max * 10;
              port_send[0][25] = the_ram_ax >> 8;   //-制热
              port_send[0][26] = the_ram_ax & 0xff;   //-制冷
            }
            else if(RUN_status == 3)
            {//-自动
              the_ram_ax = menu_set_tt_min * 10;
              port_send[0][21] = the_ram_ax >> 8;   //-制热
              port_send[0][22] = the_ram_ax & 0xff;   //-制冷 
              the_ram_ax = menu_set_tt_max * 10;
              port_send[0][27] = the_ram_ax >> 8;   //-制热
              port_send[0][28] = the_ram_ax & 0xff;   //-制冷
            }
        }
        
			  //-CRC16
			  the_ram_ax=MOD_TC_CRC16(&port_send[0][0],29);
			  port_send[0][29] =LOBYTE(the_ram_ax);
			  port_send[0][30] =HIBYTE(the_ram_ax);	//-直到这里所有的报文内容都已经准备好了

			  //-发送长度
			  port_send_len[0] = 29 + 2;
			  port_deal_flag[0] = 0xaa;

}

void MOD_TC_send_status_data2(void)		//-主动上送状态变化值
{
	  WORD the_ram_ax;

	  //-如果数据成功接收到之后就应答一次
			  port_send[0][0] = 0xaa;		//-两字节包头
			  port_send[0][1] = 0x55;

			  port_send[0][2] = 0x01;		//-整包长： 1个字节，从包头开始，到CRC16之前的数据。

			  port_send[0][3] = 0x44;		//-功能码:01h 传感器主动上报

			  //-有效数据
		    port_send[0][4] = 0x12;		//-字节数 = 2 * Zigbee读写寄存器个数

        //-温度校准
			  port_send[0][5] = 0x00;		//-数据Hi
        port_send[0][6] = menu_set_tt_cal;		//-数据Li
        
        //-
        port_send[0][7] = 0x00;
        port_send[0][8] = menu_set_tt_swing_pt;
        
        //-
        port_send[0][9] = 0x00;        
        port_send[0][10] = menu_set_tt_diff_pt;
        
        //-
        port_send[0][11] = 0x00;        
        port_send[0][12] = menu_set_tt_third_pt;
        
        //-声效设置
			  port_send[0][13] = 0x00;
        port_send[0][14] = beep_onoff;
        
        //-紧急制热
        port_send[0][15] = 0;   
        port_send[0][16] = OUT_emergency_hot;   //-

        port_send[0][17] = m_year - 2000;
        port_send[0][18] = m_month;
        port_send[0][19] = m_date;
        port_send[0][20] = m_hour;
        port_send[0][21] = m_min;
        port_send[0][22] = m_sec;
        
        //-震动
        port_send[0][23] = 0;   
        port_send[0][24] = Motor_onoff;   //-

			  //-CRC16
			  the_ram_ax=MOD_TC_CRC16(&port_send[0][0],25);
			  port_send[0][25] =LOBYTE(the_ram_ax);
			  port_send[0][26] =HIBYTE(the_ram_ax);	//-直到这里所有的报文内容都已经准备好了

			  //-发送长度
			  port_send_len[0] = 25 + 2;
			  port_deal_flag[0] = 0xaa;

}

void MOD_TC_send_ASK_data(void)		//-主动上送状态变化值
{
	  WORD the_ram_ax;

	  //-如果数据成功接收到之后就应答一次
			  port_send[0][0] = 0xaa;		//-两字节包头
			  port_send[0][1] = 0x55;

			  port_send[0][2] = 0x01;		//-整包长： 1个字节，从包头开始，到CRC16之前的数据。

			  port_send[0][3] = 0x45;		//-功能码:01h 传感器主动上报

			  //-有效数据
		    port_send[0][4] = 0x40;		//-字节数 = 2 * Zigbee读写寄存器个数
			  port_send[0][5] = 0x0F;		//-数据Hi
        
        port_send[0][6] = 0x00;		//-数据Li
        port_send[0][7] = 0x01;        

			  //-CRC16
			  the_ram_ax=MOD_TC_CRC16(&port_send[0][0],8);
			  port_send[0][8] =LOBYTE(the_ram_ax);
			  port_send[0][9] =HIBYTE(the_ram_ax);	//-直到这里所有的报文内容都已经准备好了

			  //-发送长度
			  port_send_len[0] = 10;
			  port_deal_flag[0] = 0xaa;

}

void MOD_TC_send_power_state(void)		//-主动上送状态变化值
{
	  WORD the_ram_ax;

	  //-如果数据成功接收到之后就应答一次
			  port_send[0][0] = 0xaa;		//-两字节包头
			  port_send[0][1] = 0x55;

			  port_send[0][2] = 0x01;		//-整包长： 1个字节，从包头开始，到CRC16之前的数据。

			  port_send[0][3] = 0x46;		//-功能码:01h 传感器主动上报

			  //-有效数据
		    port_send[0][4] = 0x40;		//-字节数 = 2 * Zigbee读写寄存器个数
			  port_send[0][5] = 0x0E;		//-数据Hi
        
        port_send[0][6] = 0x00;		//-数据Li
        if(SYS_power_FLAG == 0)
          port_send[0][7] = 0x00; 
        else
          port_send[0][7] = 0x01;

			  //-CRC16
			  the_ram_ax=MOD_TC_CRC16(&port_send[0][0],8);
			  port_send[0][8] =LOBYTE(the_ram_ax);
			  port_send[0][9] =HIBYTE(the_ram_ax);	//-直到这里所有的报文内容都已经准备好了

			  //-发送长度
			  port_send_len[0] = 10;
			  port_deal_flag[0] = 0xaa;

}

////////////////////////////////////////////////////////////////////////////////
//-现在需要根据实际要求来处理报文,这个是类Modbus协议,然后主要是作为从机用,某些情况
//-也作为主机,或者稍微编译

void uart1_Main(void)
{
   WORD the_ram_ax,the_ram_bx,i;
	 WORD temp_data,temp_data1,delta_len,temp_loop,temp_int;


	 //-判断发送是否结束了,如果结束了可以正常逻辑处理,否则不处理
	 if(DMA_GetFlagStatus(DMA1_FLAG_TC4))		//-进入说明发送完成了
   {//-只有当传输完成了才可以重新给DMA发送传输命令
   	  //-内容发送完成之后全部清理,由于现在不知道每个的空间大小了,那么选取一块大的清零
   	  //-for(temp_loop = 0;temp_loop < 43;temp_loop++)
   	  //-	port_send[0][temp_loop] = 0;
   	  port_deal_flag[0] = 0;		//-给一个非0值就可以再下次检查发送时发发送命令了
   	  //-MOD_TC_transmit_flag=YES;		//-当我的内容发送出去了也可以立即发送内容了??这里应该不能立即组织内容的发送,这样下面的内容会被覆盖,接收的标志位
   	  DMA_ClearFlag(DMA1_FLAG_TC4);
   }

   if((port_deal_flag[0] == 0) /*&& (UART3_TO_UART2_FLAG == 0)*/)	//-这里会不会由于一次发送失败而导致再也不能发送,,增加UART3_TO_UART2_FLAG为0开始处理接收到的报文,防止,有内容没有发送出去覆盖了,造成的丢失
   {
   	   //-首先处理接收到的数据,如果没有内容接收到,才组织可能需要发送的内容
   	   if(port_recv_pt[0]!=port_recv_dl[0])
   	   {

   	   	   if(MOD_TC_rxd_head_flag==NO)	//-接收到的数据还没有处理的时候就是NO
           {//-寻找到头0xaa 0x55
           	   MOD_TC_rxd_wait_time=cticks_5ms;	//-仅仅是为了接收完整
		   	   	   temp_data = port_recv_pt[0];
		   	   	   temp_data1 = port_recv_dl[0];
		           if(temp_data1>temp_data)	//-前面的是处理指针比实际的接收指针进行比较
		               delta_len=(temp_data+256)-temp_data1;
		           else
		               delta_len=temp_data-temp_data1;	//-一共的长度
		           for(temp_loop=temp_data1;temp_loop<(delta_len+temp_data1);temp_loop++)
		           {
		        	   if(port_recv[0][port_recv_dl[0]]==0xaa)	//-这个地方比较的是从站地址,但是我觉得没有任何规律就是通讯
		        	   {	//-利用一切可以利用的
		        	     the_ram_ax=(port_recv_dl[0]+1)&0xff;
		        	     if(temp_data == the_ram_ax)	//-如果预取的指针指向空的空间,说明数据还没有到,需要等待
		        	     	 break;
		        	     if(port_recv[0][the_ram_ax]==0x55)	//-比较的是功能码
		        	     {
		        	         MOD_TC_rxd_head_flag=YES;	//-表示已经成功识别接收到的新报文的头了
		        	         break;
		        	     }
		        	   }
		        	   port_recv_dl[0]++;	//-舍弃一个字的报文
		        	   port_recv_dl[0]&=0xff;
		           }
   	   	   }
   	   	   if(MOD_TC_rxd_head_flag==YES)	//-接收到的数据还没有处理的时候就是NO
       		 {
       		 	   temp_data = port_recv_pt[0];
       		 	   if(port_recv_dl[0]>temp_data)
               		delta_len=(temp_data+256)-port_recv_dl[0];
               else
               		delta_len=temp_data-port_recv_dl[0];

               if(delta_len>=6)	//-至少还有4个字节才能组织一包内容
		           {//-需要保证包含了功能码
		               temp_int=(port_recv_dl[0]+3)&0xff;
		               //-if(delta_len>=(unsigned short)(port_recv[0][temp_int]+2))	//-得到的报文长度和理论上的报文长度进行比较
		               if((port_recv[0][temp_int]==0x03) || (port_recv[0][temp_int]==0x06))
                   {
                     if(delta_len>=10)
                     {//-到这里已经确保接收完毕了
		                  MOD_TC_rxd_head_flag=NO;
		                  MOD_TC_rec_OK=YES;
                      goto rec_ok_deal;	//-经过重重考核,到这里就认为是成功接收到一个返回报文了
                     }
		               }
                   else if(port_recv[0][temp_int]==0x10)
                   {
                     temp_data = (port_recv_dl[0]+8)&0xff;
                     temp_data1 = port_recv[0][temp_data] + 11;
                      if(delta_len>=temp_data1)
                      {
                        MOD_TC_rxd_head_flag=NO;
                        MOD_TC_rec_OK=YES;
                        goto rec_ok_deal;	//-经过重重考核,到这里就认为是成功接收到一个返回报文了
                      }
                   }
                   else
                   {

                   }

		           }

     			 }
   	   }
   	   goto rxd_out_time;		//?由于这个不是简单的主从模式,所以直接查询发送
rec_ok_deal:
	    if(MOD_TC_rec_OK==YES)	//-肯定是防止跑飞的.
	    {	//-到这里就可以说明应答报文已经可以了
	        MOD_TC_rec_OK=NO;	//-成功接收到的数据开始处理了之后,就恢复0
	        MOD_TC_transmit_flag=YES;
	        //-MOD_TC_transmit_wait_time=Time_1ms_Counter;	//-虽然上面说可以发送数据了,但是还是要延时一段时间,因为接收到的数据需要处理
	        //-下面是对实际数据的处理,处理的是核心部分
	        the_ram_bx=(port_recv_dl[0]+3)&0xff;;
	        if((port_recv[0][the_ram_bx]==0x03) || (port_recv[0][the_ram_bx]==0x06))	//-这个是对功能码的判断,功能码不同判断的依据也不一样
	        {	//-这里是宁外一种处理现在可以不管
	          	//-the_ram_ax=(port_recv_dl[0]+2)&0xff;
	          	//-temp_int=port_recv[0][the_ram_ax]+2+port_recv_dl[0];
              temp_int=10+port_recv_dl[0];
	          	for(temp_loop=port_recv_dl[0];temp_loop<temp_int;temp_loop++)	//-上面这样干的秘密就是保证定位到需要处理的报文字节
	          	{	//-简单的不需要这样处理但是复杂的还是需要的,那么这样用了得话兼容性就会很好
	                 if(temp_loop<=255)
	           	       port_report[temp_loop-port_recv_dl[0]]=port_recv[0][temp_loop];
	                 else
	           	       port_report[temp_loop-port_recv_dl[0]]=port_recv[0][temp_loop-256];	//-难道是高速更新的缘故需要提前复制出来
	          	}	//-或者还有一种可能性就是统一处理
	         	//-port_recv_dl[1]+=delta_len;	//-这个地方就舍弃了这样的处理报文
            //-port_recv_dl[0]+=(port_report[2] + 2);
            port_recv_dl[0]+=(10);
	         	port_recv_dl[0]&=0xff;
	         	temp_int=MOD_TC_CRC16(&port_report[0],8);
	         	if((LOBYTE(temp_int)!=port_report[8+0])||(HIBYTE(temp_int)!=port_report[8+1]))	//-进行CRC检查
	         	{	//-由于这的不确定性,如果校验不正确的话就直接把头舍掉,继续寻找
	          		goto inrda;	//-到这里说明成功接收到的报文CRC校验没有通过
	          }
	        }
	        else if(port_recv[0][the_ram_bx]==0x10)
          {
              the_ram_ax=(port_recv_dl[0]+8)&0xff;
              temp_int=port_recv[0][the_ram_ax]+11+port_recv_dl[0];
	          	for(temp_loop=port_recv_dl[0];temp_loop<temp_int;temp_loop++)	//-上面这样干的秘密就是保证定位到需要处理的报文字节
	          	{	//-简单的不需要这样处理但是复杂的还是需要的,那么这样用了得话兼容性就会很好
	                 if(temp_loop<=255)
	           	       port_report[temp_loop-port_recv_dl[0]]=port_recv[0][temp_loop];
	                 else
	           	       port_report[temp_loop-port_recv_dl[0]]=port_recv[0][temp_loop-256];	//-难道是高速更新的缘故需要提前复制出来
	          	}	//-或者还有一种可能性就是统一处理
	         	//-port_recv_dl[1]+=delta_len;	//-这个地方就舍弃了这样的处理报文
            //-port_recv_dl[0]+=(port_report[2] + 2);
            port_recv_dl[0]+=(port_recv[0][the_ram_ax]+11);
	         	port_recv_dl[0]&=0xff;
	         	temp_int=MOD_TC_CRC16(&port_report[0],port_recv[0][the_ram_ax]+9);
	         	if((LOBYTE(temp_int)!=port_report[port_recv[0][the_ram_ax]+9+0])||(HIBYTE(temp_int)!=port_report[port_recv[0][the_ram_ax]+9+1]))	//-进行CRC检查
	         	{	//-由于这的不确定性,如果校验不正确的话就直接把头舍掉,继续寻找
	          		goto inrda;	//-到这里说明成功接收到的报文CRC校验没有通过
	          }
          }
          else
	        {
	            //-port_recv_dl[1]+=delta_len;	//-目前直接舍弃
              port_recv_dl[0]+=7;
	            port_recv_dl[0]&=0xff;
	            goto inrda;
	        }


      //-临时先赋值,后面这个变量可以处理逻辑
      //-可以组合判断下接收报文,然后决定下面走向
      //-test_cn[5]++;
    	//-正确内容的处理
////////////////////////////////////////////////////////////////////////////////
			if(port_report[3] == 0x06)	//-判断功能码
			{//-写单个线圈
        if((port_report[4] == 0x40) && (port_report[5] == 0x07))
        {//-设置制热温度
            //-menu_set_tt_min_eep = port_report[7]; //-制热的最小值,当实际温度低于这个值时就需要制热了
            //-MOD_TC_normal_respond(&port_report[4]);
            //-MOD_TC_transmit_wait_time=cticks_5ms;
        }
        else
        {
            if((port_report[4] == 0x40) && (port_report[5] == 0x08))
            {//-设置制冷温度
                //-menu_set_tt_max_eep = port_report[7]; //-制热的最小值,当实际温度低于这个值时就需要制热了
                //-MOD_TC_normal_respond(&port_report[4]);
                //-MOD_TC_transmit_wait_time=cticks_5ms;
            }
            else
            {
                if((port_report[4] == 0x40) && (port_report[5] == 0x02))
                {//-设置温度显示格式
                  if(port_report[7] <= 1)
                  {
                    menu_set_tt_CorF = port_report[7]; //-制热的最小值,当实际温度低于这个值时就需要制热了
                    MOD_TC_normal_respond(&port_report[4]);
                    MOD_TC_transmit_wait_time=cticks_5ms;
                    SET_Modify_flag = 0x55;
                  }
                }
                else
                {
                    if((port_report[4] == 0x40) && (port_report[5] == 0x03))
                    {//-设置温控器类型
                      //-if(RUN_ONOFF_status == 1)  //-by zj 2016.10.12
                      {//-必须开机才可以修改工作模式
                        SYS_WORK_MODE = port_report[7]; //-制热的最小值,当实际温度低于这个值时就需要制热了
                        MOD_TC_normal_respond(&port_report[4]);
                        if((SYS_WORK_MODE == 0x11) || (SYS_WORK_MODE == 0x21))
                        {
                          RUN_status = 1;  //-进入制热模式
                        }
                        else if((SYS_WORK_MODE == 0x12) || (SYS_WORK_MODE == 0x22))
                        {
                          RUN_status = 2;  //-进入制热模式
                        }
                        else
                        {//-原来是什么就是什么不需要强制转换
                        
                        }
                        MOD_TC_transmit_wait_time=cticks_5ms;
                        SET_Modify_flag = 0x55;
                        RUN_status_flag = 0x55; //-工作类型都修改了,变量肯定都需要重新初始化了
                      }
                    }
                    else
                    {
                        if((port_report[4] == 0x40) && (port_report[5] == 0x14))
                        {//-swing设置
                          if(port_report[7] <= 8)
                          {
                            menu_set_tt_swing_pt = port_report[7];
                            menu_set_tt_swing = swing_constant[port_report[7]]; //-制热的最小值,当实际温度低于这个值时就需要制热了
                            MOD_TC_normal_respond(&port_report[4]);
                            MOD_TC_transmit_wait_time=cticks_5ms;
                            SET_Modify_flag = 0x55;
                          }
                        }
                        else
                        {
                            if((port_report[4] == 0x40) && (port_report[5] == 0x15))
                            {//-diff设置
                              if(port_report[7] <= 5)
                              {
                                menu_set_tt_diff_pt = port_report[7];
                                menu_set_tt_diff = diff_constant[port_report[7]]; //-制热的最小值,当实际温度低于这个值时就需要制热了
                                MOD_TC_normal_respond(&port_report[4]);
                                MOD_TC_transmit_wait_time=cticks_5ms;
                                SET_Modify_flag = 0x55;
                              }
                            }
                            else
                            {
                                if((port_report[4] == 0x40) && (port_report[5] == 0x16))
                                {//-third设置
                                  if(port_report[7] <= 6)
                                  {
                                    menu_set_tt_third_pt = port_report[7];
                                    menu_set_tt_third = third_constant[port_report[7]]; //-制热的最小值,当实际温度低于这个值时就需要制热了
                                    MOD_TC_normal_respond(&port_report[4]);
                                    MOD_TC_transmit_wait_time=cticks_5ms;
                                    SET_Modify_flag = 0x55;
                                  }
                                }
                                else
                                {
                                    if((port_report[4] == 0x40) && (port_report[5] == 0x00))
                                    {//-设置开关机
                                      if(port_report[7] <= 1)   //-实践证明这样"多余"的话需要加
                                      {
                                        if(port_report[7] == 0)
                                        {
                                          menu_wind_status = 0x80;
                                          //-RUN_status = port_report[7];
                                        }
                                        //-else
                                        //-  RUN_status = BKP_ReadBackupRegister(BKP_DR4);  //-这里需要考虑
                                        RUN_ONOFF_status = port_report[7];
                                        out_hold_flag = 0;
                                        MOD_TC_normal_respond(&port_report[4]);
                                        MOD_TC_transmit_wait_time=cticks_5ms;
                                        SET_Modify_flag = 0x55;
                                      }
                                    }
                                    else
                                    {
                                        if((port_report[4] == 0x40) && (port_report[5] == 0x17))
                                        {//-设置蜂鸣器开关
                                            beep_onoff = port_report[7]; //-制热的最小值,当实际温度低于这个值时就需要制热了
                                            MOD_TC_normal_respond(&port_report[4]);
                                            MOD_TC_transmit_wait_time=cticks_5ms;
                                            SET_Modify_flag = 0x55;
                                        }
                                        else
                                        {
                                            if((port_report[4] == 0x40) && (port_report[5] == 0x18))
                                            {//-紧急制热控制
                                                //-OUT_emergency_hot = port_report[7]; //-制热的最小值,当实际温度低于这个值时就需要制热了
                                                if(port_report[7] == 0)  //-原来的模式是开的话,现在就启动准备关,但是实际关不关再说
                                                {
                                                  OUT_emergency_hot = 0;    //-紧急制热可以直接关闭,但是开启需要满足条件
                                                  E_out_flag = 0;
                                                }
                                                else
                                                  E_out_flag = 1;
                                                
                                                MOD_TC_normal_respond(&port_report[4]);
                                                MOD_TC_transmit_wait_time=cticks_5ms;
                                                SET_Modify_flag = 0x55;
                                            }
                                            else
                                            {
                                              if((port_report[4] == 0x40) && (port_report[5] == 0x01))
                                              {//-风机控制
                                                if(menu_wind_status != 0x51)
                                                {
                                                  if(port_report[7] == 2) //-制热的最小值,当实际温度低于这个值时就需要制热了
                                                    menu_wind_status = 0x81;   //-1表示打开
                                                  else
                                                    menu_wind_status = 0x80;   //-0表示自动
                                                }
                                                  MOD_TC_normal_respond(&port_report[4]);
                                                  MOD_TC_transmit_wait_time=cticks_5ms;
                                                  SET_Modify_flag = 0x55;
                                              }
                                              else
                                              {
                                                  if((port_report[4] == 0x40) && (port_report[5] == 0x0F))
                                                  {//-外部温度
                                                      temperature_data_out = port_report[6] << 8; //-制热的最小值,当实际温度低于这个值时就需要制热了                                                       
                                                      temperature_data_out += port_report[7];
                                                      MOD_TC_normal_respond(&port_report[4]);
                                                      MOD_TC_transmit_wait_time=cticks_5ms;
                                                      temp_data_out_time = cticks_s;
                                                      temp_data_out_flag = 1;
                                                  }
                                                  else
                                                  {
                                                      if((port_report[4] == 0x40) && (port_report[5] == 0x10))
                                                      {//-                                                                                                                
                                                          if(port_report[7] == 1) //-开始入网,最多闪15S
                                                          {
                                                            zigbee_flag = 1;
                                                          }
                                                          else if(port_report[7] == 2)  //-入网成功,常亮
                                                          {
                                                            zigbee_flag = 0x55;
                                                          }
                                                          else if(port_report[7] == 3)
                                                          {//-掉线,灭灯
                                                            zigbee_flag = 0;
                                                          }
                                                          else if(port_report[7] == 4)
                                                          {//-唤醒,亮屏即可
                                                            STOP_status2 = 1;
                                                            STOP_wait_time2 = 0;
                                                            STOP_wait_time = cticks_5ms;
                                                          }
                                                          else if(port_report[7] == 5)
                                                          {//-设备查找,固定闪6次
                                                            zigbee_flag = 1;
                                                          }
                                                          
                                                          MOD_TC_normal_respond(&port_report[4]);
                                                          MOD_TC_transmit_wait_time=cticks_5ms;
                                                      }
                                                      else
                                                      {
                                                          if((port_report[4] == 0x40) && (port_report[5] == 0x11))
                                                          {//-外部温度                                                                                                                
                                                              if(port_report[7] == 1)
                                                                zigbee_flag = 1;
                                                              else if(port_report[7] == 2)
                                                              {
                                                                zigbee_flag = 0;
                                                              }
                                                              MOD_TC_normal_respond(&port_report[4]);
                                                              MOD_TC_transmit_wait_time=cticks_5ms;
                                                          }
                                                          else
                                                          {
                                                              if((port_report[4] == 0x40) && (port_report[5] == 0x1C))
                                                              {//-震动开关                                                                                                                                                                                  
                                                                  Motor_onoff = port_report[7];                                                                  
                                                                  MOD_TC_normal_respond(&port_report[4]);
                                                                  MOD_TC_transmit_wait_time=cticks_5ms;
                                                                  SET_Modify_flag = 0x55;
                                                              }
                                                              else
                                                              {
                                                                  
                                                              }
                                                          }
                                                      }
                                                  }
                                              }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
					//-MOD_TC_data_ack_deal();
			}
			else
			{
				if(port_report[3] == 0x10)	//-判断功能码
	      {//-写多个寄存器
	      	 if((port_report[4] == 0x40) && (port_report[5] == 0x19))
	      	 {//-设置时间
	      	 	   //-the_ram_ax = (port_report[6] << 8) + port_report[7];
               the_ram_ax = port_report[8];
               if(the_ram_ax == 6)
               {
                  REG_year = 2000 + port_report[9];
                  REG_month = port_report[10];
                  REG_date = port_report[11];
                  REG_hour = port_report[12];
                  REG_min = port_report[13];
                  REG_sec = port_report[14];
                  //-Adjust time by values entered by the user on the hyperterminal
                  Time_Adjust();
                  MOD_TC_10normal_respond(&port_report[4]);
                  MOD_TC_transmit_wait_time=cticks_5ms;
               }
	      	 }
	      	 else
	      	 {
               temp_data = (port_report[4] << 8) + port_report[5];
               if((temp_data >= 0x4030) && (temp_data <= 0x4170))
	      	 	   //-if((port_report[4] == 0x40) && (port_report[5] == 0x20))
               {
                  temp_data1 = (temp_data - 0x4030) / 0x10;   //-得到表的偏移量
                  the_ram_ax = port_report[8];
                  for(i=0;i<the_ram_ax/4;i++)
                  {
                    programme_day[temp_data1].day_dot[i].MIN = ((port_report[9 + 4*i] << 8) + port_report[10 + 4*i]) * 15;
                    programme_day[temp_data1].day_dot[i].MIN_VOULE = port_report[11 + 4*i];
                    programme_day[temp_data1].day_dot[i].MAX_VOULE = port_report[12 + 4*i];
                  }
                  programme_day[temp_data1].SET_status = 0x55;    //-定值更新了,需要存储到EEP中
                  MOD_TC_10normal_respond(&port_report[4]);
                  MOD_TC_transmit_wait_time=cticks_5ms;
               }
               else
               {
                  if((port_report[4] == 0x40) && (port_report[5] == 0x04))
                  {//-设置工作方式
                    if(RUN_status != port_report[10])
                    {
                      if((SYS_WORK_MODE == 0x11) || (SYS_WORK_MODE == 0x21))
                      {
                        if(port_report[10] == 1)
                        {
                          RUN_status = 1;
                          RUN_status_flag = 0x55;
                        }
                      }
                      else if((SYS_WORK_MODE == 0x12) || (SYS_WORK_MODE == 0x22))
                      {
                        if(port_report[10] == 2)
                        {
                          RUN_status = 2;
                          RUN_status_flag = 0x55;
                        }
                      }
                      else
                      {
                        if(port_report[10] <= 3)
                        {
                          RUN_status = port_report[10];
                          out_hold_flag = 0;
                          RUN_status_flag = 0x55;
                        }
                      }
                    }
                    if(port_report[9] == 1)
                    {
                      if(RUN_status == 1)
                      {
                        menu_set_tt_min_eep = (port_report[11] << 8) + port_report[12];  //-制热
                        menu_set_tt_min_f = (u16)(3200 + menu_set_tt_min_eep * 1.8);
                        temp_data = menu_set_tt_min_f % 100;  //-华氏仅仅是整数部分没有小数,,//-显示修改的基础
                        if(temp_data >= 50)
                           menu_set_tt_min_f = (menu_set_tt_min_f / 100) + 1;
                        else
                           menu_set_tt_min_f = (menu_set_tt_min_f / 100);
                        
                        temp_data = menu_set_tt_min_eep % 100;
                        if(temp_data >= 50)
                          menu_set_tt_min_eep = (menu_set_tt_min_eep / 100) * 100 + 50;
                        else
                          menu_set_tt_min_eep = (menu_set_tt_min_eep / 100) * 100;  
                        
                        menu_set_tt_flag = 1;
                        STOP_wait_time = cticks_5ms;
                      }
                      else if(RUN_status == 2)
                      {
                        menu_set_tt_max_eep = (port_report[13] << 8) + port_report[14];  //-制冷
                        menu_set_tt_max_f = (u16)(3200 + menu_set_tt_max_eep * 1.8);
                        temp_data = menu_set_tt_max_f % 100;  //-华氏仅仅是整数部分没有小数,,//-显示修改的基础
                        if(temp_data >= 50)
                           menu_set_tt_max_f = (menu_set_tt_max_f / 100) + 1;
                        else
                           menu_set_tt_max_f = (menu_set_tt_max_f / 100);
                        
                        temp_data = menu_set_tt_max_eep % 100;
                        if(temp_data >= 50)
                          menu_set_tt_max_eep = (menu_set_tt_max_eep / 100) * 100 + 50;
                        else
                          menu_set_tt_max_eep = (menu_set_tt_max_eep / 100) * 100;
                        
                        menu_set_tt_flag = 1;
                        STOP_wait_time = cticks_5ms;
                      }
                      else if(RUN_status == 3)
                      {
                        auto_set_tt_min_eep = (port_report[11] << 8) + port_report[12];  //-制热
                        temp_data = auto_set_tt_min_eep % 100;
                        if(temp_data >= 50)
                          auto_set_tt_min_eep = (auto_set_tt_min_eep / 100) * 100 + 50;
                        else
                          auto_set_tt_min_eep = (auto_set_tt_min_eep / 100) * 100;
                        auto_set_tt_max_eep = (port_report[13] << 8) + port_report[14];  //-制冷
                        temp_data = auto_set_tt_max_eep % 100;
                        if(temp_data >= 50)
                          auto_set_tt_max_eep = (auto_set_tt_max_eep / 100) * 100 + 50;
                        else
                          auto_set_tt_max_eep = (auto_set_tt_max_eep / 100) * 100;
                      }
                      SET_Modify_flag = 0x55;   //-by zj 修改之后需要写Flash                      
                    }
                      MOD_TC_10normal_respond(&port_report[4]);
                      MOD_TC_transmit_wait_time=cticks_5ms;  
                      
                      //-now_programme_flag = 0x55;
                      now_programme_time = m_hour * 60 + m_min;

                  }
                  else
                  {
                  
                  }
               }
	      	 }
	      }
	      else
	      {//-接收LED点阵屏图片
          if(port_report[3] == 0x03)  //-查询
          {
            if((port_report[4] == 0x40) && (port_report[5] == 0x00))
            {
              //-MOD_TC_status_data_respond();
              MOD_TC_send_status_data();  //-为了保持一致
              MOD_TC_transmit_wait_time=cticks_5ms;
            }
            else
            {
                if((port_report[4] == 0x40) && (port_report[5] == 0x13))
                {
                  //-MOD_TC_status_data_respond();
                  MOD_TC_send_status_data2();  //-为了保持一致
                  MOD_TC_transmit_wait_time=cticks_5ms;
                }
                else
                {
                  
                }
            }
          }
          else
          {

          }
	      }
      }

////////////////////////////////////////////////////////////////////////////////
         STOP_status2 = 1; //-点阵屏显示
         STOP_wait_time2 = 0;
         STOP_wait_time = cticks_5ms;
    	   MOD_TC_clear_port_report_deal(delta_len);
      }
rxd_out_time:	//-执行到这里说明接收超时,或处理成功,反正现在可以继续发送了,,正常情况都会执行这里的,首先
	    if(Judge_Time_In_MainLoop(MOD_TC_rxd_wait_time,MOD_TC_WAIT_TIME_VALUE)==YES)	//-正常通信的话是不应该超时的,若过时就认为出错,下面重新初始化
	    {	//-当发送一个报文之后,超过等待回答时间之后,就可以从新发送一次
	      	MOD_TC_rec_OK=NO;
	      	MOD_TC_rxd_head_flag=NO;
	      	MOD_TC_rxd_wait_time=cticks_5ms;
	      	MOD_TC_transmit_flag=YES;	//-表示现在可以组织发送内容
	      	//-MOD_TC_wait_replay=NO;	//-表示现在还没有等待回复
	      	//-MOD_TC_transmit_wait_time=Time_1ms_Counter;

	       	//-MOD_TC_comm_err_counter[port_send[0][0]]++;	//-超时出错计数


	    }
inrda:
		   //-主动发送 ,,这里就有一个问题,多长时间发送一次,或者说这个发送触发条件是什么
		   if((UART1_transmit_flag==YES) && (MOD_TC_transmit_flag==YES))		//-目前无所谓的是双向的
		   {
          if(Judge_Time_In_MainLoop(MOD_TC_transmit_wait_time,100)==YES)	//-只有过了等待时间才会发送,这里时间主要是为了给接收端足够的处理时间
			   	  switch(UART1_transmit_control)
	          {
	                case 1:   //-入网
	                     //MOD_TC_rec_frame_type=MOD_TC_YX_FRAME;	//-表示需要接收的帧的类型是这个值,接收到这样的值才是正确的
	                     //MOD_TC_wait_replay=YES;
	                     MOD_TC_ask_join_cmd();	//-本规约定义的内容是,问答型的,这个端口上的所有装置都是接收查询
	                     MOD_TC_transmit_wait_time=cticks_5ms;

	                     UART1_transmit_flag = NO;		//-这里复杂了就不等待应答了直接发以便结束
    									 UART1_transmit_control = 0;

	                     break;
	                case 2:   //-退网
	                     //MOD_TC_rec_frame_type=MOD_TC_YX_FRAME;	//-表示需要接收的帧的类型是这个值,接收到这样的值才是正确的
	                     //MOD_TC_wait_replay=YES;
	                     MOD_TC_ask_move_cmd();	//-本规约定义的内容是,问答型的,这个端口上的所有装置都是接收查询
	                     MOD_TC_transmit_wait_time=cticks_5ms;

	                     UART1_transmit_flag = NO;		//-这里复杂了就不等待应答了直接发以便结束
    									 UART1_transmit_control = 0;

	                     break;
                   case 3:   //-请求编程值
	                     //MOD_TC_rec_frame_type=MOD_TC_YX_FRAME;	//-表示需要接收的帧的类型是这个值,接收到这样的值才是正确的
	                     //MOD_TC_wait_replay=YES;
	                     MOD_TC_ask_program_cmd();	//-本规约定义的内容是,问答型的,这个端口上的所有装置都是接收查询
	                     MOD_TC_transmit_wait_time=cticks_5ms;

	                     UART1_transmit_flag = NO;		//-这里复杂了就不等待应答了直接发以便结束
    									 UART1_transmit_control = 0;

	                     break;
                   case 4:   //-主动发送本终端状态值1
	                     //MOD_TC_rec_frame_type=MOD_TC_YX_FRAME;	//-表示需要接收的帧的类型是这个值,接收到这样的值才是正确的
	                     //MOD_TC_wait_replay=YES;
	                     MOD_TC_send_status_data();	//-本规约定义的内容是,问答型的,这个端口上的所有装置都是接收查询
	                     MOD_TC_transmit_wait_time=cticks_5ms;

	                     UART1_transmit_flag = NO;		//-这里复杂了就不等待应答了直接发以便结束
    									 UART1_transmit_control = 0;

	                     break;
                   case 5:   //-主动发送本终端状态值2
	                     //MOD_TC_rec_frame_type=MOD_TC_YX_FRAME;	//-表示需要接收的帧的类型是这个值,接收到这样的值才是正确的
	                     //MOD_TC_wait_replay=YES;
	                     MOD_TC_send_status_data2();	//-本规约定义的内容是,问答型的,这个端口上的所有装置都是接收查询
	                     MOD_TC_transmit_wait_time=cticks_5ms;

	                     UART1_transmit_flag = NO;		//-这里复杂了就不等待应答了直接发以便结束
    									 UART1_transmit_control = 0;

	                     break; 
                   case 6:   //-主动发送本终端状态值2
	                     //MOD_TC_rec_frame_type=MOD_TC_YX_FRAME;	//-表示需要接收的帧的类型是这个值,接收到这样的值才是正确的
	                     //MOD_TC_wait_replay=YES;
	                     MOD_TC_send_ASK_data();	//-本规约定义的内容是,问答型的,这个端口上的所有装置都是接收查询
	                     MOD_TC_transmit_wait_time=cticks_5ms;
                       

	                     UART1_transmit_flag = NO;		//-这里复杂了就不等待应答了直接发以便结束
    									 UART1_transmit_control = 0;

	                     break;   
                    case 7:   //-主动发送本终端状态值2
	                     //MOD_TC_rec_frame_type=MOD_TC_YX_FRAME;	//-表示需要接收的帧的类型是这个值,接收到这样的值才是正确的
	                     //MOD_TC_wait_replay=YES;
	                     MOD_TC_send_power_state();	//-本规约定义的内容是,问答型的,这个端口上的所有装置都是接收查询
	                     MOD_TC_transmit_wait_time=cticks_5ms;
                       

	                     UART1_transmit_flag = NO;		//-这里复杂了就不等待应答了直接发以便结束
    									 UART1_transmit_control = 0;

	                     break;   
	                default:
	                     break;
	          }


		   }
 	 }

}


