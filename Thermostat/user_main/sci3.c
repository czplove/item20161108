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




#define   MOD_KG_WAIT_TIME_VALUE                   250

#define   MOD_KG_polling_ID_FRAME                  0x01
#define   MOD_KG_polling_data_FRAME                0x02
#define   MOD_KG_LED_display_FRAME                 0x03
#define   MOD_KG_receive_data_FRAME                0x04
#define   MOD_KG_receive_set_FRAME                 0x05
#define   MOD_KG_SET_screen_FRAME									 0x06
#define   MOD_KG_mode_set_FRAME										 0x07
#define   MOD_KG_chage_page_FRAME									 0x08
#define   MOD_KG_chage_pic_FRAME									 0x09
#define   MOD_KG_SET_loop_FRAME		  							 0x0A
#define   MOD_KG_send_state_FRAME		  						 0x0B
#define   MOD_KG_re_Weather_FRAME		  						 0x0C
#define   MOD_KG_re_show_FRAME		  						   0x0D
#define   MOD_KG_polling_state_FRAME		  				 0x0E
#define   MOD_KG_polling_PM25_FRAME		  				   0x0F
#define   MOD_KG_end_PM25_FRAME		  	    			   0x10
#define   MOD_KG_polling_hl_FRAME		  	    			 0x11
#define   MOD_KG_SET_hl_time_FRAME		  	    		 0x13
#define   MOD_KG_send_leddis_flag_FRAME		     		 0x14
#define   MOD_KG_SET_voice_flag_FRAME		     		   0x15
#define   MOD_KG_polling_voice_flag_FRAME		   	   0x16
#define   MOD_KG_re_gateway_flag_FRAME		    	   0x17

#define   MOD_KG_send_voice_mode_FRAME	      	   0x1B

#define   MOD_KG_set_hl_set_FRAME	     	           0x21

#define   MOD_KG_7620_inquire_upSDB_FRAME          0x22
#define   MOD_KG_7620_inquire_update_FRAME	       0x23
#define   MOD_KG_7620_inquire_version_FRAME	       0x24



#define   MOD_KG_control_HRL_FRAME		  	    	   0x40
#define   MOD_KG_leddis_end_FRAME	  	  	    	   0x41
#define   MOD_KG_send_HRL_mode_FRAME	  	     	   0x42
#define   MOD_KG_control_FUN_FRAME	    	     	   0x43
#define   MOD_KG_update_status_FRAME	    	     	 0x44



//-ΪЭ�鴦����ʱ����ı���
BYTE  MOD_KG_rxd_head_flag;
BYTE  MOD_KG_rec_OK;
BYTE  MOD_KG_rec_frame_type;
WORD  MOD_KG_rxd_wait_time;
BYTE  MOD_KG_transmit_flag;
BYTE  MOD_KG_transmit_control;
WORD  MOD_KG_transmit_wait_time;



////////////////////////////////////////////////////////////////////////////////
/*
����1:��ѯ�������豸��ID��
7620���ͣ�
0xaa 0x55 0x05 0x02 0x01 CRC
STM32��Ӧ��
0xaa 0x55 0x0b 0x02 0x01 0x01 0x02 0x03 0x04 0x05 0x06 CRC






*/
unsigned int MOD_KG_CRC16(unsigned char *MOD_KG_CRC16_start,unsigned char MOD_KG_CRC16_bytes)    //*xΪָ��ÿ��ǰ5�����ݵ�ָ��
{	//-��������У��CRC
unsigned int bx, cx, i, j;

    bx = 0xffff;
    cx = 0xa001;
    for(i=0;i<MOD_KG_CRC16_bytes;i++)
     {
      bx=bx^MOD_KG_CRC16_start[i];
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

void MOD_KG_clear_port_report_deal(void)
{
	 WORD i,len,temp_loop;

	 len = port_report[2] + 2;
	 temp_loop = port_recv_dl[1];

	 for(i=0;i<len;i++)
	 {
	 	  if(temp_loop > 0)
	 	  	temp_loop = temp_loop - 1;
	 	  else
	 	  	temp_loop = 255;
	    port_recv[1][temp_loop] = 0;
	 }


	 for(i=0;i<len;i++)
	   port_report[i] = 0;
}

////////////////////////////////////////////////////////////////////////////////


void uart3_Main(void)
{
   WORD the_ram_ax,the_ram_bx,the_ram_cx,i;
	 WORD temp_data,temp_data1,delta_len,temp_loop,temp_int;


	 //-�жϷ����Ƿ������,��������˿��������߼�����,���򲻴���
	 if(DMA_GetFlagStatus(DMA1_FLAG_TC2))		//-����˵�����������
   {//-ֻ�е���������˲ſ������¸�DMA���ʹ�������
   	  //-���ݷ������֮��ȫ������,�������ڲ�֪��ÿ���Ŀռ��С��,��ôѡȡһ��������
   	  //-for(temp_loop = 0;temp_loop < 43;temp_loop++)
   	  //-	port_send[0][temp_loop] = 0;
   	  port_deal_flag[2] = 0;		//-��һ����0ֵ�Ϳ������´μ�鷢��ʱ������������
   	  //-MOD_KG_transmit_flag=YES;		//-���ҵ����ݷ��ͳ�ȥ��Ҳ������������������??����Ӧ�ò���������֯���ݵķ���,������������ݻᱻ����,���յı�־λ
   	  DMA_ClearFlag(DMA1_FLAG_TC2);
   }

   if((port_deal_flag[2] == 0) /*&& (UART3_TO_UART2_FLAG == 0)*/)	//-����᲻������һ�η���ʧ�ܶ�������Ҳ���ܷ���,,����UART3_TO_UART2_FLAGΪ0��ʼ������յ��ı���,��ֹ,������û�з��ͳ�ȥ������,��ɵĶ�ʧ
   {
   	   //-���ȴ�����յ�������,���û�����ݽ��յ�,����֯������Ҫ���͵�����
   	   if(port_recv_pt[2]!=port_recv_dl[2])
   	   {

   	   	   if(MOD_KG_rxd_head_flag==NO)	//-���յ������ݻ�û�д����ʱ�����NO
           {
           	   MOD_KG_rxd_wait_time=cticks_5ms;	//-������Ϊ�˽�������
		   	   	   temp_data = port_recv_pt[2];
		   	   	   temp_data1 = port_recv_dl[2];
		           if(temp_data1>temp_data)	//-ǰ����Ǵ���ָ���ʵ�ʵĽ���ָ����бȽ�
		               delta_len=(temp_data+256)-temp_data1;
		           else
		               delta_len=temp_data-temp_data1;	//-һ���ĳ���
		           for(temp_loop=temp_data1;temp_loop<(delta_len+temp_data1);temp_loop++)
		           {
		        	   if(port_recv[2][port_recv_dl[2]]==0xaa)	//-����ط��Ƚϵ��Ǵ�վ��ַ,�����Ҿ���û���κι��ɾ���ͨѶ
		        	   {	//-����һ�п������õ�
		        	     the_ram_ax=(port_recv_dl[2]+1)&0xff;
		        	     if(temp_data == the_ram_ax)	//-���Ԥȡ��ָ��ָ��յĿռ�,˵�����ݻ�û�е�,��Ҫ�ȴ�
		        	     	 break;
		        	     if(port_recv[2][the_ram_ax]==0x55)	//-�Ƚϵ��ǹ�����
		        	     {
		        	         MOD_KG_rxd_head_flag=YES;	//-��ʾ�Ѿ��ɹ�ʶ����յ����±��ĵ�ͷ��
		        	         break;
		        	     }
		        	   }
		        	   port_recv_dl[2]++;	//-����һ���ֵı���
		        	   port_recv_dl[2]&=0xff;
		           }
   	   	   }
   	   	   if(MOD_KG_rxd_head_flag==YES)	//-���յ������ݻ�û�д����ʱ�����NO
       		 {
       		 	   temp_data = port_recv_pt[2];
       		 	   if(port_recv_dl[2]>temp_data)
               		delta_len=(temp_data+256)-port_recv_dl[2];
               else
               		delta_len=temp_data-port_recv_dl[2];

               if(delta_len>=10)	//-���ٻ���4���ֽڲ�����֯һ������
		           {
		               //-temp_int=(port_recv_dl[0]+2)&0xff;
		               //-if(delta_len>=(unsigned short)(port_recv[0][temp_int]+2))	//-�õ��ı��ĳ��Ⱥ������ϵı��ĳ��Ƚ��бȽ�
		               {
		                  MOD_KG_rxd_head_flag=NO;
		                  MOD_KG_rec_OK=YES;
                      goto rec_ok_deal;	//-�������ؿ���,���������Ϊ�ǳɹ����յ�һ�����ر�����
		               }

		           }

     			 }
   	   }
   	   goto rxd_out_time;		//?����������Ǽ򵥵�����ģʽ,����ֱ�Ӳ�ѯ����
rec_ok_deal:
	    if(MOD_KG_rec_OK==YES)	//-�϶��Ƿ�ֹ�ܷɵ�.
	    {	//-������Ϳ���˵��Ӧ�����Ѿ�������
	        MOD_KG_rec_OK=NO;	//-�ɹ����յ������ݿ�ʼ������֮��,�ͻָ�0
	        MOD_KG_transmit_flag=YES;
	        //-MOD_KG_transmit_wait_time=Time_1ms_Counter;	//-��Ȼ����˵���Է���������,���ǻ���Ҫ��ʱһ��ʱ��,��Ϊ���յ���������Ҫ����
	        //-�����Ƕ�ʵ�����ݵĴ���,������Ǻ��Ĳ���
	        the_ram_bx=(port_recv_dl[2]+3)&0xff;;
	        if(port_recv[2][the_ram_bx]!=0xFF)	//-����ǶԹ�������ж�,�����벻ͬ�жϵ�����Ҳ��һ��
	        {	//-����������һ�ִ������ڿ��Բ���
	          	the_ram_ax=(port_recv_dl[2]+2)&0xff;
	          	//-temp_int=port_recv[0][the_ram_ax]+2+port_recv_dl[0];
              temp_int=10+port_recv_dl[2];
	          	for(temp_loop=port_recv_dl[2];temp_loop<temp_int;temp_loop++)	//-���������ɵ����ܾ��Ǳ�֤��λ����Ҫ����ı����ֽ�
	          	{	//-�򵥵Ĳ���Ҫ���������Ǹ��ӵĻ�����Ҫ��,��ô�������˵û������Ծͻ�ܺ�
	                 if(temp_loop<=255)
	           	       port_report[temp_loop-port_recv_dl[2]]=port_recv[2][temp_loop];
	                 else
	           	       port_report[temp_loop-port_recv_dl[2]]=port_recv[2][temp_loop-256];	//-�ѵ��Ǹ��ٸ��µ�Ե����Ҫ��ǰ���Ƴ���
	          	}	//-���߻���һ�ֿ����Ծ���ͳһ����
	         	//-port_recv_dl[1]+=delta_len;	//-����ط��������������Ĵ�����
            //-port_recv_dl[0]+=(port_report[2] + 2);
            port_recv_dl[2]+=(10);
	         	port_recv_dl[2]&=0xff;
	         	temp_int=MOD_KG_CRC16(&port_report[2],8);
	         	if((LOBYTE(temp_int)!=port_report[8+0])||(HIBYTE(temp_int)!=port_report[8+1]))	//-����CRC���
	         	{	//-������Ĳ�ȷ����,���У�鲻��ȷ�Ļ���ֱ�Ӱ�ͷ���,����Ѱ��
	          		goto inrda;	//-������˵���ɹ����յ��ı���CRCУ��û��ͨ��
	          }
	        }
	        else
	        {
	            //-port_recv_dl[1]+=delta_len;	//-Ŀǰֱ������
              port_recv_dl[2]+=7;
	            port_recv_dl[2]&=0xff;
	            goto inrda;
	        }


      //-��ʱ�ȸ�ֵ,��������������Դ����߼�
      //-��������ж��½��ձ���,Ȼ�������������
      //-test_cn[5]++;
    	//-��ȷ���ݵĴ���
////////////////////////////////////////////////////////////////////////////////
			if(port_report[3] == 0x81)	//-�жϹ�����
			{
					//-MOD_KG_data_ack_deal();
			}
			else
			{
				if(port_report[3] == 2)	//-�жϹ�����
	      {
	      	 if(port_report[4] == 1)
	      	 {
	      	 	   MOD_KG_rec_frame_type = MOD_KG_polling_ID_FRAME;
	      	 }
	      	 else
	      	 {
	      	 	   MOD_KG_rec_frame_type = MOD_KG_polling_data_FRAME;
	      	 }
	      }
	      else
	      {//-����LED������ͼƬ

	      }
      }

////////////////////////////////////////////////////////////////////////////////

    	   MOD_KG_clear_port_report_deal();
      }
rxd_out_time:	//-ִ�е�����˵�����ճ�ʱ,����ɹ�,�������ڿ��Լ���������,,�����������ִ�������,����
	    if(Judge_Time_In_MainLoop(MOD_KG_rxd_wait_time,MOD_KG_WAIT_TIME_VALUE)==YES)	//-����ͨ�ŵĻ��ǲ�Ӧ�ó�ʱ��,����ʱ����Ϊ����,�������³�ʼ��
	    {	//-������һ������֮��,�����ȴ��ش�ʱ��֮��,�Ϳ��Դ��·���һ��
	      	MOD_KG_rec_OK=NO;
	      	MOD_KG_rxd_head_flag=NO;
	      	MOD_KG_rxd_wait_time=cticks_5ms;
	      	MOD_KG_transmit_flag=YES;	//-��ʾ���ڿ�����֯��������
	      	//-MOD_KG_wait_replay=NO;	//-��ʾ���ڻ�û�еȴ��ظ�
	      	//-MOD_KG_transmit_wait_time=Time_1ms_Counter;

	       	//-MOD_KG_comm_err_counter[port_send[0][0]]++;	//-��ʱ�������


	    }
inrda:
		   //-�������� ,,�������һ������,�೤ʱ�䷢��һ��,����˵������ʹ���������ʲô
		   if(MOD_KG_transmit_flag==YES)		//-Ŀǰ����ν����˫���
		   {

	        //-�������3,���͵�����ȫ�����Դ���2,�Լ�����֯�κη���
	        the_ram_ax = port_send_pt[2];
	        the_ram_bx = port_send_dl[2];
	        if(the_ram_ax != the_ram_bx)
	        {
	        	 if(the_ram_ax>=the_ram_bx)
	        	 		the_ram_cx = the_ram_ax-the_ram_bx;
	        	 else
	        	 	  the_ram_cx = 256+the_ram_ax-the_ram_bx;

	        	 for(i=0;i < the_ram_cx;i++)
	        	 {
	        	 	  the_ram_ax = (port_send_dl[2] + i) & 0xff;
	        	 		UART3_port_send[i] = port_send[0][the_ram_ax];		//-���շ������ݵĻ���,ǰ����ÿ�δӻ�������ȡ������,���ڷ���
	        	 		port_send[2][the_ram_ax] = 0;		//-Ϊ�˿ɿ�����,����ʧЧ����0
	        	 }

	        	 port_send_dl[2] = (port_send_dl[2] + the_ram_cx) & 0xff;
	        	 port_deal_flag[2] = 0xaa;    //-��֤���ڷ��ͽ���֮ǰ,�ٴ���֯����
	        	 port_send_len[2] = the_ram_cx;
             //-test_cn[1]++;    //-������

             //-if((test_cn[4] == 1) && (Judge_Time_In_MainLoop(test_cn_wait_time,2)==YES))
             //-{
             //-   test_cn[4] = 2;
             //-}
	        }

		   }
 	 }

}


