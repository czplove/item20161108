#include "user_conf.h"

extern u8 USART2MemoryBuffer[3];
extern u8 USART2ReceiveBuffer[30];




void uart2_to_EXTI(void)
{
	EXTI_InitTypeDef   EXTI_InitStructure;
  GPIO_InitTypeDef   GPIO_InitStructure;
  //-NVIC_InitTypeDef   NVIC_InitStructure;

  //����һ:����GPIOCʱ�Ӻ͸�������IOʱ��
  //-RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO, ENABLE);

  //�����������PC0Ϊ��������
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  //������������EXTI0�ⲿ�ж��ߵ��˿�PC0
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource3);

  //�����ģ�����EXTI3�ⲿ�ж���
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
  //-EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;		//-������
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;		//-�½���
  EXTI_InitStructure.EXTI_LineCmd = DISABLE;
  EXTI_Init(&EXTI_InitStructure);

  /* ��2������USART Tx��GPIO����Ϊ���츴��ģʽ */
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
  port_recv_pt[1] = 256 - DMA_GetCurrDataCounter(DMA1_Channel6);	//-�õ����ո���,�����Ϊ����ƫ�����Ļ�,���ڵ���ֵ����ָ�����ռ��
  if(port_recv_pt[1] >= 256)
  	port_recv_pt[1] = 0;
}

void uart2_Main(void)
{
  WORD temp_dl,temp_pt;

  temp_dl = port_recv_dl[1];
  temp_pt = port_recv_pt[1];

  if(temp_pt < temp_dl) //-ת����һȦ
    temp_pt += 256;


  if(temp_pt >= (temp_dl + 3))
  {//-�����㹻�Ŀռ���,���ڿ��Դ�����ȥ
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

        //-�����ǿ�����Ч����,Ҳ���Կ����о���
        STOP_status2 = 1; //-��������ʾ
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
//-��ZigBeeͨѶUART1
//-�Ժ�ʹ�ô���ȫ��ʹ��DMA��ʽ,�����ô��ڽ����ж�,������������ж�����̫��
void uart1_config(void)   //-ʹ��DMA��ʽ,���ò�ѯ
{
	USART_InitTypeDef USART_InitStructure;

	//-���ÿ��ܵ�DMA
	DMA_InitTypeDef DMA_InitStructure;

  //-���Գ�ʼ��������
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
     ����:
          ��׼���õ�����һ������DMA���ͳ�ȥ,ÿ�ζ���Ҫ�޸ĳ���.������Է�����ѭ���и�,�Ƿ��������Ϳ������ж��в�ѯ
     ����:
          ����һ��512�Ľ��ջ�����,���DMA���͵�����,Ȼ���ȡ����.����DMA�����ڴ����,���ú�ָ��Ĺ�ϵ,�ҾͿ���ģ���
          һ��512��FIFO.
  */
  /* USARTy TX DMA1 Channel (triggered by USARTy Tx event) Config */
  DMA_DeInit(DMA1_Channel4);  //-���ǰ����еĳ�ʼ��ΪĬ��ֵ
  DMA_InitStructure.DMA_PeripheralBaseAddr = USART1_DR_Address;		//-�����ַ(USART1):0x4001 3800 - 0x4001 3BFF + 0x04(�Ĵ���ƫ�Ƶ�ַ) = ������Ե�ַ
  DMA_InitStructure.DMA_MemoryBaseAddr = (UINT32)&port_send[0][0];   //-�洢����ַ
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;            //-���ݴ��䷽��,�������
  DMA_InitStructure.DMA_BufferSize = 1;             //-���ͻ������ߴ�,���ݴ�������
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //-�����ַ����ģʽ,��ִ�������ַ��������
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;				//-�洢����ģʽ
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;   //-�������ݿ��,8λ
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;    //-�洢�����ݿ��,8λ
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;     //-ѭ��ģʽ,ִ��ѭ������,���ݴ������Ŀ��Ϊ0ʱ�������Զ��ر��ָ�������ͨ��ʱ���õĳ�ֵ
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;   //-ͨ�����ȼ�,��?�������ȼ���ô��:�ܹ���4�����ȼ�ͬһ��ģ����
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;   //-�洢�����洢��ģʽ,�Ǵ洢�����洢��ģʽ
  DMA_Init(DMA1_Channel4, &DMA_InitStructure);

  /* USARTy RX DMA1 Channel (triggered by USARTy Rx event) Config */
  DMA_DeInit(DMA1_Channel5);
  DMA_InitStructure.DMA_PeripheralBaseAddr = USART1_DR_Address;
  DMA_InitStructure.DMA_MemoryBaseAddr = (UINT32)&port_recv[0][0];
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = 256;   //-���տռ�
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;		//-��Ҫ����ѭ��,��DMA�Զ�ȫ������
  DMA_Init(DMA1_Channel5, &DMA_InitStructure);

	//-END

	//-��������Ĵ���,ͬ��ͬ����ͬһ���˿����ǿ������õ�
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
  //-USART_InitStructure.USART_Parity = USART_Parity_Even; //-żУ��
  USART_InitStructure.USART_Parity = USART_Parity_No;		//-��У��
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  /* Configure USART1 */
  USART_Init(USART1, &USART_InitStructure);
	//-���ʹ���жϷ�ʽ,������Ҫ��������
  /* Enable the USART1 */
  USART_Cmd(USART1, ENABLE);  //-ʹ����������
  while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
	{ //-����0˵�����ͻ�������������û�з��ͳ�ȥ,������Ҫ�ȴ�ֱ�����Ϳ�
	}


	/* Enable USARTy DMA TX request */
  USART_DMACmd(USART1, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE);

  //-ͨ��ʹ�� ����
  DMA_Cmd(DMA1_Channel5, ENABLE);
  /* Enable USARTy DMA TX Channel */
  DMA_Cmd(DMA1_Channel4, ENABLE);

}


void uart1_to_EXTI(void)
{
	EXTI_InitTypeDef   EXTI_InitStructure;
  GPIO_InitTypeDef   GPIO_InitStructure;
  //-NVIC_InitTypeDef   NVIC_InitStructure;

  //����һ:����GPIOCʱ�Ӻ͸�������IOʱ��
  //-RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO, ENABLE);

  //�����������PC0Ϊ��������
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  //������������EXTI0�ⲿ�ж��ߵ��˿�PC0
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource10);

  //�����ģ�����EXTI3�ⲿ�ж���
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
  //-EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;		//-������
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;		//-�½���
  EXTI_InitStructure.EXTI_LineCmd = DISABLE;
  EXTI_Init(&EXTI_InitStructure);

  /* ��2������USART Tx��GPIO����Ϊ���츴��ģʽ */
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
  port_recv_pt[0] = 256 - DMA_GetCurrDataCounter(DMA1_Channel5);	//-�õ����ո���,�����Ϊ����ƫ�����Ļ�,���ڵ���ֵ����ָ�����ռ��
  if(port_recv_pt[0] >= 256)
  	port_recv_pt[0] = 0;
}

void UART1_Tx_Deal(void)
{
  /* Enable USARTy DMA TX request */
  //-USART_DMACmd(USARTy, USART_DMAReq_Tx, ENABLE);			//-����DMA����

  if(port_send_len[0] != 0)	//-һ��Ҫ��֤���ݷ��ͳ�ȥ��,��ʹ��DMA��������
  {
		  DMA_Cmd(DMA1_Channel4, DISABLE);
		  DMA1_Channel4->CNDTR = port_send_len[0]; //-���������Ĵ���,ָʾʣ��Ĵ������ֽ���Ŀ
		  DMA_Cmd(DMA1_Channel4, ENABLE);
		  port_send_len[0] = 0;
	}
}


///////////////////////////////////////////////////////////////////////////////
//-��������ͨѶRF-WM-3200B1
void uart3_config(void)   //-ʹ��DMA��ʽ,���ò�ѯ
{
  USART_InitTypeDef USART_InitStructure;

	//-���ÿ��ܵ�DMA
	DMA_InitTypeDef DMA_InitStructure;

  DMA_DeInit(DMA1_Channel2);  //-���ǰ����еĳ�ʼ��ΪĬ��ֵ
  DMA_InitStructure.DMA_PeripheralBaseAddr = USART3_DR_Address;		//-�����ַ(USART1):0x4001 3800 - 0x4001 3BFF + 0x04(�Ĵ���ƫ�Ƶ�ַ) = ������Ե�ַ
  DMA_InitStructure.DMA_MemoryBaseAddr = (UINT32)&UART3_port_send[0];   //-�洢����ַ
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;            //-���ݴ��䷽��,�������
  DMA_InitStructure.DMA_BufferSize = 1;             //-���ͻ������ߴ�,���ݴ�������
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //-�����ַ����ģʽ,��ִ�������ַ��������
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;				//-�洢����ģʽ
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;   //-�������ݿ��,8λ
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;    //-�洢�����ݿ��,8λ
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;     //-ѭ��ģʽ,ִ��ѭ������,���ݴ������Ŀ��Ϊ0ʱ�������Զ��ر��ָ�������ͨ��ʱ���õĳ�ֵ
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;   //-ͨ�����ȼ�,��?�������ȼ���ô��:�ܹ���4�����ȼ�ͬһ��ģ����
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;   //-�洢�����洢��ģʽ,�Ǵ洢�����洢��ģʽ
  DMA_Init(DMA1_Channel2, &DMA_InitStructure);

  //- USARTy RX DMA1 Channel (triggered by USARTy Rx event) Config
  DMA_DeInit(DMA1_Channel3);
  DMA_InitStructure.DMA_PeripheralBaseAddr = USART3_DR_Address;
  DMA_InitStructure.DMA_MemoryBaseAddr = (UINT32)&port_recv[1][0];
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = 256;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;		//-��Ҫ����ѭ��,��DMA�Զ�ȫ������
  DMA_Init(DMA1_Channel3, &DMA_InitStructure);
	//-USART3 configuration
	USART_InitStructure.USART_BaudRate = 9600; //-38400 115200
 //USART_InitStructure.USART_BaudRate = 9600;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  //USART_InitStructure.USART_Parity = USART_Parity_Even; //-żУ��
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USART3, &USART_InitStructure);
  USART_Cmd(USART3, ENABLE);
  while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET)
	{ //-����0˵�����ͻ�������������û�з��ͳ�ȥ,������Ҫ�ȴ�ֱ�����Ϳ�
	}

	//- Enable USARTy DMA TX request
  USART_DMACmd(USART3, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE);

  //-ͨ��ʹ�� ����
  DMA_Cmd(DMA1_Channel3, ENABLE);
  //- Enable USARTy DMA TX Channel
  DMA_Cmd(DMA1_Channel2, ENABLE);
}


void uart3_to_EXTI(void)
{
	EXTI_InitTypeDef   EXTI_InitStructure;
  GPIO_InitTypeDef   GPIO_InitStructure;
  //-NVIC_InitTypeDef   NVIC_InitStructure;

  //����һ:����GPIOCʱ�Ӻ͸�������IOʱ��
  //-RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO, ENABLE);

  //�����������PC0Ϊ��������
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  //������������EXTI0�ⲿ�ж��ߵ��˿�PC0
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource11);

  //�����ģ�����EXTI3�ⲿ�ж���
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
  //-EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;		//-������
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;		//-�½���
  EXTI_InitStructure.EXTI_LineCmd = DISABLE;
  EXTI_Init(&EXTI_InitStructure);

  /* ��2������USART Tx��GPIO����Ϊ���츴��ģʽ */
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
  port_recv_pt[2] = 256 - DMA_GetCurrDataCounter(DMA1_Channel3);	//-�õ����ո���,�����Ϊ����ƫ�����Ļ�,���ڵ���ֵ����ָ�����ռ��
  if(port_recv_pt[2] >= 256)
  	port_recv_pt[2] = 0;
}

void UART3_Tx_Deal(void)
{
  /* Enable USARTy DMA TX request */
  //-USART_DMACmd(USARTy, USART_DMAReq_Tx, ENABLE);			//-����DMA����

  if(port_send_len[2] != 0)	//-һ��Ҫ��֤���ݷ��ͳ�ȥ��,��ʹ��DMA��������
  {
		  DMA_Cmd(DMA1_Channel2, DISABLE);
		  DMA1_Channel2->CNDTR = port_send_len[2]; //-���������Ĵ���,ָʾʣ��Ĵ������ֽ���Ŀ
		  DMA_Cmd(DMA1_Channel2, ENABLE);
		  port_send_len[2] = 0;
	}
}

