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



//-为协议处理临时定义的变量
BYTE  MOD_KG_rxd_head_flag;
BYTE  MOD_KG_rec_OK;
BYTE  MOD_KG_rec_frame_type;
WORD  MOD_KG_rxd_wait_time;
BYTE  MOD_KG_transmit_flag;
BYTE  MOD_KG_transmit_control;
WORD  MOD_KG_transmit_wait_time;



////////////////////////////////////////////////////////////////////////////////
/*
例子1:查询传感器设备（ID）
7620发送：
0xaa 0x55 0x05 0x02 0x01 CRC
STM32回应：
0xaa 0x55 0x0b 0x02 0x01 0x01 0x02 0x03 0x04 0x05 0x06 CRC






*/
unsigned int MOD_KG_CRC16(unsigned char *MOD_KG_CRC16_start,unsigned char MOD_KG_CRC16_bytes)    //*x为指向每行前5个数据的指针
{	//-计算冗余校验CRC
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


	 //-判断发送是否结束了,如果结束了可以正常逻辑处理,否则不处理
	 if(DMA_GetFlagStatus(DMA1_FLAG_TC2))		//-进入说明发送完成了
   {//-只有当传输完成了才可以重新给DMA发送传输命令
   	  //-内容发送完成之后全部清理,由于现在不知道每个的空间大小了,那么选取一块大的清零
   	  //-for(temp_loop = 0;temp_loop < 43;temp_loop++)
   	  //-	port_send[0][temp_loop] = 0;
   	  port_deal_flag[2] = 0;		//-给一个非0值就可以再下次检查发送时发发送命令了
   	  //-MOD_KG_transmit_flag=YES;		//-当我的内容发送出去了也可以立即发送内容了??这里应该不能立即组织内容的发送,这样下面的内容会被覆盖,接收的标志位
   	  DMA_ClearFlag(DMA1_FLAG_TC2);
   }

   if((port_deal_flag[2] == 0) /*&& (UART3_TO_UART2_FLAG == 0)*/)	//-这里会不会由于一次发送失败而导致再也不能发送,,增加UART3_TO_UART2_FLAG为0开始处理接收到的报文,防止,有内容没有发送出去覆盖了,造成的丢失
   {
   	   //-首先处理接收到的数据,如果没有内容接收到,才组织可能需要发送的内容
   	   if(port_recv_pt[2]!=port_recv_dl[2])
   	   {

   	   	   if(MOD_KG_rxd_head_flag==NO)	//-接收到的数据还没有处理的时候就是NO
           {
           	   MOD_KG_rxd_wait_time=cticks_5ms;	//-仅仅是为了接收完整
		   	   	   temp_data = port_recv_pt[2];
		   	   	   temp_data1 = port_recv_dl[2];
		           if(temp_data1>temp_data)	//-前面的是处理指针比实际的接收指针进行比较
		               delta_len=(temp_data+256)-temp_data1;
		           else
		               delta_len=temp_data-temp_data1;	//-一共的长度
		           for(temp_loop=temp_data1;temp_loop<(delta_len+temp_data1);temp_loop++)
		           {
		        	   if(port_recv[2][port_recv_dl[2]]==0xaa)	//-这个地方比较的是从站地址,但是我觉得没有任何规律就是通讯
		        	   {	//-利用一切可以利用的
		        	     the_ram_ax=(port_recv_dl[2]+1)&0xff;
		        	     if(temp_data == the_ram_ax)	//-如果预取的指针指向空的空间,说明数据还没有到,需要等待
		        	     	 break;
		        	     if(port_recv[2][the_ram_ax]==0x55)	//-比较的是功能码
		        	     {
		        	         MOD_KG_rxd_head_flag=YES;	//-表示已经成功识别接收到的新报文的头了
		        	         break;
		        	     }
		        	   }
		        	   port_recv_dl[2]++;	//-舍弃一个字的报文
		        	   port_recv_dl[2]&=0xff;
		           }
   	   	   }
   	   	   if(MOD_KG_rxd_head_flag==YES)	//-接收到的数据还没有处理的时候就是NO
       		 {
       		 	   temp_data = port_recv_pt[2];
       		 	   if(port_recv_dl[2]>temp_data)
               		delta_len=(temp_data+256)-port_recv_dl[2];
               else
               		delta_len=temp_data-port_recv_dl[2];

               if(delta_len>=10)	//-至少还有4个字节才能组织一包内容
		           {
		               //-temp_int=(port_recv_dl[0]+2)&0xff;
		               //-if(delta_len>=(unsigned short)(port_recv[0][temp_int]+2))	//-得到的报文长度和理论上的报文长度进行比较
		               {
		                  MOD_KG_rxd_head_flag=NO;
		                  MOD_KG_rec_OK=YES;
                      goto rec_ok_deal;	//-经过重重考核,到这里就认为是成功接收到一个返回报文了
		               }

		           }

     			 }
   	   }
   	   goto rxd_out_time;		//?由于这个不是简单的主从模式,所以直接查询发送
rec_ok_deal:
	    if(MOD_KG_rec_OK==YES)	//-肯定是防止跑飞的.
	    {	//-到这里就可以说明应答报文已经可以了
	        MOD_KG_rec_OK=NO;	//-成功接收到的数据开始处理了之后,就恢复0
	        MOD_KG_transmit_flag=YES;
	        //-MOD_KG_transmit_wait_time=Time_1ms_Counter;	//-虽然上面说可以发送数据了,但是还是要延时一段时间,因为接收到的数据需要处理
	        //-下面是对实际数据的处理,处理的是核心部分
	        the_ram_bx=(port_recv_dl[2]+3)&0xff;;
	        if(port_recv[2][the_ram_bx]!=0xFF)	//-这个是对功能码的判断,功能码不同判断的依据也不一样
	        {	//-这里是宁外一种处理现在可以不管
	          	the_ram_ax=(port_recv_dl[2]+2)&0xff;
	          	//-temp_int=port_recv[0][the_ram_ax]+2+port_recv_dl[0];
              temp_int=10+port_recv_dl[2];
	          	for(temp_loop=port_recv_dl[2];temp_loop<temp_int;temp_loop++)	//-上面这样干的秘密就是保证定位到需要处理的报文字节
	          	{	//-简单的不需要这样处理但是复杂的还是需要的,那么这样用了得话兼容性就会很好
	                 if(temp_loop<=255)
	           	       port_report[temp_loop-port_recv_dl[2]]=port_recv[2][temp_loop];
	                 else
	           	       port_report[temp_loop-port_recv_dl[2]]=port_recv[2][temp_loop-256];	//-难道是高速更新的缘故需要提前复制出来
	          	}	//-或者还有一种可能性就是统一处理
	         	//-port_recv_dl[1]+=delta_len;	//-这个地方就舍弃了这样的处理报文
            //-port_recv_dl[0]+=(port_report[2] + 2);
            port_recv_dl[2]+=(10);
	         	port_recv_dl[2]&=0xff;
	         	temp_int=MOD_KG_CRC16(&port_report[2],8);
	         	if((LOBYTE(temp_int)!=port_report[8+0])||(HIBYTE(temp_int)!=port_report[8+1]))	//-进行CRC检查
	         	{	//-由于这的不确定性,如果校验不正确的话就直接把头舍掉,继续寻找
	          		goto inrda;	//-到这里说明成功接收到的报文CRC校验没有通过
	          }
	        }
	        else
	        {
	            //-port_recv_dl[1]+=delta_len;	//-目前直接舍弃
              port_recv_dl[2]+=7;
	            port_recv_dl[2]&=0xff;
	            goto inrda;
	        }


      //-临时先赋值,后面这个变量可以处理逻辑
      //-可以组合判断下接收报文,然后决定下面走向
      //-test_cn[5]++;
    	//-正确内容的处理
////////////////////////////////////////////////////////////////////////////////
			if(port_report[3] == 0x81)	//-判断功能码
			{
					//-MOD_KG_data_ack_deal();
			}
			else
			{
				if(port_report[3] == 2)	//-判断功能码
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
	      {//-接收LED点阵屏图片

	      }
      }

////////////////////////////////////////////////////////////////////////////////

    	   MOD_KG_clear_port_report_deal();
      }
rxd_out_time:	//-执行到这里说明接收超时,或处理成功,反正现在可以继续发送了,,正常情况都会执行这里的,首先
	    if(Judge_Time_In_MainLoop(MOD_KG_rxd_wait_time,MOD_KG_WAIT_TIME_VALUE)==YES)	//-正常通信的话是不应该超时的,若过时就认为出错,下面重新初始化
	    {	//-当发送一个报文之后,超过等待回答时间之后,就可以从新发送一次
	      	MOD_KG_rec_OK=NO;
	      	MOD_KG_rxd_head_flag=NO;
	      	MOD_KG_rxd_wait_time=cticks_5ms;
	      	MOD_KG_transmit_flag=YES;	//-表示现在可以组织发送内容
	      	//-MOD_KG_wait_replay=NO;	//-表示现在还没有等待回复
	      	//-MOD_KG_transmit_wait_time=Time_1ms_Counter;

	       	//-MOD_KG_comm_err_counter[port_send[0][0]]++;	//-超时出错计数


	    }
inrda:
		   //-主动发送 ,,这里就有一个问题,多长时间发送一次,或者说这个发送触发条件是什么
		   if(MOD_KG_transmit_flag==YES)		//-目前无所谓的是双向的
		   {

	        //-这个串口3,发送的内容全部来自串口2,自己不组织任何发送
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
	        	 		UART3_port_send[i] = port_send[0][the_ram_ax];		//-接收发送内容的缓存,前面是每次从缓存中提取的数据,用于发送
	        	 		port_send[2][the_ram_ax] = 0;		//-为了可靠增加,数据失效后即清0
	        	 }

	        	 port_send_dl[2] = (port_send_dl[2] + the_ram_cx) & 0xff;
	        	 port_deal_flag[2] = 0xaa;    //-保证了在发送结束之前,再次组织数据
	        	 port_send_len[2] = the_ram_cx;
             //-test_cn[1]++;    //-测试用

             //-if((test_cn[4] == 1) && (Judge_Time_In_MainLoop(test_cn_wait_time,2)==YES))
             //-{
             //-   test_cn[4] = 2;
             //-}
	        }

		   }
 	 }

}


