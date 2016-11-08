#include "user_conf.h"

extern u8 USART2MemoryBuffer[3];
extern u8 USART2ReceiveBuffer[30];




void uart2_to_EXTI(void)
{
	EXTI_InitTypeDef   EXTI_InitStructure;
  GPIO_InitTypeDef   GPIO_InitStructure;
  //-NVIC_InitTypeDef   NVIC_InitStructure;

  //步骤一:开启GPIOC时钟和辅助功能IO时钟
  //-RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO, ENABLE);

  //步骤二：配置PC0为浮空输入
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  //步骤三：连接EXTI0外部中断线到端口PC0
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource3);

  //步骤四：配置EXTI3外部中断线
  EXTI_InitStructure.EXTI_Line = EXTI_Line3;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

}

void EXTI_to_uart2(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
  //-USART_InitTypeDef USART_InitStructure;
  EXTI_InitTypeDef   EXTI_InitStructure;


  //- Configure EXTI Line12 to generate an interrupt on falling edge
  EXTI_InitStructure.EXTI_Line = EXTI_Line3;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  //-EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;		//-上升沿
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;		//-下降沿
  EXTI_InitStructure.EXTI_LineCmd = DISABLE;
  EXTI_Init(&EXTI_InitStructure);

  /* 第2步：将USART Tx的GPIO配置为推挽复用模式 */
  //-GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  //-GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  //-GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  //-GPIO_Init(GPIOA, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

}

void UART2_Rx_Deal(void)
{
  port_recv_pt[1] = 256 - DMA_GetCurrDataCounter(DMA1_Channel6);	//-得到接收个数,如果作为数组偏移量的话,现在的数值就是指向待存空间的
  if(port_recv_pt[1] >= 256)
  	port_recv_pt[1] = 0;
}

void uart2_Main(void)
{
  WORD temp_dl,temp_pt;

  temp_dl = port_recv_dl[1];
  temp_pt = port_recv_pt[1];

  if(temp_pt < temp_dl) //-转过了一圈
    temp_pt += 256;


  if(temp_pt >= (temp_dl + 3))
  {//-有了足够的空间了,现在可以处理舍去
    if(port_recv[1][temp_dl + 0]==0x90)
    {
      if(port_recv[1][(temp_dl + 2) & 0xFF]==0x97)
      {
        //-memcpy(USART2MemoryBuffer,&USART2ReceiveBuffer[Touch_Count2_pt],3);
        //-memset(&USART2ReceiveBuffer[Touch_Count2_pt],0,3);
        //-memset(&USART2ReceiveBuffer[0],0,30);
        //-Touch_Count2 = 0;
        USART2MemoryBuffer[0] = port_recv[1][temp_dl + 0];
        USART2MemoryBuffer[1] = (port_recv[1][temp_dl + 1] & 0xFF);
        USART2MemoryBuffer[2] = (port_recv[1][temp_dl + 2] & 0xFF);
        port_recv_dl[1] = ((port_recv_dl[1] + 3) & 0xFF);

        //-这里是考虑有效才算,也可以考虑有就算
        STOP_status2 = 1; //-点阵屏显示
        STOP_wait_time2 = 0;
        Touch_wait_time = cticks_5ms;
        STOP_wait_time = cticks_5ms;
      }
      else
      {
        //-USART2ReceiveBuffer[Touch_Count2_pt + 0]=0;
        port_recv_dl[1] = ((port_recv_dl[1] + 1) & 0xFF);
      }
    }
    else
    {
        port_recv_dl[1] = ((port_recv_dl[1] + 1) & 0xFF);
    }
  }

}

///////////////////////////////////////////////////////////////////////////////
//-和ZigBee通讯UART1
//-以后使用串口全部使用DMA方式,不采用串口接收中断,这个反复进入中断限制太多
void uart1_config(void)   //-使用DMA方式,采用查询
{
	USART_InitTypeDef USART_InitStructure;

	//-配置可能的DMA
	DMA_InitTypeDef DMA_InitStructure;

  //-测试初始发送数据
  /*port_send[0][0] = 0xaa;
  port_send[0][1] = 0x55;
  port_send[0][2] = 0x01;
  port_send[0][3] = 0x42;
  port_send[0][4] = 0x20;
  port_send[0][5] = 0x00;
  port_send[0][6] = 0x00;
  port_send[0][7] = 0x01;

  port_send[0][8] = 0x7c;
  port_send[0][9] = 0xd7;*/

  /*
     发送:
          把准备好的内容一次性让DMA发送出去,每次都需要修改长度.处理可以放在主循环中干,是否启动发送可以在中断中查询
     接收:
          开辟一个512的接收缓冲区,检查DMA传送的数量,然后读取数据.接收DMA是周期处理的,运用好指针的关系,我就可以模拟出
          一个512的FIFO.
  */
  /* USARTy TX DMA1 Channel (triggered by USARTy Tx event) Config */
  DMA_DeInit(DMA1_Channel4);  //-就是把所有的初始化为默认值
  DMA_InitStructure.DMA_PeripheralBaseAddr = USART1_DR_Address;		//-外设地址(USART1):0x4001 3800 - 0x4001 3BFF + 0x04(寄存器偏移地址) = 外设绝对地址
  DMA_InitStructure.DMA_MemoryBaseAddr = (UINT32)&port_send[0][0];   //-存储器地址
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;            //-数据传输方向,从外设读
  DMA_InitStructure.DMA_BufferSize = 1;             //-发送缓冲区尺寸,数据传输数量
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //-外设地址增量模式,不执行外设地址增量操作
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;				//-存储器增模式
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;   //-外设数据宽度,8位
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;    //-存储器数据宽度,8位
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;     //-循环模式,执行循环操作,数据传输的数目变为0时，将会自动地被恢复成配置通道时设置的初值
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;   //-通道优先级,高?几个优先级怎么办:总共有4个优先级同一个模块上
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;   //-存储器到存储器模式,非存储器到存储器模式
  DMA_Init(DMA1_Channel4, &DMA_InitStructure);

  /* USARTy RX DMA1 Channel (triggered by USARTy Rx event) Config */
  DMA_DeInit(DMA1_Channel5);
  DMA_InitStructure.DMA_PeripheralBaseAddr = USART1_DR_Address;
  DMA_InitStructure.DMA_MemoryBaseAddr = (UINT32)&port_recv[0][0];
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = 256;   //-接收空间
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;		//-需要周期循环,让DMA自动全部接收
  DMA_Init(DMA1_Channel5, &DMA_InitStructure);

	//-END

	//-对于下面的串口,同不同步在同一个端口上是可以设置的
  /* USART1 configuration ------------------------------------------------------*/
  /* USART and USART2 configured as follow:
        - BaudRate = 115200 baud
        - Word Length = 8 Bits
        - One Stop Bit
        - Even parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
  */
  USART_InitStructure.USART_BaudRate = 9600;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  //-USART_InitStructure.USART_Parity = USART_Parity_Even; //-偶校验
  USART_InitStructure.USART_Parity = USART_Parity_No;		//-无校验
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  /* Configure USART1 */
  USART_Init(USART1, &USART_InitStructure);
	//-如果使用中断方式,这里需要增加设置
  /* Enable the USART1 */
  USART_Cmd(USART1, ENABLE);  //-使能整个外设
  while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
	{ //-对于0说明发送缓冲区还有数据没有发送出去,所以需要等待直到发送空
	}


	/* Enable USARTy DMA TX request */
  USART_DMACmd(USART1, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE);

  //-通道使能 接收
  DMA_Cmd(DMA1_Channel5, ENABLE);
  /* Enable USARTy DMA TX Channel */
  DMA_Cmd(DMA1_Channel4, ENABLE);

}


void uart1_to_EXTI(void)
{
	EXTI_InitTypeDef   EXTI_InitStructure;
  GPIO_InitTypeDef   GPIO_InitStructure;
  //-NVIC_InitTypeDef   NVIC_InitStructure;

  //步骤一:开启GPIOC时钟和辅助功能IO时钟
  //-RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO, ENABLE);

  //步骤二：配置PC0为浮空输入
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  //步骤三：连接EXTI0外部中断线到端口PC0
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource10);

  //步骤四：配置EXTI3外部中断线
  EXTI_InitStructure.EXTI_Line = EXTI_Line10;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

}

void EXTI_to_uart1(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
  //-USART_InitTypeDef USART_InitStructure;
  EXTI_InitTypeDef   EXTI_InitStructure;


  //- Configure EXTI Line12 to generate an interrupt on falling edge
  EXTI_InitStructure.EXTI_Line = EXTI_Line10;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  //-EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;		//-上升沿
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;		//-下降沿
  EXTI_InitStructure.EXTI_LineCmd = DISABLE;
  EXTI_Init(&EXTI_InitStructure);

  /* 第2步：将USART Tx的GPIO配置为推挽复用模式 */
  //-GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  //-GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  //-GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  //-GPIO_Init(GPIOA, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

}


void UART1_Rx_Deal(void)
{
  port_recv_pt[0] = 256 - DMA_GetCurrDataCounter(DMA1_Channel5);	//-得到接收个数,如果作为数组偏移量的话,现在的数值就是指向待存空间的
  if(port_recv_pt[0] >= 256)
  	port_recv_pt[0] = 0;
}

void UART1_Tx_Deal(void)
{
  /* Enable USARTy DMA TX request */
  //-USART_DMACmd(USARTy, USART_DMAReq_Tx, ENABLE);			//-启动DMA发送

  if(port_send_len[0] != 0)	//-一定要保证内容发送出去了,再使用DMA发送数据
  {
		  DMA_Cmd(DMA1_Channel4, DISABLE);
		  DMA1_Channel4->CNDTR = port_send_len[0]; //-传输数量寄存器,指示剩余的待传输字节数目
		  DMA_Cmd(DMA1_Channel4, ENABLE);
		  port_send_len[0] = 0;
	}
}


///////////////////////////////////////////////////////////////////////////////
//-和无线网通讯RF-WM-3200B1
void uart3_config(void)   //-使用DMA方式,采用查询
{
  USART_InitTypeDef USART_InitStructure;

	//-配置可能的DMA
	DMA_InitTypeDef DMA_InitStructure;

  DMA_DeInit(DMA1_Channel2);  //-就是把所有的初始化为默认值
  DMA_InitStructure.DMA_PeripheralBaseAddr = USART3_DR_Address;		//-外设地址(USART1):0x4001 3800 - 0x4001 3BFF + 0x04(寄存器偏移地址) = 外设绝对地址
  DMA_InitStructure.DMA_MemoryBaseAddr = (UINT32)&UART3_port_send[0];   //-存储器地址
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;            //-数据传输方向,从外设读
  DMA_InitStructure.DMA_BufferSize = 1;             //-发送缓冲区尺寸,数据传输数量
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //-外设地址增量模式,不执行外设地址增量操作
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;				//-存储器增模式
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;   //-外设数据宽度,8位
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;    //-存储器数据宽度,8位
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;     //-循环模式,执行循环操作,数据传输的数目变为0时，将会自动地被恢复成配置通道时设置的初值
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;   //-通道优先级,高?几个优先级怎么办:总共有4个优先级同一个模块上
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;   //-存储器到存储器模式,非存储器到存储器模式
  DMA_Init(DMA1_Channel2, &DMA_InitStructure);

  //- USARTy RX DMA1 Channel (triggered by USARTy Rx event) Config
  DMA_DeInit(DMA1_Channel3);
  DMA_InitStructure.DMA_PeripheralBaseAddr = USART3_DR_Address;
  DMA_InitStructure.DMA_MemoryBaseAddr = (UINT32)&port_recv[1][0];
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = 256;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;		//-需要周期循环,让DMA自动全部接收
  DMA_Init(DMA1_Channel3, &DMA_InitStructure);
	//-USART3 configuration
	USART_InitStructure.USART_BaudRate = 9600; //-38400 115200
 //USART_InitStructure.USART_BaudRate = 9600;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  //USART_InitStructure.USART_Parity = USART_Parity_Even; //-偶校验
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USART3, &USART_InitStructure);
  USART_Cmd(USART3, ENABLE);
  while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET)
	{ //-对于0说明发送缓冲区还有数据没有发送出去,所以需要等待直到发送空
	}

	//- Enable USARTy DMA TX request
  USART_DMACmd(USART3, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE);

  //-通道使能 接收
  DMA_Cmd(DMA1_Channel3, ENABLE);
  //- Enable USARTy DMA TX Channel
  DMA_Cmd(DMA1_Channel2, ENABLE);
}


void uart3_to_EXTI(void)
{
	EXTI_InitTypeDef   EXTI_InitStructure;
  GPIO_InitTypeDef   GPIO_InitStructure;
  //-NVIC_InitTypeDef   NVIC_InitStructure;

  //步骤一:开启GPIOC时钟和辅助功能IO时钟
  //-RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO, ENABLE);

  //步骤二：配置PC0为浮空输入
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  //步骤三：连接EXTI0外部中断线到端口PC0
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource11);

  //步骤四：配置EXTI3外部中断线
  EXTI_InitStructure.EXTI_Line = EXTI_Line11;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

}

void EXTI_to_uart3(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
  //-USART_InitTypeDef USART_InitStructure;
  EXTI_InitTypeDef   EXTI_InitStructure;


  //- Configure EXTI Line12 to generate an interrupt on falling edge
  EXTI_InitStructure.EXTI_Line = EXTI_Line11;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  //-EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;		//-上升沿
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;		//-下降沿
  EXTI_InitStructure.EXTI_LineCmd = DISABLE;
  EXTI_Init(&EXTI_InitStructure);

  /* 第2步：将USART Tx的GPIO配置为推挽复用模式 */
  //-GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  //-GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  //-GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  //-GPIO_Init(GPIOA, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

}


void UART3_Rx_Deal(void)
{
  port_recv_pt[2] = 256 - DMA_GetCurrDataCounter(DMA1_Channel3);	//-得到接收个数,如果作为数组偏移量的话,现在的数值就是指向待存空间的
  if(port_recv_pt[2] >= 256)
  	port_recv_pt[2] = 0;
}

void UART3_Tx_Deal(void)
{
  /* Enable USARTy DMA TX request */
  //-USART_DMACmd(USARTy, USART_DMAReq_Tx, ENABLE);			//-启动DMA发送

  if(port_send_len[2] != 0)	//-一定要保证内容发送出去了,再使用DMA发送数据
  {
		  DMA_Cmd(DMA1_Channel2, DISABLE);
		  DMA1_Channel2->CNDTR = port_send_len[2]; //-传输数量寄存器,指示剩余的待传输字节数目
		  DMA_Cmd(DMA1_Channel2, ENABLE);
		  port_send_len[2] = 0;
	}
}

