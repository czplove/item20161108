/*
����:ʹ��һ�������߼�,����һ��512�ֽڵĻ�����,Ȼ��ѭ���洢
����:Ҳʹ��512�ֽڵĿռ䱣������,Ȼ����һ�����ͱ�־,ֱ�����ݷ��ͳ�ȥΪֹ

����3 ��Ϊ��ư��7620֮��ͨѶ͸����,���ھ����������κδ���,����˫��͸��
7620��������Ҫ���ݵ���ư�Ķ���,ֱ���ô���3����
STM32����������,ֱ���ڴ���3�д���,�������κ�����,�����ñ�־λ�ô���2�Լ���֯������7620����

2016/4/9 15:45:27
1.���������¶�
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


//-����ĳ������,ÿ��������һ����Ա0,�����Ϳ��Զ�Ӧ��1��ʼ
BYTE swing_constant[] = {0,2,5,8,10,12,15,18,20};
BYTE diff_constant[] = {0,10,15,20,25,30};
BYTE third_constant[] = {0,25,30,35,40,45,50};

//-ΪЭ�鴦����ʱ����ı���
BYTE  MOD_TC_rxd_head_flag;
BYTE  MOD_TC_rec_OK;
BYTE  MOD_TC_rec_frame_type;
WORD  MOD_TC_rxd_wait_time;
BYTE  MOD_TC_transmit_flag;
BYTE  MOD_TC_transmit_control;
WORD  MOD_TC_transmit_wait_time;



////////////////////////////////////////////////////////////////////////////////
/*
����1:��ѯ�������豸��ID��
7620���ͣ�
0xaa 0x55 0x05 0x02 0x01 CRC
STM32��Ӧ��
0xaa 0x55 0x0b 0x02 0x01 0x01 0x02 0x03 0x04 0x05 0x06 CRC






*/
unsigned int MOD_TC_CRC16(unsigned char *MOD_TC_CRC16_start,unsigned char MOD_TC_CRC16_bytes)    //*xΪָ��ÿ��ǰ5�����ݵ�ָ��
{	//-��������У��CRC
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

	  port_send[0][0] = 0xaa;		//-���ֽڰ�ͷ
	  port_send[0][1] = 0x55;

	  port_send[0][2] = 0x01;		//-�������� 1���ֽڣ��Ӱ�ͷ��ʼ����CRC16֮ǰ�����ݡ�

    port_send[0][3] = 0x06;
	  port_send[0][4] = temp_data[0];		//-������:01h �����������ϱ�

	  //-��Ч����
	  //-�������Ч���������Կ�Ϊ��λ��"���ݳ���+�豸ID+������+��������+����λ"
	  port_send[0][5] = temp_data[1];		//-���ݳ���
    port_send[0][6] = temp_data[2];
    port_send[0][7] = temp_data[3];

	  //-CRC16
	  the_ram_ax=MOD_TC_CRC16(&port_send[0][0],8);
	  port_send[0][8] =LOBYTE(the_ram_ax);
	  port_send[0][9] =HIBYTE(the_ram_ax);	//-ֱ���������еı������ݶ��Ѿ�׼������

	  //-���ͳ���
	  port_send_len[0] = 10;
	  port_deal_flag[0] = 0xaa;

	  //-cticks_500ms = 0;
}

void MOD_TC_10normal_respond(BYTE *data_pt)
{
    WORD the_ram_ax;

	  port_send[0][0] = 0xaa;		//-���ֽڰ�ͷ
	  port_send[0][1] = 0x55;

	  port_send[0][2] = 0x01;		//-�������� 1���ֽڣ��Ӱ�ͷ��ʼ����CRC16֮ǰ�����ݡ�

    port_send[0][3] = 0x10;

	  port_send[0][4] = data_pt[0];		//-������:01h �����������ϱ�

	  //-��Ч����
	  //-�������Ч���������Կ�Ϊ��λ��"���ݳ���+�豸ID+������+��������+����λ"
	  port_send[0][5] = data_pt[1];		//-���ݳ���

    port_send[0][6] = data_pt[2];
    port_send[0][7] = data_pt[3];

	  //-CRC16
	  the_ram_ax=MOD_TC_CRC16(&port_send[0][0],8);
	  port_send[0][8] =LOBYTE(the_ram_ax);
	  port_send[0][9] =HIBYTE(the_ram_ax);	//-ֱ���������еı������ݶ��Ѿ�׼������

	  //-���ͳ���
	  port_send_len[0] = 10;
	  port_deal_flag[0] = 0xaa;

	  //-cticks_500ms = 0;
}


//-�������������͵ı���
void MOD_TC_ask_join_cmd(void)		//-���յ���ѯ����״̬����֮��,ֱ�ӷ���״ֵ̬
{
	  WORD the_ram_ax;

	  //-������ݳɹ����յ�֮���Ӧ��һ��
			  port_send[0][0] = 0xaa;		//-���ֽڰ�ͷ
			  port_send[0][1] = 0x55;

			  port_send[0][2] = 0x01;		//-�������� 1���ֽڣ��Ӱ�ͷ��ʼ����CRC16֮ǰ�����ݡ�

			  port_send[0][3] = 0x41;		//-������:01h �����������ϱ�

			  //-��Ч����
		    port_send[0][4] = 0x40;		//-���ݳ���

			  port_send[0][5] = 0x00;		//-���ݳ���
			  port_send[0][6] = 0x00;		//-���ݳ���
			  port_send[0][7] = 0x01;

			  //-CRC16
			  the_ram_ax=MOD_TC_CRC16(&port_send[0][0],8);
			  port_send[0][8] =LOBYTE(the_ram_ax);
			  port_send[0][9] =HIBYTE(the_ram_ax);	//-ֱ���������еı������ݶ��Ѿ�׼������

			  //-���ͳ���
			  port_send_len[0] = 8 + 2;
			  port_deal_flag[0] = 0xaa;

}

void MOD_TC_ask_move_cmd(void)		//-���յ���ѯ����״̬����֮��,ֱ�ӷ���״ֵ̬
{
	  WORD the_ram_ax;

	  //-������ݳɹ����յ�֮���Ӧ��һ��
			  port_send[0][0] = 0xaa;		//-���ֽڰ�ͷ
			  port_send[0][1] = 0x55;

			  port_send[0][2] = 0x01;		//-�������� 1���ֽڣ��Ӱ�ͷ��ʼ����CRC16֮ǰ�����ݡ�

			  port_send[0][3] = 0x42;		//-������:01h �����������ϱ�

			  //-��Ч����
		    port_send[0][4] = 0x40;		//-���ݳ���

			  port_send[0][5] = 0x01;		//-���ݳ���
			  port_send[0][6] = 0x00;		//-���ݳ���
			  port_send[0][7] = 0x01;

			  //-CRC16
			  the_ram_ax=MOD_TC_CRC16(&port_send[0][0],8);
			  port_send[0][8] =LOBYTE(the_ram_ax);
			  port_send[0][9] =HIBYTE(the_ram_ax);	//-ֱ���������еı������ݶ��Ѿ�׼������

			  //-���ͳ���
			  port_send_len[0] = 8 + 2;
			  port_deal_flag[0] = 0xaa;

}

void MOD_TC_ask_program_cmd(void)		//-���յ���ѯ����״̬����֮��,ֱ�ӷ���״ֵ̬
{
	  WORD the_ram_ax;

	  //-������ݳɹ����յ�֮���Ӧ��һ��
			  port_send[0][0] = 0xaa;		//-���ֽڰ�ͷ
			  port_send[0][1] = 0x55;

			  port_send[0][2] = 0x01;		//-�������� 1���ֽڣ��Ӱ�ͷ��ʼ����CRC16֮ǰ�����ݡ�

			  port_send[0][3] = 0x43;		//-������:01h �����������ϱ�

			  //-��Ч����
		    port_send[0][4] = 0x40;		//-���ݳ���

			  port_send[0][5] = 0x20;		//-���ݳ���
			  port_send[0][6] = 0x00;		//-���ݳ���
			  port_send[0][7] = 0x54;

			  //-CRC16
			  the_ram_ax=MOD_TC_CRC16(&port_send[0][0],8);
			  port_send[0][8] =LOBYTE(the_ram_ax);
			  port_send[0][9] =HIBYTE(the_ram_ax);	//-ֱ���������еı������ݶ��Ѿ�׼������

			  //-���ͳ���
			  port_send_len[0] = 8 + 2;
			  port_deal_flag[0] = 0xaa;

}

void MOD_TC_send_status_data(void)		//-��������״̬�仯ֵ
{
	  WORD the_ram_ax;

	  //-������ݳɹ����յ�֮���Ӧ��һ��
			  port_send[0][0] = 0xaa;		//-���ֽڰ�ͷ
			  port_send[0][1] = 0x55;

			  port_send[0][2] = 0x01;		//-�������� 1���ֽڣ��Ӱ�ͷ��ʼ����CRC16֮ǰ�����ݡ�

			  port_send[0][3] = 0x43;		//-������:01h �����������ϱ�

			  //-��Ч����
		    port_send[0][4] = 0x18;		//-�ֽ��� = 2 * Zigbee��д�Ĵ�������

        //-����״̬,MM->00:�رգ�MM->01����
			  port_send[0][5] = 0x00;		//-����Hi
        //-if(RUN_status == 0)
        //-  port_send[0][6] = 0x00;		//-����Li
        //-else
          port_send[0][6] = RUN_ONOFF_status;		//-����Li
        
        //-���ģʽ,MM->01�Զ���MM->02����
        port_send[0][7] = 0x00;
        if((menu_wind_status & 0x01) == 1)
          port_send[0][8] = 0x02;
        else
          port_send[0][8] = 0x01;
        
        //-�±�
        port_send[0][9] = 0x00;        
        port_send[0][10] = menu_set_tt_CorF;
        
        //-�豸����
        port_send[0][11] = 0x00;        
        port_send[0][12] = SYS_WORK_MODE;
        
        //-����ģʽ,MM->01���ȣ�MM->02�����䣬MM->03���Զ�
			  port_send[0][13] = 0x00;
        port_send[0][14] = RUN_status;
        
        //-�¶�
        port_send[0][15] = menu_set_tt_min_eep >> 8;   //-����
        port_send[0][16] = menu_set_tt_min_eep & 0xff;   //-����

        if(temperature_data_flag)
          the_ram_ax = 0 - port_send_sense_data[2];
        else
          the_ram_ax = port_send_sense_data[2];
        port_send[0][17] = the_ram_ax >> 8;
        port_send[0][18] = the_ram_ax;

        //-ʪ��
        port_send[0][19] = 0;
        port_send[0][20] = humidity_data & 0xff;

        //-�Զ����� 
        port_send[0][21] = auto_set_tt_min_eep >> 8;   //-����
        port_send[0][22] = auto_set_tt_min_eep & 0xff;   //-���� 

        //-����
        port_send[0][23] = 0;
        port_send[0][24] = SYS_LOCK_FLAG;
        
        //-�����¶�
        port_send[0][25] = menu_set_tt_max_eep >> 8;   //-����
        port_send[0][26] = menu_set_tt_max_eep & 0xff;   //-����
        
        //-�Զ������¶�
        port_send[0][27] = auto_set_tt_max_eep >> 8;   //-����
        port_send[0][28] = auto_set_tt_max_eep & 0xff;   //-����

        if(now_programme_flag == 1)
        {
            if(RUN_status == 1)
            {//-����
              the_ram_ax = menu_set_tt_min * 10;
              port_send[0][15] = the_ram_ax >> 8;   //-����
              port_send[0][16] = the_ram_ax & 0xff;   //-����
            }
            else if(RUN_status == 2)
            {//-����
              the_ram_ax = menu_set_tt_max * 10;
              port_send[0][25] = the_ram_ax >> 8;   //-����
              port_send[0][26] = the_ram_ax & 0xff;   //-����
            }
            else if(RUN_status == 3)
            {//-�Զ�
              the_ram_ax = menu_set_tt_min * 10;
              port_send[0][21] = the_ram_ax >> 8;   //-����
              port_send[0][22] = the_ram_ax & 0xff;   //-���� 
              the_ram_ax = menu_set_tt_max * 10;
              port_send[0][27] = the_ram_ax >> 8;   //-����
              port_send[0][28] = the_ram_ax & 0xff;   //-����
            }
        }
        
			  //-CRC16
			  the_ram_ax=MOD_TC_CRC16(&port_send[0][0],29);
			  port_send[0][29] =LOBYTE(the_ram_ax);
			  port_send[0][30] =HIBYTE(the_ram_ax);	//-ֱ���������еı������ݶ��Ѿ�׼������

			  //-���ͳ���
			  port_send_len[0] = 29 + 2;
			  port_deal_flag[0] = 0xaa;

}

void MOD_TC_send_status_data2(void)		//-��������״̬�仯ֵ
{
	  WORD the_ram_ax;

	  //-������ݳɹ����յ�֮���Ӧ��һ��
			  port_send[0][0] = 0xaa;		//-���ֽڰ�ͷ
			  port_send[0][1] = 0x55;

			  port_send[0][2] = 0x01;		//-�������� 1���ֽڣ��Ӱ�ͷ��ʼ����CRC16֮ǰ�����ݡ�

			  port_send[0][3] = 0x44;		//-������:01h �����������ϱ�

			  //-��Ч����
		    port_send[0][4] = 0x12;		//-�ֽ��� = 2 * Zigbee��д�Ĵ�������

        //-�¶�У׼
			  port_send[0][5] = 0x00;		//-����Hi
        port_send[0][6] = menu_set_tt_cal;		//-����Li
        
        //-
        port_send[0][7] = 0x00;
        port_send[0][8] = menu_set_tt_swing_pt;
        
        //-
        port_send[0][9] = 0x00;        
        port_send[0][10] = menu_set_tt_diff_pt;
        
        //-
        port_send[0][11] = 0x00;        
        port_send[0][12] = menu_set_tt_third_pt;
        
        //-��Ч����
			  port_send[0][13] = 0x00;
        port_send[0][14] = beep_onoff;
        
        //-��������
        port_send[0][15] = 0;   
        port_send[0][16] = OUT_emergency_hot;   //-

        port_send[0][17] = m_year - 2000;
        port_send[0][18] = m_month;
        port_send[0][19] = m_date;
        port_send[0][20] = m_hour;
        port_send[0][21] = m_min;
        port_send[0][22] = m_sec;
        
        //-��
        port_send[0][23] = 0;   
        port_send[0][24] = Motor_onoff;   //-

			  //-CRC16
			  the_ram_ax=MOD_TC_CRC16(&port_send[0][0],25);
			  port_send[0][25] =LOBYTE(the_ram_ax);
			  port_send[0][26] =HIBYTE(the_ram_ax);	//-ֱ���������еı������ݶ��Ѿ�׼������

			  //-���ͳ���
			  port_send_len[0] = 25 + 2;
			  port_deal_flag[0] = 0xaa;

}

void MOD_TC_send_ASK_data(void)		//-��������״̬�仯ֵ
{
	  WORD the_ram_ax;

	  //-������ݳɹ����յ�֮���Ӧ��һ��
			  port_send[0][0] = 0xaa;		//-���ֽڰ�ͷ
			  port_send[0][1] = 0x55;

			  port_send[0][2] = 0x01;		//-�������� 1���ֽڣ��Ӱ�ͷ��ʼ����CRC16֮ǰ�����ݡ�

			  port_send[0][3] = 0x45;		//-������:01h �����������ϱ�

			  //-��Ч����
		    port_send[0][4] = 0x40;		//-�ֽ��� = 2 * Zigbee��д�Ĵ�������
			  port_send[0][5] = 0x0F;		//-����Hi
        
        port_send[0][6] = 0x00;		//-����Li
        port_send[0][7] = 0x01;        

			  //-CRC16
			  the_ram_ax=MOD_TC_CRC16(&port_send[0][0],8);
			  port_send[0][8] =LOBYTE(the_ram_ax);
			  port_send[0][9] =HIBYTE(the_ram_ax);	//-ֱ���������еı������ݶ��Ѿ�׼������

			  //-���ͳ���
			  port_send_len[0] = 10;
			  port_deal_flag[0] = 0xaa;

}

void MOD_TC_send_power_state(void)		//-��������״̬�仯ֵ
{
	  WORD the_ram_ax;

	  //-������ݳɹ����յ�֮���Ӧ��һ��
			  port_send[0][0] = 0xaa;		//-���ֽڰ�ͷ
			  port_send[0][1] = 0x55;

			  port_send[0][2] = 0x01;		//-�������� 1���ֽڣ��Ӱ�ͷ��ʼ����CRC16֮ǰ�����ݡ�

			  port_send[0][3] = 0x46;		//-������:01h �����������ϱ�

			  //-��Ч����
		    port_send[0][4] = 0x40;		//-�ֽ��� = 2 * Zigbee��д�Ĵ�������
			  port_send[0][5] = 0x0E;		//-����Hi
        
        port_send[0][6] = 0x00;		//-����Li
        if(SYS_power_FLAG == 0)
          port_send[0][7] = 0x00; 
        else
          port_send[0][7] = 0x01;

			  //-CRC16
			  the_ram_ax=MOD_TC_CRC16(&port_send[0][0],8);
			  port_send[0][8] =LOBYTE(the_ram_ax);
			  port_send[0][9] =HIBYTE(the_ram_ax);	//-ֱ���������еı������ݶ��Ѿ�׼������

			  //-���ͳ���
			  port_send_len[0] = 10;
			  port_deal_flag[0] = 0xaa;

}

////////////////////////////////////////////////////////////////////////////////
//-������Ҫ����ʵ��Ҫ����������,�������ModbusЭ��,Ȼ����Ҫ����Ϊ�ӻ���,ĳЩ���
//-Ҳ��Ϊ����,������΢����

void uart1_Main(void)
{
   WORD the_ram_ax,the_ram_bx,i;
	 WORD temp_data,temp_data1,delta_len,temp_loop,temp_int;


	 //-�жϷ����Ƿ������,��������˿��������߼�����,���򲻴���
	 if(DMA_GetFlagStatus(DMA1_FLAG_TC4))		//-����˵�����������
   {//-ֻ�е���������˲ſ������¸�DMA���ʹ�������
   	  //-���ݷ������֮��ȫ������,�������ڲ�֪��ÿ���Ŀռ��С��,��ôѡȡһ��������
   	  //-for(temp_loop = 0;temp_loop < 43;temp_loop++)
   	  //-	port_send[0][temp_loop] = 0;
   	  port_deal_flag[0] = 0;		//-��һ����0ֵ�Ϳ������´μ�鷢��ʱ������������
   	  //-MOD_TC_transmit_flag=YES;		//-���ҵ����ݷ��ͳ�ȥ��Ҳ������������������??����Ӧ�ò���������֯���ݵķ���,������������ݻᱻ����,���յı�־λ
   	  DMA_ClearFlag(DMA1_FLAG_TC4);
   }

   if((port_deal_flag[0] == 0) /*&& (UART3_TO_UART2_FLAG == 0)*/)	//-����᲻������һ�η���ʧ�ܶ�������Ҳ���ܷ���,,����UART3_TO_UART2_FLAGΪ0��ʼ������յ��ı���,��ֹ,������û�з��ͳ�ȥ������,��ɵĶ�ʧ
   {
   	   //-���ȴ�����յ�������,���û�����ݽ��յ�,����֯������Ҫ���͵�����
   	   if(port_recv_pt[0]!=port_recv_dl[0])
   	   {

   	   	   if(MOD_TC_rxd_head_flag==NO)	//-���յ������ݻ�û�д����ʱ�����NO
           {//-Ѱ�ҵ�ͷ0xaa 0x55
           	   MOD_TC_rxd_wait_time=cticks_5ms;	//-������Ϊ�˽�������
		   	   	   temp_data = port_recv_pt[0];
		   	   	   temp_data1 = port_recv_dl[0];
		           if(temp_data1>temp_data)	//-ǰ����Ǵ���ָ���ʵ�ʵĽ���ָ����бȽ�
		               delta_len=(temp_data+256)-temp_data1;
		           else
		               delta_len=temp_data-temp_data1;	//-һ���ĳ���
		           for(temp_loop=temp_data1;temp_loop<(delta_len+temp_data1);temp_loop++)
		           {
		        	   if(port_recv[0][port_recv_dl[0]]==0xaa)	//-����ط��Ƚϵ��Ǵ�վ��ַ,�����Ҿ���û���κι��ɾ���ͨѶ
		        	   {	//-����һ�п������õ�
		        	     the_ram_ax=(port_recv_dl[0]+1)&0xff;
		        	     if(temp_data == the_ram_ax)	//-���Ԥȡ��ָ��ָ��յĿռ�,˵�����ݻ�û�е�,��Ҫ�ȴ�
		        	     	 break;
		        	     if(port_recv[0][the_ram_ax]==0x55)	//-�Ƚϵ��ǹ�����
		        	     {
		        	         MOD_TC_rxd_head_flag=YES;	//-��ʾ�Ѿ��ɹ�ʶ����յ����±��ĵ�ͷ��
		        	         break;
		        	     }
		        	   }
		        	   port_recv_dl[0]++;	//-����һ���ֵı���
		        	   port_recv_dl[0]&=0xff;
		           }
   	   	   }
   	   	   if(MOD_TC_rxd_head_flag==YES)	//-���յ������ݻ�û�д����ʱ�����NO
       		 {
       		 	   temp_data = port_recv_pt[0];
       		 	   if(port_recv_dl[0]>temp_data)
               		delta_len=(temp_data+256)-port_recv_dl[0];
               else
               		delta_len=temp_data-port_recv_dl[0];

               if(delta_len>=6)	//-���ٻ���4���ֽڲ�����֯һ������
		           {//-��Ҫ��֤�����˹�����
		               temp_int=(port_recv_dl[0]+3)&0xff;
		               //-if(delta_len>=(unsigned short)(port_recv[0][temp_int]+2))	//-�õ��ı��ĳ��Ⱥ������ϵı��ĳ��Ƚ��бȽ�
		               if((port_recv[0][temp_int]==0x03) || (port_recv[0][temp_int]==0x06))
                   {
                     if(delta_len>=10)
                     {//-�������Ѿ�ȷ�����������
		                  MOD_TC_rxd_head_flag=NO;
		                  MOD_TC_rec_OK=YES;
                      goto rec_ok_deal;	//-�������ؿ���,���������Ϊ�ǳɹ����յ�һ�����ر�����
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
                        goto rec_ok_deal;	//-�������ؿ���,���������Ϊ�ǳɹ����յ�һ�����ر�����
                      }
                   }
                   else
                   {

                   }

		           }

     			 }
   	   }
   	   goto rxd_out_time;		//?����������Ǽ򵥵�����ģʽ,����ֱ�Ӳ�ѯ����
rec_ok_deal:
	    if(MOD_TC_rec_OK==YES)	//-�϶��Ƿ�ֹ�ܷɵ�.
	    {	//-������Ϳ���˵��Ӧ�����Ѿ�������
	        MOD_TC_rec_OK=NO;	//-�ɹ����յ������ݿ�ʼ������֮��,�ͻָ�0
	        MOD_TC_transmit_flag=YES;
	        //-MOD_TC_transmit_wait_time=Time_1ms_Counter;	//-��Ȼ����˵���Է���������,���ǻ���Ҫ��ʱһ��ʱ��,��Ϊ���յ���������Ҫ����
	        //-�����Ƕ�ʵ�����ݵĴ���,������Ǻ��Ĳ���
	        the_ram_bx=(port_recv_dl[0]+3)&0xff;;
	        if((port_recv[0][the_ram_bx]==0x03) || (port_recv[0][the_ram_bx]==0x06))	//-����ǶԹ�������ж�,�����벻ͬ�жϵ�����Ҳ��һ��
	        {	//-����������һ�ִ������ڿ��Բ���
	          	//-the_ram_ax=(port_recv_dl[0]+2)&0xff;
	          	//-temp_int=port_recv[0][the_ram_ax]+2+port_recv_dl[0];
              temp_int=10+port_recv_dl[0];
	          	for(temp_loop=port_recv_dl[0];temp_loop<temp_int;temp_loop++)	//-���������ɵ����ܾ��Ǳ�֤��λ����Ҫ����ı����ֽ�
	          	{	//-�򵥵Ĳ���Ҫ���������Ǹ��ӵĻ�����Ҫ��,��ô�������˵û������Ծͻ�ܺ�
	                 if(temp_loop<=255)
	           	       port_report[temp_loop-port_recv_dl[0]]=port_recv[0][temp_loop];
	                 else
	           	       port_report[temp_loop-port_recv_dl[0]]=port_recv[0][temp_loop-256];	//-�ѵ��Ǹ��ٸ��µ�Ե����Ҫ��ǰ���Ƴ���
	          	}	//-���߻���һ�ֿ����Ծ���ͳһ����
	         	//-port_recv_dl[1]+=delta_len;	//-����ط��������������Ĵ�����
            //-port_recv_dl[0]+=(port_report[2] + 2);
            port_recv_dl[0]+=(10);
	         	port_recv_dl[0]&=0xff;
	         	temp_int=MOD_TC_CRC16(&port_report[0],8);
	         	if((LOBYTE(temp_int)!=port_report[8+0])||(HIBYTE(temp_int)!=port_report[8+1]))	//-����CRC���
	         	{	//-������Ĳ�ȷ����,���У�鲻��ȷ�Ļ���ֱ�Ӱ�ͷ���,����Ѱ��
	          		goto inrda;	//-������˵���ɹ����յ��ı���CRCУ��û��ͨ��
	          }
	        }
	        else if(port_recv[0][the_ram_bx]==0x10)
          {
              the_ram_ax=(port_recv_dl[0]+8)&0xff;
              temp_int=port_recv[0][the_ram_ax]+11+port_recv_dl[0];
	          	for(temp_loop=port_recv_dl[0];temp_loop<temp_int;temp_loop++)	//-���������ɵ����ܾ��Ǳ�֤��λ����Ҫ����ı����ֽ�
	          	{	//-�򵥵Ĳ���Ҫ���������Ǹ��ӵĻ�����Ҫ��,��ô�������˵û������Ծͻ�ܺ�
	                 if(temp_loop<=255)
	           	       port_report[temp_loop-port_recv_dl[0]]=port_recv[0][temp_loop];
	                 else
	           	       port_report[temp_loop-port_recv_dl[0]]=port_recv[0][temp_loop-256];	//-�ѵ��Ǹ��ٸ��µ�Ե����Ҫ��ǰ���Ƴ���
	          	}	//-���߻���һ�ֿ����Ծ���ͳһ����
	         	//-port_recv_dl[1]+=delta_len;	//-����ط��������������Ĵ�����
            //-port_recv_dl[0]+=(port_report[2] + 2);
            port_recv_dl[0]+=(port_recv[0][the_ram_ax]+11);
	         	port_recv_dl[0]&=0xff;
	         	temp_int=MOD_TC_CRC16(&port_report[0],port_recv[0][the_ram_ax]+9);
	         	if((LOBYTE(temp_int)!=port_report[port_recv[0][the_ram_ax]+9+0])||(HIBYTE(temp_int)!=port_report[port_recv[0][the_ram_ax]+9+1]))	//-����CRC���
	         	{	//-������Ĳ�ȷ����,���У�鲻��ȷ�Ļ���ֱ�Ӱ�ͷ���,����Ѱ��
	          		goto inrda;	//-������˵���ɹ����յ��ı���CRCУ��û��ͨ��
	          }
          }
          else
	        {
	            //-port_recv_dl[1]+=delta_len;	//-Ŀǰֱ������
              port_recv_dl[0]+=7;
	            port_recv_dl[0]&=0xff;
	            goto inrda;
	        }


      //-��ʱ�ȸ�ֵ,��������������Դ����߼�
      //-��������ж��½��ձ���,Ȼ�������������
      //-test_cn[5]++;
    	//-��ȷ���ݵĴ���
////////////////////////////////////////////////////////////////////////////////
			if(port_report[3] == 0x06)	//-�жϹ�����
			{//-д������Ȧ
        if((port_report[4] == 0x40) && (port_report[5] == 0x07))
        {//-���������¶�
            //-menu_set_tt_min_eep = port_report[7]; //-���ȵ���Сֵ,��ʵ���¶ȵ������ֵʱ����Ҫ������
            //-MOD_TC_normal_respond(&port_report[4]);
            //-MOD_TC_transmit_wait_time=cticks_5ms;
        }
        else
        {
            if((port_report[4] == 0x40) && (port_report[5] == 0x08))
            {//-���������¶�
                //-menu_set_tt_max_eep = port_report[7]; //-���ȵ���Сֵ,��ʵ���¶ȵ������ֵʱ����Ҫ������
                //-MOD_TC_normal_respond(&port_report[4]);
                //-MOD_TC_transmit_wait_time=cticks_5ms;
            }
            else
            {
                if((port_report[4] == 0x40) && (port_report[5] == 0x02))
                {//-�����¶���ʾ��ʽ
                  if(port_report[7] <= 1)
                  {
                    menu_set_tt_CorF = port_report[7]; //-���ȵ���Сֵ,��ʵ���¶ȵ������ֵʱ����Ҫ������
                    MOD_TC_normal_respond(&port_report[4]);
                    MOD_TC_transmit_wait_time=cticks_5ms;
                    SET_Modify_flag = 0x55;
                  }
                }
                else
                {
                    if((port_report[4] == 0x40) && (port_report[5] == 0x03))
                    {//-�����¿�������
                      //-if(RUN_ONOFF_status == 1)  //-by zj 2016.10.12
                      {//-���뿪���ſ����޸Ĺ���ģʽ
                        SYS_WORK_MODE = port_report[7]; //-���ȵ���Сֵ,��ʵ���¶ȵ������ֵʱ����Ҫ������
                        MOD_TC_normal_respond(&port_report[4]);
                        if((SYS_WORK_MODE == 0x11) || (SYS_WORK_MODE == 0x21))
                        {
                          RUN_status = 1;  //-��������ģʽ
                        }
                        else if((SYS_WORK_MODE == 0x12) || (SYS_WORK_MODE == 0x22))
                        {
                          RUN_status = 2;  //-��������ģʽ
                        }
                        else
                        {//-ԭ����ʲô����ʲô����Ҫǿ��ת��
                        
                        }
                        MOD_TC_transmit_wait_time=cticks_5ms;
                        SET_Modify_flag = 0x55;
                        RUN_status_flag = 0x55; //-�������Ͷ��޸���,�����϶�����Ҫ���³�ʼ����
                      }
                    }
                    else
                    {
                        if((port_report[4] == 0x40) && (port_report[5] == 0x14))
                        {//-swing����
                          if(port_report[7] <= 8)
                          {
                            menu_set_tt_swing_pt = port_report[7];
                            menu_set_tt_swing = swing_constant[port_report[7]]; //-���ȵ���Сֵ,��ʵ���¶ȵ������ֵʱ����Ҫ������
                            MOD_TC_normal_respond(&port_report[4]);
                            MOD_TC_transmit_wait_time=cticks_5ms;
                            SET_Modify_flag = 0x55;
                          }
                        }
                        else
                        {
                            if((port_report[4] == 0x40) && (port_report[5] == 0x15))
                            {//-diff����
                              if(port_report[7] <= 5)
                              {
                                menu_set_tt_diff_pt = port_report[7];
                                menu_set_tt_diff = diff_constant[port_report[7]]; //-���ȵ���Сֵ,��ʵ���¶ȵ������ֵʱ����Ҫ������
                                MOD_TC_normal_respond(&port_report[4]);
                                MOD_TC_transmit_wait_time=cticks_5ms;
                                SET_Modify_flag = 0x55;
                              }
                            }
                            else
                            {
                                if((port_report[4] == 0x40) && (port_report[5] == 0x16))
                                {//-third����
                                  if(port_report[7] <= 6)
                                  {
                                    menu_set_tt_third_pt = port_report[7];
                                    menu_set_tt_third = third_constant[port_report[7]]; //-���ȵ���Сֵ,��ʵ���¶ȵ������ֵʱ����Ҫ������
                                    MOD_TC_normal_respond(&port_report[4]);
                                    MOD_TC_transmit_wait_time=cticks_5ms;
                                    SET_Modify_flag = 0x55;
                                  }
                                }
                                else
                                {
                                    if((port_report[4] == 0x40) && (port_report[5] == 0x00))
                                    {//-���ÿ��ػ�
                                      if(port_report[7] <= 1)   //-ʵ��֤������"����"�Ļ���Ҫ��
                                      {
                                        if(port_report[7] == 0)
                                        {
                                          menu_wind_status = 0x80;
                                          //-RUN_status = port_report[7];
                                        }
                                        //-else
                                        //-  RUN_status = BKP_ReadBackupRegister(BKP_DR4);  //-������Ҫ����
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
                                        {//-���÷���������
                                            beep_onoff = port_report[7]; //-���ȵ���Сֵ,��ʵ���¶ȵ������ֵʱ����Ҫ������
                                            MOD_TC_normal_respond(&port_report[4]);
                                            MOD_TC_transmit_wait_time=cticks_5ms;
                                            SET_Modify_flag = 0x55;
                                        }
                                        else
                                        {
                                            if((port_report[4] == 0x40) && (port_report[5] == 0x18))
                                            {//-�������ȿ���
                                                //-OUT_emergency_hot = port_report[7]; //-���ȵ���Сֵ,��ʵ���¶ȵ������ֵʱ����Ҫ������
                                                if(port_report[7] == 0)  //-ԭ����ģʽ�ǿ��Ļ�,���ھ�����׼����,����ʵ�ʹز�����˵
                                                {
                                                  OUT_emergency_hot = 0;    //-�������ȿ���ֱ�ӹر�,���ǿ�����Ҫ��������
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
                                              {//-�������
                                                if(menu_wind_status != 0x51)
                                                {
                                                  if(port_report[7] == 2) //-���ȵ���Сֵ,��ʵ���¶ȵ������ֵʱ����Ҫ������
                                                    menu_wind_status = 0x81;   //-1��ʾ��
                                                  else
                                                    menu_wind_status = 0x80;   //-0��ʾ�Զ�
                                                }
                                                  MOD_TC_normal_respond(&port_report[4]);
                                                  MOD_TC_transmit_wait_time=cticks_5ms;
                                                  SET_Modify_flag = 0x55;
                                              }
                                              else
                                              {
                                                  if((port_report[4] == 0x40) && (port_report[5] == 0x0F))
                                                  {//-�ⲿ�¶�
                                                      temperature_data_out = port_report[6] << 8; //-���ȵ���Сֵ,��ʵ���¶ȵ������ֵʱ����Ҫ������                                                       
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
                                                          if(port_report[7] == 1) //-��ʼ����,�����15S
                                                          {
                                                            zigbee_flag = 1;
                                                          }
                                                          else if(port_report[7] == 2)  //-�����ɹ�,����
                                                          {
                                                            zigbee_flag = 0x55;
                                                          }
                                                          else if(port_report[7] == 3)
                                                          {//-����,���
                                                            zigbee_flag = 0;
                                                          }
                                                          else if(port_report[7] == 4)
                                                          {//-����,��������
                                                            STOP_status2 = 1;
                                                            STOP_wait_time2 = 0;
                                                            STOP_wait_time = cticks_5ms;
                                                          }
                                                          else if(port_report[7] == 5)
                                                          {//-�豸����,�̶���6��
                                                            zigbee_flag = 1;
                                                          }
                                                          
                                                          MOD_TC_normal_respond(&port_report[4]);
                                                          MOD_TC_transmit_wait_time=cticks_5ms;
                                                      }
                                                      else
                                                      {
                                                          if((port_report[4] == 0x40) && (port_report[5] == 0x11))
                                                          {//-�ⲿ�¶�                                                                                                                
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
                                                              {//-�𶯿���                                                                                                                                                                                  
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
				if(port_report[3] == 0x10)	//-�жϹ�����
	      {//-д����Ĵ���
	      	 if((port_report[4] == 0x40) && (port_report[5] == 0x19))
	      	 {//-����ʱ��
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
                  temp_data1 = (temp_data - 0x4030) / 0x10;   //-�õ����ƫ����
                  the_ram_ax = port_report[8];
                  for(i=0;i<the_ram_ax/4;i++)
                  {
                    programme_day[temp_data1].day_dot[i].MIN = ((port_report[9 + 4*i] << 8) + port_report[10 + 4*i]) * 15;
                    programme_day[temp_data1].day_dot[i].MIN_VOULE = port_report[11 + 4*i];
                    programme_day[temp_data1].day_dot[i].MAX_VOULE = port_report[12 + 4*i];
                  }
                  programme_day[temp_data1].SET_status = 0x55;    //-��ֵ������,��Ҫ�洢��EEP��
                  MOD_TC_10normal_respond(&port_report[4]);
                  MOD_TC_transmit_wait_time=cticks_5ms;
               }
               else
               {
                  if((port_report[4] == 0x40) && (port_report[5] == 0x04))
                  {//-���ù�����ʽ
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
                        menu_set_tt_min_eep = (port_report[11] << 8) + port_report[12];  //-����
                        menu_set_tt_min_f = (u16)(3200 + menu_set_tt_min_eep * 1.8);
                        temp_data = menu_set_tt_min_f % 100;  //-���Ͻ�������������û��С��,,//-��ʾ�޸ĵĻ���
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
                        menu_set_tt_max_eep = (port_report[13] << 8) + port_report[14];  //-����
                        menu_set_tt_max_f = (u16)(3200 + menu_set_tt_max_eep * 1.8);
                        temp_data = menu_set_tt_max_f % 100;  //-���Ͻ�������������û��С��,,//-��ʾ�޸ĵĻ���
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
                        auto_set_tt_min_eep = (port_report[11] << 8) + port_report[12];  //-����
                        temp_data = auto_set_tt_min_eep % 100;
                        if(temp_data >= 50)
                          auto_set_tt_min_eep = (auto_set_tt_min_eep / 100) * 100 + 50;
                        else
                          auto_set_tt_min_eep = (auto_set_tt_min_eep / 100) * 100;
                        auto_set_tt_max_eep = (port_report[13] << 8) + port_report[14];  //-����
                        temp_data = auto_set_tt_max_eep % 100;
                        if(temp_data >= 50)
                          auto_set_tt_max_eep = (auto_set_tt_max_eep / 100) * 100 + 50;
                        else
                          auto_set_tt_max_eep = (auto_set_tt_max_eep / 100) * 100;
                      }
                      SET_Modify_flag = 0x55;   //-by zj �޸�֮����ҪдFlash                      
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
	      {//-����LED������ͼƬ
          if(port_report[3] == 0x03)  //-��ѯ
          {
            if((port_report[4] == 0x40) && (port_report[5] == 0x00))
            {
              //-MOD_TC_status_data_respond();
              MOD_TC_send_status_data();  //-Ϊ�˱���һ��
              MOD_TC_transmit_wait_time=cticks_5ms;
            }
            else
            {
                if((port_report[4] == 0x40) && (port_report[5] == 0x13))
                {
                  //-MOD_TC_status_data_respond();
                  MOD_TC_send_status_data2();  //-Ϊ�˱���һ��
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
         STOP_status2 = 1; //-��������ʾ
         STOP_wait_time2 = 0;
         STOP_wait_time = cticks_5ms;
    	   MOD_TC_clear_port_report_deal(delta_len);
      }
rxd_out_time:	//-ִ�е�����˵�����ճ�ʱ,����ɹ�,�������ڿ��Լ���������,,�����������ִ�������,����
	    if(Judge_Time_In_MainLoop(MOD_TC_rxd_wait_time,MOD_TC_WAIT_TIME_VALUE)==YES)	//-����ͨ�ŵĻ��ǲ�Ӧ�ó�ʱ��,����ʱ����Ϊ����,�������³�ʼ��
	    {	//-������һ������֮��,�����ȴ��ش�ʱ��֮��,�Ϳ��Դ��·���һ��
	      	MOD_TC_rec_OK=NO;
	      	MOD_TC_rxd_head_flag=NO;
	      	MOD_TC_rxd_wait_time=cticks_5ms;
	      	MOD_TC_transmit_flag=YES;	//-��ʾ���ڿ�����֯��������
	      	//-MOD_TC_wait_replay=NO;	//-��ʾ���ڻ�û�еȴ��ظ�
	      	//-MOD_TC_transmit_wait_time=Time_1ms_Counter;

	       	//-MOD_TC_comm_err_counter[port_send[0][0]]++;	//-��ʱ�������


	    }
inrda:
		   //-�������� ,,�������һ������,�೤ʱ�䷢��һ��,����˵������ʹ���������ʲô
		   if((UART1_transmit_flag==YES) && (MOD_TC_transmit_flag==YES))		//-Ŀǰ����ν����˫���
		   {
          if(Judge_Time_In_MainLoop(MOD_TC_transmit_wait_time,100)==YES)	//-ֻ�й��˵ȴ�ʱ��Żᷢ��,����ʱ����Ҫ��Ϊ�˸����ն��㹻�Ĵ���ʱ��
			   	  switch(UART1_transmit_control)
	          {
	                case 1:   //-����
	                     //MOD_TC_rec_frame_type=MOD_TC_YX_FRAME;	//-��ʾ��Ҫ���յ�֡�����������ֵ,���յ�������ֵ������ȷ��
	                     //MOD_TC_wait_replay=YES;
	                     MOD_TC_ask_join_cmd();	//-����Լ�����������,�ʴ��͵�,����˿��ϵ�����װ�ö��ǽ��ղ�ѯ
	                     MOD_TC_transmit_wait_time=cticks_5ms;

	                     UART1_transmit_flag = NO;		//-���︴���˾Ͳ��ȴ�Ӧ����ֱ�ӷ��Ա����
    									 UART1_transmit_control = 0;

	                     break;
	                case 2:   //-����
	                     //MOD_TC_rec_frame_type=MOD_TC_YX_FRAME;	//-��ʾ��Ҫ���յ�֡�����������ֵ,���յ�������ֵ������ȷ��
	                     //MOD_TC_wait_replay=YES;
	                     MOD_TC_ask_move_cmd();	//-����Լ�����������,�ʴ��͵�,����˿��ϵ�����װ�ö��ǽ��ղ�ѯ
	                     MOD_TC_transmit_wait_time=cticks_5ms;

	                     UART1_transmit_flag = NO;		//-���︴���˾Ͳ��ȴ�Ӧ����ֱ�ӷ��Ա����
    									 UART1_transmit_control = 0;

	                     break;
                   case 3:   //-������ֵ
	                     //MOD_TC_rec_frame_type=MOD_TC_YX_FRAME;	//-��ʾ��Ҫ���յ�֡�����������ֵ,���յ�������ֵ������ȷ��
	                     //MOD_TC_wait_replay=YES;
	                     MOD_TC_ask_program_cmd();	//-����Լ�����������,�ʴ��͵�,����˿��ϵ�����װ�ö��ǽ��ղ�ѯ
	                     MOD_TC_transmit_wait_time=cticks_5ms;

	                     UART1_transmit_flag = NO;		//-���︴���˾Ͳ��ȴ�Ӧ����ֱ�ӷ��Ա����
    									 UART1_transmit_control = 0;

	                     break;
                   case 4:   //-�������ͱ��ն�״ֵ̬1
	                     //MOD_TC_rec_frame_type=MOD_TC_YX_FRAME;	//-��ʾ��Ҫ���յ�֡�����������ֵ,���յ�������ֵ������ȷ��
	                     //MOD_TC_wait_replay=YES;
	                     MOD_TC_send_status_data();	//-����Լ�����������,�ʴ��͵�,����˿��ϵ�����װ�ö��ǽ��ղ�ѯ
	                     MOD_TC_transmit_wait_time=cticks_5ms;

	                     UART1_transmit_flag = NO;		//-���︴���˾Ͳ��ȴ�Ӧ����ֱ�ӷ��Ա����
    									 UART1_transmit_control = 0;

	                     break;
                   case 5:   //-�������ͱ��ն�״ֵ̬2
	                     //MOD_TC_rec_frame_type=MOD_TC_YX_FRAME;	//-��ʾ��Ҫ���յ�֡�����������ֵ,���յ�������ֵ������ȷ��
	                     //MOD_TC_wait_replay=YES;
	                     MOD_TC_send_status_data2();	//-����Լ�����������,�ʴ��͵�,����˿��ϵ�����װ�ö��ǽ��ղ�ѯ
	                     MOD_TC_transmit_wait_time=cticks_5ms;

	                     UART1_transmit_flag = NO;		//-���︴���˾Ͳ��ȴ�Ӧ����ֱ�ӷ��Ա����
    									 UART1_transmit_control = 0;

	                     break; 
                   case 6:   //-�������ͱ��ն�״ֵ̬2
	                     //MOD_TC_rec_frame_type=MOD_TC_YX_FRAME;	//-��ʾ��Ҫ���յ�֡�����������ֵ,���յ�������ֵ������ȷ��
	                     //MOD_TC_wait_replay=YES;
	                     MOD_TC_send_ASK_data();	//-����Լ�����������,�ʴ��͵�,����˿��ϵ�����װ�ö��ǽ��ղ�ѯ
	                     MOD_TC_transmit_wait_time=cticks_5ms;
                       

	                     UART1_transmit_flag = NO;		//-���︴���˾Ͳ��ȴ�Ӧ����ֱ�ӷ��Ա����
    									 UART1_transmit_control = 0;

	                     break;   
                    case 7:   //-�������ͱ��ն�״ֵ̬2
	                     //MOD_TC_rec_frame_type=MOD_TC_YX_FRAME;	//-��ʾ��Ҫ���յ�֡�����������ֵ,���յ�������ֵ������ȷ��
	                     //MOD_TC_wait_replay=YES;
	                     MOD_TC_send_power_state();	//-����Լ�����������,�ʴ��͵�,����˿��ϵ�����װ�ö��ǽ��ղ�ѯ
	                     MOD_TC_transmit_wait_time=cticks_5ms;
                       

	                     UART1_transmit_flag = NO;		//-���︴���˾Ͳ��ȴ�Ӧ����ֱ�ӷ��Ա����
    									 UART1_transmit_control = 0;

	                     break;   
	                default:
	                     break;
	          }


		   }
 	 }

}


