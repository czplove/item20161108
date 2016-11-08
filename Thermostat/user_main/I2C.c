/**
  ******************************************************************************
  * @file OptimizedI2Cexamples/src/I2CRoutines.c
  * @author  MCD Application Team
  * @version  V4.0.0
  * @date  06/18/2010
  * @brief  Contains the I2Cx slave/Master read and write routines.
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2010 STMicroelectronics</center></h2>
  */

/* Includes ------------------------------------------------------------------*/
#include "user_conf.h"

/** @addtogroup Optimized I2C examples
  * @{
  */

//-������Ӳ���ӿ�I2C
/*
  �趨����
  
*/

#define I2C_DIRECTION_TX	      1		//-Ϊ�˱����ͨ����ʱ���ӵĶ���,ʵ��ʹ��ʱ��Ҫ�޸�
#define DMA	                    1
#define Polling	                2
#define OwnAddress1             0x30
#define OwnAddress2             0x31
#define ClockSpeed              100000



/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* I2C SPE mask */
#define CR1_PE_Set              ((u16)0x0001)
#define CR1_PE_Reset            ((u16)0xFFFE)

/* I2C START mask */
#define CR1_START_Set           ((u16)0x0100)
#define CR1_START_Reset         ((u16)0xFEFF)

/* I2C STOP mask */
#define CR1_STOP_Set            ((u16)0x0200)
#define CR1_STOP_Reset          ((u16)0xFDFF)

/* I2C ACK mask */
#define CR1_ACK_Set             ((u16)0x0400)
#define CR1_ACK_Reset           ((u16)0xFBFF)

/* I2C ENGC mask */
#define CR1_ENGC_Set            ((u16)0x0040)
#define CR1_ENGC_Reset          ((u16)0xFFBF)

/* I2C SWRST mask */
#define CR1_SWRST_Set           ((u16)0x8000)
#define CR1_SWRST_Reset         ((u16)0x7FFF)

/* I2C PEC mask */
#define CR1_PEC_Set             ((u16)0x1000)
#define CR1_PEC_Reset           ((u16)0xEFFF)

/* I2C ENPEC mask */
#define CR1_ENPEC_Set           ((u16)0x0020)
#define CR1_ENPEC_Reset         ((u16)0xFFDF)

/* I2C ENARP mask */
#define CR1_ENARP_Set           ((u16)0x0010)
#define CR1_ENARP_Reset         ((u16)0xFFEF)

/* I2C NOSTRETCH mask */
#define CR1_NOSTRETCH_Set       ((u16)0x0080)
#define CR1_NOSTRETCH_Reset     ((u16)0xFF7F)

/* I2C registers Masks */
#define CR1_CLEAR_Mask          ((u16)0xFBF5)

/* I2C DMAEN mask */
#define CR2_DMAEN_Set           ((u16)0x0800)
#define CR2_DMAEN_Reset         ((u16)0xF7FF)

/* I2C LAST mask */
#define CR2_LAST_Set            ((u16)0x1000)
#define CR2_LAST_Reset          ((u16)0xEFFF)

/* I2C FREQ mask */
#define CR2_FREQ_Reset          ((u16)0xFFC0)

/* I2C ADD0 mask */
#define OAR1_ADD0_Set           ((u16)0x0001)
#define OAR1_ADD0_Reset         ((u16)0xFFFE)

/* I2C ENDUAL mask */
#define OAR2_ENDUAL_Set         ((u16)0x0001)
#define OAR2_ENDUAL_Reset       ((u16)0xFFFE)

/* I2C ADD2 mask */
#define OAR2_ADD2_Reset         ((u16)0xFF01)

/* I2C F/S mask */
#define CCR_FS_Set              ((u16)0x8000)

/* I2C CCR mask */
#define CCR_CCR_Set             ((u16)0x0FFF)

/* I2C FLAG mask */
#define FLAG_Mask               ((u32)0x00FFFFFF)

/* I2C Interrupt Enable mask */
#define ITEN_Mask               ((u32)0x07000000)


#define TWI_NOP                  TWI_Delay()
#define TWI_NACK                 0
#define TWI_ACK                  1
#define TWI_READY                0
#define TWI_BUS_BUSY             1
#define TWI_BUS_ERROR            0



/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
DMA_InitTypeDef  I2CDMA_InitStructure;
I2C_InitTypeDef  I2C_InitStructure;
UINT32 I2CDirection = I2C_DIRECTION_TX;
UINT32 NumbOfBytes1;
UINT32 NumbOfBytes2;
UINT8 Address;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
//-���ʹ��ģ��I2C�Ͳ�������ĳ���,���������������˿ڵ�
//-ģ���Ӧ�ò���������������,���������߱���ռ,��β������ܳɹ�,��ʱ���ٶ�д������
//-����ʲô�Ļ�������˵
//-����ģ��ʱ�жϵĴ���,�Ҿ���Ŀǰ����Ҫ�ر��ж�,ģ���ź���ͬ��ʱ��,ֻҪ��Ӱ��ʱ�����
//-����������߼�������(����ģ���),�����Ҫ�����Ļ�,Ҳ������ʱ����
#ifdef I2C_SOFTWARE_FLAG
//-����������ܻ���Ϊ��IO����Ϊ��©���,��ѹ����ǯλ,�����������³�ʼ�����ָ���ƽ

//-I2C��ͬ��ʱ��,�������ǿ�����������,���˿��������������ⷴӦ������
//-��̬��Ϊ����,һֱ���ڷ���״̬,��Ҫʱ�л�Ϊ����,�����л�Ϊ��̬
u8 I2C1_START(void)			//-����æ(�������޷�����);������ʼ�ź�ʧ��(�������޷�����)
{
	  //-������Ҫ��֤�����ǿ���
		I2C1_SDA_H;		//-ȫ������û��ʱ��Ҫ������������ʱ		
		I2C1_SCL_H;
		TWI_NOP;		
		//-if(!I2C1_SDA_read)		//-��������ʹ������ģʽ,��ô������Ȳ����˷��򷴸��л�
		//-{
		//-	////DebugPrint("TWI_START:BUSY\n");
		//-	return TWI_BUS_BUSY;
		//-}
		//-��ʼ������ʼ�ź�
		I2C1_SDA_L;
		TWI_NOP;
		I2C1_SCL_L;
		TWI_NOP;
		//-��֤��ʼ�źŷ��ͳɹ�		
		//-if(I2C1_SDA_read)
		//-{
		//-	////DebugPrint("TWI_START:BUS ERROR\n");
		//-	return TWI_BUS_ERROR;
		//-}
		return TWI_READY;
}

void I2C1_STOP(void)
{
	  //-�ø��õͽ�����Ϊ�˱�֤����,��������һ���ı���״̬,Ҳ����ǰ�������״̬
		I2C1_SDA_L;
    I2C1_SCL_L;		//-���Ա��յ����������ԭ���ǵ͵�������û��Ӱ���
		TWI_NOP;
		I2C1_SCL_H;
		TWI_NOP;
		//-��ʼ���ͽ����ź�
		I2C1_SDA_H;
		TWI_NOP;		
		//////DebugPrint("TWI_STOP\n");
}

void I2C1_SendACK(void)
{
	  I2C1_SCL_L;		//-����û��Ҳ��,������Ϊ�˿ɿ�
		I2C1_SDA_L;		//-�͵�ƽʱ�ı�����
		TWI_NOP;
		I2C1_SCL_H;		//-�ߵ�ƽʱ������
		TWI_NOP;
		I2C1_SCL_L;		//-����ʱ���ź�
		TWI_NOP;
		//////DebugPrint("TWI_SendACK\n");
}

void I2C1_SendNACK(void)
{
	  I2C1_SCL_L;		//-����û��Ҳ��,������Ϊ�˿ɿ�
		I2C1_SDA_H;
		TWI_NOP;
		I2C1_SCL_H;
		TWI_NOP;
		I2C1_SCL_L;
		TWI_NOP;
		//////DebugPrint("TWI_SendNACK\n");
}

u8 I2C1_SendByte(u8 Data)	//-���ص�ֵ�����˴ӻ�Ӧ�������(ACK OR NACK)
{
		u8 i,ucErrTime=0;
		
		I2C1_SCL_L;		//-�������ݵĵ�һ����������ʱ��,���ʱ��ſ��Ըı�����,�ߵ�ƽ�ǲ�����ı����ݵ�
		for(i=0;i<8;i++)
		{
				//---------���ݽ���----------
				if(Data&0x80)
				{
					 I2C1_SDA_H;
				}
				else
				{
					 I2C1_SDA_L;
				}
				Data <<= 1;
				TWI_NOP;	//-��ʱ���������������ȶ�
				//---���ݽ�������һ����ʱ----
				//----����һ��������[������]
				I2C1_SCL_H;
				TWI_NOP;		//-�ߵ�ƽʱ�䱣֤�������
				I2C1_SCL_L;
				TWI_NOP;//��ʱ,��ֹSCL��û��ɵ�ʱ�ı�SDA,�Ӷ�����START/STOP�ź�
				//---------------------------
		}
		
		//���մӻ���Ӧ��
		I2C1_SDA_IN();      //SDA����Ϊ����  
		I2C1_SDA_H;		//-���������ߵ�ƽ��ʵ���趨��������������
		//-I2C1_SDA_L;
		I2C1_SCL_H;		//-�ߵ�ƽ�ڼ���������ϵ�����
		TWI_NOP;
		while(I2C1_SDA_read)
	  {
			ucErrTime++;
			if(ucErrTime>20)		//-100���ȴ���
			{
				I2C1_SCL_L;		//-��Ӧ����ǳ���,����͸ûָ�����ģʽ
				TWI_NOP;
				I2C1_SDA_OUT();//sda�����
				return TWI_NACK;
			}
	  }
		
		//-�͵�ƽ��Ӧ���ź�
		I2C1_SCL_L;
		TWI_NOP;
		I2C1_SDA_OUT();//sda�����
		//////DebugPrint("TWI_ACK!\n");
		return TWI_ACK;    
}

u8 I2C1_ReceiveByte(void)	//-���ص���8λ����ֵ
{
		u8 i,Dat;
		
		I2C1_SDA_IN();//SDA����Ϊ����
		I2C1_SDA_H;	//-�����Ŀ�Ĳ��������ƽ,������������,����
		//-I2C1_SDA_L;
		I2C1_SCL_L;	//-������Ϊ���������߿��Ըı�״̬
		Dat=0;
		for(i=0;i<8;i++)
		{
				I2C1_SCL_H;//����ʱ��������[������],�ôӻ�׼��������
				TWI_NOP;
				Dat<<=1;
				if(I2C1_SDA_read) //������״̬,,�ߵ�ƽʱ��״̬��������������Ч������
				{
				   Dat|=0x01;
				}
				I2C1_SCL_L;//׼�����ٴν�������
				TWI_NOP;//�ȴ�����׼����
		}
		//////DebugPrint("TWI_Dat:%x\n",Dat);
		I2C1_SDA_OUT();		//-�ָ�����̬
		return Dat;
}

//-ģ��I2C2
u8 I2C2_START(void)
{
	  //-������Ҫ��֤�����ǿ���
		I2C2_SDA_H;		//-ȫ������û��ʱ��Ҫ������������ʱ		
		I2C2_SCL_H;
		TWI_NOP;		
		//-if(!I2C1_SDA_read)		//-��������ʹ������ģʽ,��ô������Ȳ����˷��򷴸��л�
		//-{
		//-	////DebugPrint("TWI_START:BUSY\n");
		//-	return TWI_BUS_BUSY;
		//-}
		//-��ʼ������ʼ�ź�
		I2C2_SDA_L;
		TWI_NOP;
		I2C2_SCL_L;
		TWI_NOP;
		//-��֤��ʼ�źŷ��ͳɹ�		
		//-if(I2C1_SDA_read)
		//-{
		//-	////DebugPrint("TWI_START:BUS ERROR\n");
		//-	return TWI_BUS_ERROR;
		//-}
		return TWI_READY;
}

void I2C2_STOP(void)
{
	  //-�ø��õͽ�����Ϊ�˱�֤����,��������һ���ı���״̬,Ҳ����ǰ�������״̬
		I2C2_SDA_L;
    I2C2_SCL_L;		//-���Ա��յ����������ԭ���ǵ͵�������û��Ӱ���
		TWI_NOP;
		I2C2_SCL_H;
		TWI_NOP;
		//-��ʼ���ͽ����ź�
		I2C2_SDA_H;
		TWI_NOP;	
}

void I2C2_SendACK(void)
{
		I2C2_SCL_L;		//-����û��Ҳ��,������Ϊ�˿ɿ�
		I2C2_SDA_L;		//-�͵�ƽʱ�ı�����
		TWI_NOP;
		I2C2_SCL_H;		//-�ߵ�ƽʱ������
		TWI_NOP;
		I2C2_SCL_L;		//-����ʱ���ź�
		TWI_NOP;
}

void I2C2_SendNACK(void)
{
		I2C2_SCL_L;		//-����û��Ҳ��,������Ϊ�˿ɿ�
		I2C2_SDA_H;
		TWI_NOP;
		I2C2_SCL_H;
		TWI_NOP;
		I2C2_SCL_L;
		TWI_NOP;
}

u8 I2C2_SendByte(u8 Data)	//-���ص�ֵ�����˴ӻ�Ӧ�������(ACK OR NACK)
{
		u8 i=0;
    u8 ucErrTime=0;
		
		I2C2_SCL_L;		//-�������ݵĵ�һ����������ʱ��,���ʱ��ſ��Ըı�����,�ߵ�ƽ�ǲ�����ı����ݵ�
		for(i=0;i<8;i++)
		{
				//---------���ݽ���----------
				if(Data&0x80)
				{
					 I2C2_SDA_H;
				}
				else
				{
					 I2C2_SDA_L;
				}
				Data <<= 1;
				TWI_NOP;	//-��ʱ���������������ȶ�
				//---���ݽ�������һ����ʱ----
				//----����һ��������[������]
				I2C2_SCL_H;
				TWI_NOP;		//-�ߵ�ƽʱ�䱣֤�������
				I2C2_SCL_L;
				TWI_NOP;//��ʱ,��ֹSCL��û��ɵ�ʱ�ı�SDA,�Ӷ�����START/STOP�ź�
				//---------------------------
		}
		
		//���մӻ���Ӧ��
		I2C2_SDA_IN();      //SDA����Ϊ����  
		I2C2_SDA_H;		//-���������ߵ�ƽ��ʵ���趨��������������
		I2C2_SCL_H;		//-�ߵ�ƽ�ڼ���������ϵ�����
		TWI_NOP;
		while(I2C2_SDA_read)
	  {
			ucErrTime++;
			if(ucErrTime>20)		//-���ȴ���,����ֵΪ100ʱ��Լ�ȴ�46uS
			{
				I2C2_SCL_L;		//-��Ӧ����ǳ���,����͸ûָ�����ģʽ,����������Ҫ�������һ��ʱ������,������Ҫ����
				TWI_NOP;
				I2C2_SDA_OUT();//sda�����
				return TWI_NACK;
			}
	  }
		
		//-�͵�ƽ��Ӧ���ź�
		I2C2_SCL_L;
		TWI_NOP;
		I2C2_SDA_OUT();//sda�����
		//////DebugPrint("TWI_ACK!\n");
		return TWI_ACK;  
}

u8 I2C2_ReceiveByte(void)	//-���ص���8λ����ֵ
{
		u8 i,Dat;
		
		I2C2_SDA_IN();//SDA����Ϊ����
		I2C2_SDA_H;	//-�����Ŀ�Ĳ��������ƽ,������������,����
		I2C2_SCL_L;	//-������Ϊ���������߿��Ըı�״̬
		Dat=0;
		for(i=0;i<8;i++)
		{
				I2C2_SCL_H;//����ʱ��������[������],�ôӻ�׼��������
				TWI_NOP;
				Dat<<=1;
				if(I2C2_SDA_read) //������״̬,,�ߵ�ƽʱ��״̬��������������Ч������
				{
				   Dat|=0x01;
				}
				I2C2_SCL_L;//׼�����ٴν�������
				TWI_NOP;//�ȴ�����׼����
		}
		//////DebugPrint("TWI_Dat:%x\n",Dat);
		I2C2_SDA_OUT();		//-�ָ�����̬
		return Dat;
}

#endif

#ifdef I2C_HARDWARE_FLAG
/**
  * @brief  Reads buffer of bytes  from the slave.
  * @param pBuffer: Buffer of bytes to be read from the slave.
  * @param NumByteToRead: Number of bytes to be read by the Master.
  * @param Mode: Polling or DMA or Interrupt having the highest priority in the application.
  * @param SlaveAddress: The address of the slave to be addressed by the Master.
  * @retval : None.
  */
//-ʵ����Ӳ���Ķ�����,�����ַ�ʽ��ѡ��DMA POLLING IT,���ڽ���Ĭ����POLLING��ʽ  
int I2C_Master_BufferRead(I2C_TypeDef* I2Cx, UINT8* pBuffer,  UINT32 NumByteToRead, UINT8 Mode, UINT8 SlaveAddress)
{
    UINT32 temp = 0;
    UINT32 Timeout = 0;

    /* Enable I2C errors interrupts (used in all modes: Polling, DMA and Interrupts */
    I2Cx->CR2 |= I2C_IT_ERR;

    /*if (Mode == DMA) //- I2Cx Master Reception using DMA 
    {
        // Configure I2Cx DMA channel 
        //-I2C_DMAConfig(I2Cx, pBuffer, NumByteToRead, I2C_DIRECTION_RX);
        // Set Last bit to have a NACK on the last received byte 
        I2Cx->CR2 |= CR2_LAST_Set;
        // Enable I2C DMA requests 
        I2Cx->CR2 |= CR2_DMAEN_Set;
        Timeout = 0xFFFF;
        // Send START condition 
        I2Cx->CR1 |= CR1_START_Set;
        // Wait until SB flag is set: EV5  
        while ((I2Cx->SR1&0x0001) != 0x0001)
        {
            if (Timeout-- == 0)
                return Error;
        }
        Timeout = 0xFFFF;
        // Send slave address 
        // Set the address bit0 for read 
        SlaveAddress |= OAR1_ADD0_Set;
        Address = SlaveAddress;
        // Send the slave address 
        I2Cx->DR = Address;
        // Wait until ADDR is set: EV6 
        while ((I2Cx->SR1&0x0002) != 0x0002)
        {
            if (Timeout-- == 0)
                return Error;
        }
        // Clear ADDR flag by reading SR2 register 
        temp = I2Cx->SR2;
        if (I2Cx == I2C1)
        {
            // Wait until DMA end of transfer 
            while (!DMA_GetFlagStatus(DMA1_FLAG_TC7));
            // Disable DMA Channel 
            //-DMA_Cmd(I2C1_DMA_CHANNEL_RX, DISABLE);
            // Clear the DMA Transfer Complete flag 
            //-DMA_ClearFlag(DMA1_FLAG_TC7);

        }

        else // I2Cx = I2C2
        {
            // Wait until DMA end of transfer 
            while (!DMA_GetFlagStatus(DMA1_FLAG_TC5));
            // Disable DMA Channel 
            //-DMA_Cmd(I2C2_DMA_CHANNEL_RX, DISABLE);
            // Clear the DMA Transfer Complete flag 
            DMA_ClearFlag(DMA1_FLAG_TC5);
        }
        // Program the STOP 
        I2Cx->CR1 |= CR1_STOP_Set;
        // Make sure that the STOP bit is cleared by Hardware before CR1 write access 
        while ((I2Cx->CR1&0x200) == 0x200);
    }*/
    //-else if (Mode == Polling) // I2Cx Master Reception using Polling 
    {


        if (NumByteToRead == 1)
        {
            Timeout = 0xFFFF;
            /* Send START condition */
            I2Cx->CR1 |= CR1_START_Set;
            /* Wait until SB flag is set: EV5  */
            while ((I2Cx->SR1&0x0001) != 0x0001)
            {
                if (Timeout-- == 0)
                    return Error;
            }
            /* Send slave address */
            /* Reset the address bit0 for read */
            SlaveAddress |= OAR1_ADD0_Set;		//-��д�������һλֵ������,�����ȷ����ֵ
            Address = SlaveAddress;
            /* Send the slave address */
            I2Cx->DR = Address;
            /* Wait until ADDR is set: EV6_3, then program ACK = 0, clear ADDR
            and program the STOP just after ADDR is cleared. The EV6_3 
            software sequence must complete before the current byte end of transfer.*/
            /* Wait until ADDR is set */
            Timeout = 0xFFFF;
            while ((I2Cx->SR1&0x0002) != 0x0002)
            {
                if (Timeout-- == 0)
                    return Error;
            }
            /* Clear ACK bit */
            I2Cx->CR1 &= CR1_ACK_Reset;
            /* Disable all active IRQs around ADDR clearing and STOP programming because the EV6_3
            software sequence must complete before the current byte end of transfer */
            //-__disable_irq();     //?˼���¹ر��жϵ�����,��û�����жϹر�
            NVIC_SETPRIMASK();
            /* Clear ADDR flag */
            temp = I2Cx->SR2;
            /* Program the STOP */
            I2Cx->CR1 |= CR1_STOP_Set;
            /* Re-enable IRQs */
            //-__enable_irq();
            NVIC_RESETPRIMASK();
            /* Wait until a data is received in DR register (RXNE = 1) EV7 */
            while ((I2Cx->SR1 & 0x00040) != 0x000040);
            /* Read the data */
            *pBuffer = I2Cx->DR;
            /* Make sure that the STOP bit is cleared by Hardware before CR1 write access */
            while ((I2Cx->CR1&0x200) == 0x200);
            /* Enable Acknowledgement to be ready for another reception */
            I2Cx->CR1 |= CR1_ACK_Set;

        }
        else if (NumByteToRead == 2)
        {
            /* Set POS bit */
            //-I2Cx->CR1 |= CR1_POS_Set;
            Timeout = 0xFFFF;
            /* Send START condition */
            I2Cx->CR1 |= CR1_START_Set;
            /* Wait until SB flag is set: EV5 */
            while ((I2Cx->SR1&0x0001) != 0x0001)
            {
                if (Timeout-- == 0)
                    return Error;
            }
            Timeout = 0xFFFF;
            /* Send slave address */
            /* Set the address bit0 for read */
            SlaveAddress |= OAR1_ADD0_Set;
            Address = SlaveAddress;
            /* Send the slave address */
            I2Cx->DR = Address;
            /* Wait until ADDR is set: EV6 */
            while ((I2Cx->SR1&0x0002) != 0x0002)
            {
                if (Timeout-- == 0)
                    return Error;
            }
            /* EV6_1: The acknowledge disable should be done just after EV6,
            that is after ADDR is cleared, so disable all active IRQs around ADDR clearing and 
            ACK clearing */
            //-__disable_irq();
            NVIC_SETPRIMASK();          //-��Ч�ر����ж�
            /* Clear ADDR by reading SR2 register  */
            temp = I2Cx->SR2;
            /* Clear ACK */
            I2Cx->CR1 &= CR1_ACK_Reset;
            /*Re-enable IRQs */
            //-__enable_irq();
            NVIC_RESETPRIMASK();
            /* Wait until BTF is set */
            while ((I2Cx->SR1 & 0x00004) != 0x000004);
            /* Disable IRQs around STOP programming and data reading because of the limitation ?*/
            //-__disable_irq();
            NVIC_SETPRIMASK();
            /* Program the STOP */
            I2C_GenerateSTOP(I2Cx, ENABLE);
            /* Read first data */
            *pBuffer = I2Cx->DR;
            /* Re-enable IRQs */
            //-__enable_irq();
            NVIC_RESETPRIMASK();
            /**/
            pBuffer++;
            /* Read second data */
            *pBuffer = I2Cx->DR;
            /* Make sure that the STOP bit is cleared by Hardware before CR1 write access */
            while ((I2Cx->CR1&0x200) == 0x200);
            /* Enable Acknowledgement to be ready for another reception */
            I2Cx->CR1  |= CR1_ACK_Set;
            /* Clear POS bit */
            //-I2Cx->CR1  &= CR1_POS_Reset;

        }
        else
        {

            Timeout = 0xFFFF;
            /* Send START condition */
            I2Cx->CR1 |= CR1_START_Set;
            /* Wait until SB flag is set: EV5 */
            while ((I2Cx->SR1&0x0001) != 0x0001)
            {
                if (Timeout-- == 0)
                    return Error;
            }
            Timeout = 0xFFFF;
            /* Send slave address */
            /* Reset the address bit0 for write */
            SlaveAddress |= OAR1_ADD0_Set;;
            Address = SlaveAddress;
            /* Send the slave address */
            I2Cx->DR = Address;
            /* Wait until ADDR is set: EV6 */
            while ((I2Cx->SR1&0x0002) != 0x0002)
            {
                if (Timeout-- == 0)
                    return Error;
            }
            /* Clear ADDR by reading SR2 status register */
            temp = I2Cx->SR2;
            /* While there is data to be read */
            while (NumByteToRead)
            {
                /* Receive bytes from first byte until byte N-3 */
                if (NumByteToRead != 3)
                {
                    /* Poll on BTF to receive data because in polling mode we can not guarantee the
                    EV7 software sequence is managed before the current byte transfer completes */
                    while ((I2Cx->SR1 & 0x00004) != 0x000004);
                    /* Read data */
                    *pBuffer = I2Cx->DR;
                    /* */
                    pBuffer++;
                    /* Decrement the read bytes counter */
                    NumByteToRead--;
                }

                /* it remains to read three data: data N-2, data N-1, Data N */
                if (NumByteToRead == 3)
                {

                    /* Wait until BTF is set: Data N-2 in DR and data N -1 in shift register */
                    while ((I2Cx->SR1 & 0x00004) != 0x000004);
                    /* Clear ACK */
                    I2Cx->CR1 &= CR1_ACK_Reset;

                    /* Disable IRQs around data reading and STOP programming because of the
                    limitation ? */
                    //-__disable_irq();
                    NVIC_SETPRIMASK();
                    /* Read Data N-2 */
                    *pBuffer = I2Cx->DR;
                    /* Increment */
                    pBuffer++;
                    /* Program the STOP */
                    I2Cx->CR1 |= CR1_STOP_Set;
                    /* Read DataN-1 */
                    *pBuffer = I2Cx->DR;
                    /* Re-enable IRQs */
                    //-__enable_irq();
                    NVIC_RESETPRIMASK();
                    /* Increment */
                    pBuffer++;
                    /* Wait until RXNE is set (DR contains the last data) */
                    while ((I2Cx->SR1 & 0x00040) != 0x000040);
                    /* Read DataN */
                    *pBuffer = I2Cx->DR;
                    /* Reset the number of bytes to be read by master */
                    NumByteToRead = 0;

                }
            }
            /* Make sure that the STOP bit is cleared by Hardware before CR1 write access */
            while ((I2Cx->CR1&0x200) == 0x200);
            /* Enable Acknowledgement to be ready for another reception */
            I2Cx->CR1 |= CR1_ACK_Set;

        }

    }
    /*else // I2Cx Master Reception using Interrupts with highest priority in an application 
    {
        // Enable EVT IT
        I2Cx->CR2 |= I2C_IT_EVT;
        // Enable BUF IT 
        I2Cx->CR2 |= I2C_IT_BUF;
        // Set the I2C direction to reception 
        //-I2CDirection = I2C_DIRECTION_RX;
        SlaveAddress |= OAR1_ADD0_Set;
        Address = SlaveAddress;
        if (I2Cx == I2C1)    
        	NumbOfBytes1 = NumByteToRead;
        else 
        	NumbOfBytes2 = NumByteToRead;
        // Send START condition 
        I2Cx->CR1 |= CR1_START_Set;
        // Wait until the START condition is generated on the bus: START bit is cleared by hardware 
        while ((I2Cx->CR1&0x100) == 0x100);
        // Wait until BUSY flag is reset (until a STOP is generated) 
        while ((I2Cx->SR2 &0x0002) == 0x0002);
        // Enable Acknowledgement to be ready for another reception 
        I2Cx->CR1 |= CR1_ACK_Set;
    }*/

    return Success;
}

/**
  * @brief  Writes buffer of bytes.
  * @param pBuffer: Buffer of bytes to be sent to the slave.
  * @param NumByteToWrite: Number of bytes to be sent by the Master.
  * @param Mode: Polling or DMA or Interrupt having the highest priority in the application.
  * @param SlaveAddress: The address of the slave to be addressed by the Master.
  * @retval : None.
  */
//-Ӳ��дһϵ������,����������,�����ʵ����Ӳ���Ĳ���
int I2C_Master_BufferWrite(I2C_TypeDef* I2Cx, UINT8* pBuffer,  UINT32 NumByteToWrite, UINT8 Mode, UINT8 SlaveAddress )
{

    UINT32 temp = 0;
    UINT32 Timeout = 0;

    /* Enable Error IT (used in all modes: DMA, Polling and Interrupts */
    //-I2Cx->CR2 |= I2C_IT_ERR;
    /*if (Mode == DMA)  // I2Cx Master Transmission using DMA 
    {
        Timeout = 0xFFFF;
        // Configure the DMA channel for I2Cx transmission 
        //-I2C_DMAConfig (I2Cx, pBuffer, NumByteToWrite, I2C_DIRECTION_TX);
        // Enable the I2Cx DMA requests 
        I2Cx->CR2 |= CR2_DMAEN_Set;
        // Send START condition 
        I2Cx->CR1 |= CR1_START_Set;
        // Wait until SB flag is set: EV5 
        while ((I2Cx->SR1&0x0001) != 0x0001)
        {
            if (Timeout-- == 0)
                return Error;
        }
        Timeout = 0xFFFF;
        // Send slave address 
        // Reset the address bit0 for write 
        SlaveAddress &= OAR1_ADD0_Reset;
        Address = SlaveAddress;
        // Send the slave address 
        I2Cx->DR = Address;
        // Wait until ADDR is set: EV6 
        while ((I2Cx->SR1&0x0002) != 0x0002)
        {
            if (Timeout-- == 0)
                return Error;
        }

        // Clear ADDR flag by reading SR2 register 
        temp = I2Cx->SR2;
        if (I2Cx == I2C1)
        {
            // Wait until DMA end of transfer 
            while (!DMA_GetFlagStatus(DMA1_FLAG_TC6));
            // Disable the DMA1 Channel 6 
            //-DMA_Cmd(I2C1_DMA_CHANNEL_TX, DISABLE);
            // Clear the DMA Transfer complete flag 
            //-DMA_ClearFlag(DMA1_FLAG_TC6);
        }
        else  // I2Cx = I2C2 
        {
            // Wait until DMA end of transfer 
            while (!DMA_GetFlagStatus(DMA1_FLAG_TC4));
            // Disable the DMA1 Channel 4 
            //-DMA_Cmd(I2C2_DMA_CHANNEL_TX, DISABLE);
            // Clear the DMA Transfer complete flag
            //-DMA_ClearFlag(DMA1_FLAG_TC4);
        }

        // EV8_2: Wait until BTF is set before programming the STOP 
        while ((I2Cx->SR1 & 0x00004) != 0x000004);
        // Program the STOP 
        I2Cx->CR1 |= CR1_STOP_Set;
        // Make sure that the STOP bit is cleared by Hardware 
        while ((I2Cx->CR1&0x200) == 0x200);

    }*/
    //-else if (Mode == Polling) // I2Cx Master Transmission using Polling 
    {

        Timeout = 0xFFFF;
        /* Send START condition */
        I2Cx->CR1 |= CR1_START_Set;	//-����Ҫֱ�ӿ�������״̬,ͨ��Ӳ���ӿڼĴ����Ϳ���
        /* Wait until SB flag is set: EV5 */
        while ((I2Cx->SR1&0x0001) != 0x0001)	//-�ɹ�������ʼ�������������
        {
            if (Timeout-- == 0)
                return Error;
        }

        /* Send slave address */
        /* Reset the address bit0 for write*/
        SlaveAddress &= OAR1_ADD0_Reset;
        Address = SlaveAddress;
        /* Send the slave address */
        I2Cx->DR = Address;
        Timeout = 0xFFFF;
        /* Wait until ADDR is set: EV6 */
        while ((I2Cx->SR1 &0x0002) != 0x0002)
        {
            if (Timeout-- == 0)
                return Error;
        }

        /* Clear ADDR flag by reading SR2 register */
        temp = I2Cx->SR2;
        /* Write the first data in DR register (EV8_1) */
        I2Cx->DR = *pBuffer;
        /* Increment */
        pBuffer++;
        /* Decrement the number of bytes to be written */
        NumByteToWrite--;
        /* While there is data to be written */
        while (NumByteToWrite--)	//-���е����ݷ��ͳ�ȥ
        {
            /* Poll on BTF to receive data because in polling mode we can not guarantee the
              EV8 software sequence is managed before the current byte transfer completes */
            while ((I2Cx->SR1 & 0x00004) != 0x000004);
            /* Send the current byte */
            I2Cx->DR = *pBuffer;
            /* Point to the next byte to be written */
            pBuffer++;
        }
        /* EV8_2: Wait until BTF is set before programming the STOP */
        while ((I2Cx->SR1 & 0x00004) != 0x000004);
        /* Send STOP condition */
        I2Cx->CR1 |= CR1_STOP_Set;
        /* Make sure that the STOP bit is cleared by Hardware */
        while ((I2Cx->CR1&0x200) == 0x200);

    }
    /*else // I2Cx Master Transmission using Interrupt with highest priority in the application 
    {
        // Enable EVT IT
        I2Cx->CR2 |= I2C_IT_EVT;
        // Enable BUF IT 
        I2Cx->CR2 |= I2C_IT_BUF;
        // Set the I2C direction to Transmission 
        I2CDirection = I2C_DIRECTION_TX;
        SlaveAddress &= OAR1_ADD0_Reset;
        Address = SlaveAddress;
        if (I2Cx == I2C1)    
        	NumbOfBytes1 = NumByteToWrite;
        else 
        	NumbOfBytes2 = NumByteToWrite;
        // Send START condition 
        I2Cx->CR1 |= CR1_START_Set;
        // Wait until the START condition is generated on the bus: the START bit is cleared by hardware 
        while ((I2Cx->CR1&0x100) == 0x100);
        // Wait until BUSY flag is reset: a STOP has been generated on the bus signaling the end
        //of transmission 
        while ((I2Cx->SR2 &0x0002) == 0x0002);
    }*/

    return Success;

}

/**
  * @brief Prepares the I2Cx slave for transmission.
  * @param I2Cx: I2C1 or I2C2.
  * @param Mode: DMA or Interrupt having the highest priority in the application.
  * @retval : None.
  */
void I2C_Slave_BufferReadWrite(I2C_TypeDef* I2Cx,UINT8 Mode)
{
    /* Enable Event IT needed for ADDR and STOPF events ITs */
    I2Cx->CR2 |= I2C_IT_EVT ;
    /* Enable Error IT */
    I2Cx->CR2 |= I2C_IT_ERR;

    if (Mode == DMA)  /* I2Cx Slave Transmission using DMA */
    {
        /* Enable I2Cx DMA requests */
        I2Cx->CR2 |= CR2_DMAEN_Set;
    }
    else  /* I2Cx Slave Transmission using Interrupt with highest priority in the application */
    {
        /* Enable Buffer IT (TXE and RXNE ITs) */
        I2Cx->CR2 |= I2C_IT_BUF;

    }

}
#else		//-���ʹ��ģ��I2C��ʹ������ĳ���
//-�������ݵ�ʱ�����ָ���ӻ���ַ(��������),Ȼ���ȡ�����������ϵ����ݾ���
//-������õ���ģ��I2C��д����,����ʱ��Ͳ�����Ҫ��֤
//-����ģ��I2C����һ��ͬʱ���ڵĿ�����,���������Mode��Ϊ�˿�ѡ�����
//-�����ܹ�������������һ��������������,һ�����������װ��û��Ӧ��.
int I2C_Master_BufferRead(I2C_TypeDef* I2Cx, UINT8* pBuffer,  UINT32 NumByteToRead, UINT8 Mode, UINT8 SlaveAddress)
{
    int res;
    
    if(Mode == 1)
    {	
			  if (NumByteToRead == 1)
		    {
		        // Send START condition 
		        res = I2C1_START();
		        if(res == TWI_READY)	//-�ж������Ƿ����
		        {
		        	 i2c1_bus_error_cn = 0;
		        	 // Send slave address 
		           // Reset the address bit0 for read 
		           SlaveAddress |= OAR1_ADD0_Set;		//-��д�������һλֵ������,�����ȷ����ֵ
		           Address = SlaveAddress;
		           // Send the slave address 
		           res = I2C1_SendByte(Address);
		           if(res == TWI_ACK)
		           {//-�ӻ�Ӧ���,�Ϳ��Կ�ʼ��ȡ�����ϵ�������,����������ݺ���Ҫ����ACK�Ƿ���NACK
		           	// Read the data
		           	  *pBuffer = I2C1_ReceiveByte();
		           	  I2C1_SendNACK();
		           	  I2C1_STOP();
		           	  return Success;		//-���ճɹ�
		           }
		        }	
		        else
		        	 i2c1_bus_error_cn++;                                                    
		    }
		    else if (NumByteToRead == 2)
		    {//-Ŀǰû��һ�ζ������ֽڵ�����
		    	  // Send START condition 
		        res = I2C1_START();
		        if(res == TWI_READY)	//-�ж������Ƿ����
		        {
		        	 i2c1_bus_error_cn = 0;
		        	 // Send slave address 
		           // Reset the address bit0 for read 
		           SlaveAddress |= OAR1_ADD0_Set;		//-��д�������һλֵ������,�����ȷ����ֵ
		           Address = SlaveAddress;
		           // Send the slave address 
		           res = I2C1_SendByte(Address);
		           if(res == TWI_ACK)
		           {
		           	  // Read first data,,�����������Ƿ��������Ч������Ҫ����Ϊȥ����,����ģ���,���ܵȴ��������
		           	  *pBuffer = I2C1_ReceiveByte();
		           	  I2C1_SendACK();
		           	  pBuffer++;
		           	  // Read second data 
		              *pBuffer = I2C1_ReceiveByte();
		              I2C1_SendNACK();
		           	  I2C1_STOP();
		           	  return Success;		//-���ճɹ�
		           }
		        }	
		        else
		        	 i2c1_bus_error_cn++;    	                           
		    }
		    else
		    {    
		        // Send START condition 
		        res = I2C1_START();
		        if(res == TWI_READY)	//-�ж������Ƿ����
		        {
		        	 i2c1_bus_error_cn = 0;
		        	 // Send slave address 
		           // Reset the address bit0 for read 
		           SlaveAddress |= OAR1_ADD0_Set;		//-��д�������һλֵ������,�����ȷ����ֵ
		           Address = SlaveAddress;
		           // Send the slave address 
		           res = I2C1_SendByte(Address);
		           if(res == TWI_ACK)
		           {
		           	  // While there is data to be read 
		              while (NumByteToRead)
		              {
		        	         // Receive bytes from first byte until byte N-3
		        	         if (NumByteToRead != 3)	//-Ӳ����I2C��������ֽ���Ҫ�ر���,��Ϊ��Щ�Ĵ���������Ҫ�޸Ĳ������ó���������Ч��,ǰ��Ķ���һ����,һ������һ��Ӧ��
		                   {
		                    // Read data 
		                    *pBuffer = I2C1_ReceiveByte();
		                    I2C1_SendACK();
		                    /* */
		                    pBuffer++;
		                    // Decrement the read bytes counter 
		                    NumByteToRead--;
		                   }
		                   
		                   // it remains to read three data: data N-2, data N-1, Data N 
			                 if (NumByteToRead == 3)
			                 {
			                    // Read Data N-2 
			                    *pBuffer = I2C1_ReceiveByte();
			                    I2C1_SendACK();
			                    // Increment 
			                    pBuffer++;
			                    // Program the STOP 
			                    //-I2Cx->CR1 |= CR1_STOP_Set;		//-������ֹͣλʲô��˼
			                    /* Read DataN-1 */
			                    *pBuffer = I2C1_ReceiveByte();
			                    I2C1_SendACK();
			                    // Increment 
			                    pBuffer++;
			                    // Wait until RXNE is set (DR contains the last data)
			                    //-while ((I2Cx->SR1 & 0x00040) != 0x000040);		//-ģ���ʱ���ֱ�Ӷ���,û�������ı�־
			                    // Read DataN 
			                    *pBuffer = I2C1_ReceiveByte();
			                    I2C1_SendNACK();
			                    // Reset the number of bytes to be read by master 
			                    NumByteToRead = 0;	
			                 }
		              }
		              
		           	  I2C1_STOP();
		           	  return Success;		//-���ճɹ�
		           }
		        }  
		        else
		        	 i2c1_bus_error_cn++;                  
		    }
    }
    else if(Mode == 2)
    {
    	  if (NumByteToRead == 1)
		    {
		        // Send START condition 
		        res = I2C2_START();
		        if(res == TWI_READY)	//-�ж������Ƿ����
		        {
		        	 i2c2_bus_error_cn = 0; 
		        	 // Send slave address 
		           // Reset the address bit0 for read 
		           SlaveAddress |= OAR1_ADD0_Set;		//-��д�������һλֵ������,�����ȷ����ֵ
		           Address = SlaveAddress;
		           // Send the slave address 
		           res = I2C2_SendByte(Address);
		           if(res == TWI_ACK)
		           {//-�ӻ�Ӧ���,�Ϳ��Կ�ʼ��ȡ�����ϵ�������,����������ݺ���Ҫ����ACK�Ƿ���NACK
		           	// Read the data
		           	  *pBuffer = I2C2_ReceiveByte();
		           	  I2C2_SendNACK();
		           	  I2C2_STOP();
		           	  return Success;		//-���ճɹ�
		           }
		        }	  
		        else
		        	 i2c2_bus_error_cn++;                                                 
		    }
		    else if (NumByteToRead == 2)
		    {//-Ŀǰû��һ�ζ������ֽڵ�����
		    	  // Send START condition 
		        res = I2C2_START();
		        if(res == TWI_READY)	//-�ж������Ƿ����
		        {
		        	 i2c2_bus_error_cn = 0;
		        	 // Send slave address 
		           // Reset the address bit0 for read 
		           SlaveAddress |= OAR1_ADD0_Set;		//-��д�������һλֵ������,�����ȷ����ֵ
		           Address = SlaveAddress;
		           // Send the slave address 
		           res = I2C2_SendByte(Address);
		           if(res == TWI_ACK)
		           {
		           	  // Read first data,,�����������Ƿ��������Ч������Ҫ����Ϊȥ����,����ģ���,���ܵȴ��������
		           	  *pBuffer = I2C2_ReceiveByte();
		           	  I2C2_SendACK();
		           	  pBuffer++;
		           	  // Read second data 
		              *pBuffer = I2C2_ReceiveByte();
		              I2C2_SendNACK();
		           	  I2C2_STOP();
		           	  return Success;		//-���ճɹ�
		           }
		        }
		        else
		        	 i2c2_bus_error_cn++;	    	                           
		    }
		    else
		    {    
		        // Send START condition 
		        res = I2C2_START();
		        if(res == TWI_READY)	//-�ж������Ƿ����
		        {
		        	 i2c2_bus_error_cn = 0;
		        	 // Send slave address 
		           // Reset the address bit0 for read 
		           SlaveAddress |= OAR1_ADD0_Set;		//-��д�������һλֵ������,�����ȷ����ֵ
		           Address = SlaveAddress;
		           // Send the slave address 
		           res = I2C2_SendByte(Address);
		           if(res == TWI_ACK)
		           {
		           	  // While there is data to be read 
		              while (NumByteToRead)
		              {
		        	         // Receive bytes from first byte until byte N-3
		        	         if (NumByteToRead != 3)	//-Ӳ����I2C��������ֽ���Ҫ�ر���,��Ϊ��Щ�Ĵ���������Ҫ�޸Ĳ������ó���������Ч��,ǰ��Ķ���һ����,һ������һ��Ӧ��
		                   {
		                    // Read data 
		                    *pBuffer = I2C2_ReceiveByte();
		                    I2C2_SendACK();
		                    /* */
		                    pBuffer++;
		                    // Decrement the read bytes counter 
		                    NumByteToRead--;
		                   }
		                   
		                   // it remains to read three data: data N-2, data N-1, Data N 
			                 if (NumByteToRead == 3)
			                 {
			                    // Read Data N-2 
			                    *pBuffer = I2C2_ReceiveByte();
			                    I2C2_SendACK();
			                    // Increment 
			                    pBuffer++;
			                    // Program the STOP 
			                    //-I2Cx->CR1 |= CR1_STOP_Set;		//-������ֹͣλʲô��˼
			                    /* Read DataN-1 */
			                    *pBuffer = I2C2_ReceiveByte();
			                    I2C2_SendACK();
			                    // Increment 
			                    pBuffer++;
			                    // Wait until RXNE is set (DR contains the last data)
			                    //-while ((I2Cx->SR1 & 0x00040) != 0x000040);		//-ģ���ʱ���ֱ�Ӷ���,û�������ı�־
			                    // Read DataN 
			                    *pBuffer = I2C2_ReceiveByte();
			                    I2C2_SendNACK();
			                    // Reset the number of bytes to be read by master 
			                    NumByteToRead = 0;	
			                 }
		              }
		              
		           	  I2C2_STOP();
		           	  return Success;		//-���ճɹ�
		           }
		        } 
		        else
		        	 i2c2_bus_error_cn++;                   
		    }
    }
        	       
    return Error;			//-ֵΪ0
}
//-�ӻ���ַ��SlaveAddress,����pBuffer�в������ӻ���ַ
int I2C_Master_BufferWrite(I2C_TypeDef* I2Cx, UINT8* pBuffer,  UINT32 NumByteToWrite, UINT8 Mode, UINT8 SlaveAddress )
{
	 int res;
   
   if(Mode == 1)
   {
		   res = I2C1_START();
		   if(res == TWI_READY)
		   {//-ֻ����ʼ�����ɹ��˲ż�������
		   	  i2c1_bus_error_cn = 0;
		   	  //- Send slave address 
		      //- Reset the address bit0 for write
		      SlaveAddress &= OAR1_ADD0_Reset;
		      Address = SlaveAddress;
		      //- Send the slave address 
		      //-Wait until ADDR is set: EV6 
		      res = I2C1_SendByte(Address);	//-�᷵��Ӧ��ֵACK��NACK
		      if(res == TWI_ACK)
		      {
		      	 //- Write the first data in DR register (EV8_1) 
		         res = I2C1_SendByte(*pBuffer);		//-Ӳ��I2C������ȴ����ݷ��ͽ������ٷ�����һ������,�������ܽ�����ѭ��(��һ�˼Ҳ��ش�Ļ����绵��),����ģ��I2C,
		         //- Increment              //-ʹ�ô���,�Է����ܳɹ�ACK�Ļ�Ҳ��Ϊû�н��ճɹ�
		         pBuffer++;
		         //- Decrement the number of bytes to be written 
		         NumByteToWrite--;
		         //- While there is data to be written,���ﲻ�ٿ����Ƿ�Ӧ��ɹ���������,ֱ��ʱ����,Ĭ���Ǻõ� 
		         while (NumByteToWrite--)	//-���е����ݷ��ͳ�ȥ
		         {
		             // Poll on BTF to receive data because in polling mode we can not guarantee the
		             // EV8 software sequence is managed before the current byte transfer completes 
		             //-while ((I2Cx->SR1 & 0x00004) != 0x000004);
		             //-����ģ��Ŀ϶����ͳ�ȥ��,���Բ���Ҫ��Ӳ���������ȴ���־λ
		             if(res == TWI_ACK)
		             {
				             // Send the current byte 
				             res = I2C1_SendByte(*pBuffer);
				             if(res == TWI_ACK)
				             {
				                 // Point to the next byte to be written 
				                 pBuffer++;
				             }
				             else
				             	   return Error; 
		             }
		             else
		             	   return Error;
		         }
		         // EV8_2: Wait until BTF is set before programming the STOP 
		         //-while ((I2Cx->SR1 & 0x00004) != 0x000004);
		         if(res == TWI_ACK)
		         {
				         // Send STOP condition 
				         I2C1_STOP();
				         // Make sure that the STOP bit is cleared by Hardware ,�Ҿ������ﲻ��Ҫ�ȴ�,�����ģ��Ļ�
				         //-while ((I2Cx->CR1&0x200) == 0x200);
				         return Success;
		         }
		      }	
		            
		   }
		   else
		   	  i2c1_bus_error_cn++;		//-���Ŀǰ���ܼӼ���,ǰ������жϹ����Ѿ�ȥ����
   }
   else if(Mode == 2)
   {
   	   res = I2C2_START();
		   if(res == TWI_READY)
		   {//-ֻ����ʼ�����ɹ��˲ż�������
		   	  i2c2_bus_error_cn = 0;
		   	  //- Send slave address 
		      //- Reset the address bit0 for write
		      SlaveAddress &= OAR1_ADD0_Reset;
		      Address = SlaveAddress;
		      //- Send the slave address 
		      //-Wait until ADDR is set: EV6 
		      res = I2C2_SendByte(Address);	//-�᷵��Ӧ��ֵACK��NACK
		      if(res == TWI_ACK)
		      {
		      	 //- Write the first data in DR register (EV8_1) 
		         res = I2C2_SendByte(*pBuffer);		//-Ӳ��I2C������ȴ����ݷ��ͽ������ٷ�����һ������,�������ܽ�����ѭ��(��һ�˼Ҳ��ش�Ļ����绵��),����ģ��I2C,
		         //- Increment              //-ʹ�ô���,�Է����ܳɹ�ACK�Ļ�Ҳ��Ϊû�н��ճɹ�
		         pBuffer++;
		         //- Decrement the number of bytes to be written 
		         NumByteToWrite--;
		         //- While there is data to be written,���ﲻ�ٿ����Ƿ�Ӧ��ɹ���������,ֱ��ʱ����,Ĭ���Ǻõ� 
		         while (NumByteToWrite--)	//-���е����ݷ��ͳ�ȥ
		         {
		             // Poll on BTF to receive data because in polling mode we can not guarantee the
		             // EV8 software sequence is managed before the current byte transfer completes 
		             //-while ((I2Cx->SR1 & 0x00004) != 0x000004);
		             //-����ģ��Ŀ϶����ͳ�ȥ��,���Բ���Ҫ��Ӳ���������ȴ���־λ
		             if(res == TWI_ACK)
		             {
				             // Send the current byte 
				             res = I2C2_SendByte(*pBuffer);
				             if(res == TWI_ACK)
				             {
				                 // Point to the next byte to be written 
				                 pBuffer++;
				             }
				             else
				             	   return Error; 
		             }
		             else
		             	   return Error;
		         }
		         // EV8_2: Wait until BTF is set before programming the STOP 
		         //-while ((I2Cx->SR1 & 0x00004) != 0x000004);
		         if(res == TWI_ACK)
		         {
				         // Send STOP condition 
				         I2C2_STOP();
				         // Make sure that the STOP bit is cleared by Hardware ,�Ҿ������ﲻ��Ҫ�ȴ�,�����ģ��Ļ�
				         //-while ((I2Cx->CR1&0x200) == 0x200);
				         return Success;
		         }
		      }	
		            
		   }
		   else
		   	  i2c2_bus_error_cn++;
   }
   	         
   return Error;
}
#endif

/**
* @brief  Initializes peripherals: I2Cx, GPIO, DMA channels .
  * @param  None
  * @retval None
  */
void I2C_LowLevel_Init(I2C_TypeDef* I2Cx)
{
    //-GPIO_InitTypeDef  GPIO_InitStructure;
    I2C_InitTypeDef  I2C_InitStructure;

    //-/* GPIOB clock enable */
    //-RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    //-/* Enable the DMA1 clock */
    //-RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    //-if (I2Cx == I2C1)
    //-{
        //-/* I2C1 clock enable */
        //-RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
        //-/* I2C1 SDA and SCL configuration */
        //-GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
        //-GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        //-GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
        //-GPIO_Init(GPIOB, &GPIO_InitStructure);

        //-GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
        //-GPIO_Init(GPIOB, &GPIO_InitStructure);

        //-/* Enable I2C1 reset state */
        //-RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, ENABLE);
        //-/* Release I2C1 from reset state */
        //-RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, DISABLE);
    //-}
    //-else /* I2Cx = I2C2 */
    //-{

        //-/* I2C2 clock enable */
        //-RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
        //-/* I2C1 SDA and SCL configuration */
        //-GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
        //-GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        //-GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
        //-GPIO_Init(GPIOB, &GPIO_InitStructure);

        //-GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
        //-GPIO_Init(GPIOB, &GPIO_InitStructure);

        //-/* Enable I2C2 reset state */
        //-RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C2, ENABLE);
        //-/* Release I2C2 from reset state */
        //-RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C2, DISABLE);
    //-}

    /* I2C1 and I2C2 configuration */
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = OwnAddress1;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = ClockSpeed;
    I2C_Init(I2C1, &I2C_InitStructure);
    
    I2C_InitStructure.I2C_OwnAddress1 = OwnAddress2;
    I2C_Init(I2C2, &I2C_InitStructure);

    if (I2Cx == I2C1)
    {   //-/* I2C1 TX DMA Channel configuration */
        //-DMA_DeInit(I2C1_DMA_CHANNEL_TX);
        //-I2CDMA_InitStructure.DMA_PeripheralBaseAddr = (UINT32)I2C1_DR_Address;
        //-I2CDMA_InitStructure.DMA_MemoryBaseAddr = (UINT32)0;   /* This parameter will be configured durig communication */
        //-I2CDMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;    /* This parameter will be configured durig communication */
        //-I2CDMA_InitStructure.DMA_BufferSize = 0xFFFF;            /* This parameter will be configured durig communication */
        //-I2CDMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
        //-I2CDMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
        //-I2CDMA_InitStructure.DMA_PeripheralDataSize = DMA_MemoryDataSize_Byte;
        //-I2CDMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
        //-I2CDMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
        //-I2CDMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
        //-I2CDMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
        //-DMA_Init(I2C1_DMA_CHANNEL_TX, &I2CDMA_InitStructure);

        //-/* I2C1 RX DMA Channel configuration */
        //-DMA_DeInit(I2C1_DMA_CHANNEL_RX);
        //-DMA_Init(I2C1_DMA_CHANNEL_RX, &I2CDMA_InitStructure);
    }
    else /* I2Cx = I2C2 */
    {
        //-/* I2C2 TX DMA Channel configuration */
        //-DMA_DeInit(I2C2_DMA_CHANNEL_TX);
        //-I2CDMA_InitStructure.DMA_PeripheralBaseAddr = (UINT32)I2C2_DR_Address;
        //-I2CDMA_InitStructure.DMA_MemoryBaseAddr = (UINT32)0;   /* This parameter will be configured durig communication */
        //-I2CDMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;    /* This parameter will be configured durig communication */
        //-I2CDMA_InitStructure.DMA_BufferSize = 0xFFFF;            /* This parameter will be configured durig communication */
        //-I2CDMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
        //-I2CDMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
        //-I2CDMA_InitStructure.DMA_PeripheralDataSize = DMA_MemoryDataSize_Byte;
        //-I2CDMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
        //-I2CDMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
        //-I2CDMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
        //-I2CDMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
        //-DMA_Init(I2C2_DMA_CHANNEL_TX, &I2CDMA_InitStructure);

        //-/* I2C2 RX DMA Channel configuration */
        //-DMA_DeInit(I2C2_DMA_CHANNEL_RX);
        //-DMA_Init(I2C2_DMA_CHANNEL_RX, &I2CDMA_InitStructure);

    }
}

/**
  * @brief  Initializes DMA channel used by the I2C Write/read routines.
  * @param  None.
  * @retval None.
  */
void I2C_DMAConfig(I2C_TypeDef* I2Cx, UINT8* pBuffer, UINT32 BufferSize, UINT32 Direction)
{
    /* Initialize the DMA with the new parameters */
    if (Direction == I2C_DIRECTION_TX)
    {
        /* Configure the DMA Tx Channel with the buffer address and the buffer size */
        I2CDMA_InitStructure.DMA_MemoryBaseAddr = (UINT32)pBuffer;
        I2CDMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
        I2CDMA_InitStructure.DMA_BufferSize = (UINT32)BufferSize;

        if (I2Cx == I2C1)
        {
            //-I2CDMA_InitStructure.DMA_PeripheralBaseAddr = (UINT32)I2C1_DR_Address;
            //-DMA_Cmd(I2C1_DMA_CHANNEL_TX, DISABLE);
            //-DMA_Init(I2C1_DMA_CHANNEL_TX, &I2CDMA_InitStructure);
            //-DMA_Cmd(I2C1_DMA_CHANNEL_TX, ENABLE);
        }
        else
        {
            //-I2CDMA_InitStructure.DMA_PeripheralBaseAddr = (UINT32)I2C2_DR_Address;
            //-DMA_Cmd(I2C2_DMA_CHANNEL_TX, DISABLE);
            //-DMA_Init(I2C2_DMA_CHANNEL_TX, &I2CDMA_InitStructure);
            //-DMA_Cmd(I2C2_DMA_CHANNEL_TX, ENABLE);
        }
    }
    else /* Reception */
    {
        /* Configure the DMA Rx Channel with the buffer address and the buffer size */
        I2CDMA_InitStructure.DMA_MemoryBaseAddr = (UINT32)pBuffer;
        I2CDMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
        I2CDMA_InitStructure.DMA_BufferSize = (UINT32)BufferSize;
        if (I2Cx == I2C1)
        {

            //-I2CDMA_InitStructure.DMA_PeripheralBaseAddr = (UINT32)I2C1_DR_Address;
            //-DMA_Cmd(I2C1_DMA_CHANNEL_RX, DISABLE);
            //-DMA_Init(I2C1_DMA_CHANNEL_RX, &I2CDMA_InitStructure);
            //-DMA_Cmd(I2C1_DMA_CHANNEL_RX, ENABLE);
        }

        else
        {
            //-I2CDMA_InitStructure.DMA_PeripheralBaseAddr = (UINT32)I2C2_DR_Address;
            //-DMA_Cmd(I2C2_DMA_CHANNEL_RX, DISABLE);
            //-DMA_Init(I2C2_DMA_CHANNEL_RX, &I2CDMA_InitStructure);
            //-DMA_Cmd(I2C2_DMA_CHANNEL_RX, ENABLE);
        }

    }
}

//-����ƽ̨�ӿڶ�д����,�ϲ��Ӳ��ƽ̨�Ĺ�ϵ������ת��
//-msg  ��Ϣ������;num  Ӳ���˿ں�
int i2c_transfer(struct i2c_msg *msg, int num)
{
	 int ret;
	 UINT8* pBuffer;  
	 UINT32 NumByteToWrite; 
	 //-UINT8 Mode; 
	 UINT8 SlaveAddress;

		SlaveAddress = msg->addr;
		pBuffer = msg->buf;
		NumByteToWrite = msg->len;
		
		if(num == 1)	//-�ж����Ǹ�Ӳ���˿�
		//-��Ҫ�Ĳ����ӻ���ַ,���ģʽ��һ����Ҫ,��д����,��������,I2C�˿ں�
    	 ret = I2C_Master_BufferWrite( I2C1, pBuffer, NumByteToWrite, num, SlaveAddress );
    else if(num == 2)
    	 ret = I2C_Master_BufferWrite( I2C2, pBuffer, NumByteToWrite, num, SlaveAddress );
    else if(num == 3)
    	 ret = I2C_Master_BufferWrite( I2C2, pBuffer, NumByteToWrite, num, SlaveAddress );	//-����Ӳ������֧������I2C�ӿ�,���Ե�һ���������ڳ���2��û������
    else if(num == 4)
    	 ret = I2C_Master_BufferWrite( I2C2, pBuffer, NumByteToWrite, num, SlaveAddress );
    else if(num == 5)
    	 ret = I2C_Master_BufferWrite( I2C2, pBuffer, NumByteToWrite, num, SlaveAddress );	 	 	 
    
    return ret;
}

//-�ײ�I2C���յĴ�����
int i2c_receive(struct i2c_msg *msg, int num)
{
	 int ret;
	 UINT8* pBuffer;  
	 UINT32 NumByteToWrite; 
	 //-UINT8 Mode; 
	 UINT8 SlaveAddress;

		SlaveAddress = msg->addr;
		pBuffer = msg->buf;
		NumByteToWrite = msg->len;
		
		if(num == 1)	//-�ж����Ǹ�Ӳ���˿�
		//-��Ҫ�Ĳ����ӻ���ַ,���ģʽ��һ����Ҫ,��д����,��������,I2C�˿ں�
    	 ret = I2C_Master_BufferRead( I2C1, pBuffer, NumByteToWrite, num, SlaveAddress );
    else if(num == 2)
    	 ret = I2C_Master_BufferRead( I2C2, pBuffer, NumByteToWrite, num, SlaveAddress );
    else if(num == 3)
    	 ret = I2C_Master_BufferRead( I2C2, pBuffer, NumByteToWrite, num, SlaveAddress );
    else if(num == 4)
    	 ret = I2C_Master_BufferRead( I2C2, pBuffer, NumByteToWrite, num, SlaveAddress );
    else if(num == 5)
    	 ret = I2C_Master_BufferRead( I2C2, pBuffer, NumByteToWrite, num, SlaveAddress );	 	 	 
    
    return ret;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*
//-�����I2C��ģ��I2C  
void I2C_delay(void)
{	
   u8 i = I2C_Delay_Wait; //��������Ż��ٶ�	����������͵�5����д��
   while(i) 
   { 
     i--; 
   } 
}  

void I2C_SendByte(u8 SendByte) //���ݴӸ�λ����λ//
{
    u8 i=8;
    while(i--)
    {
        SCL_L;	//-��GPIO��Ӧ����Ϊ�趨Ϊ��Ҫ�ĵ�ƽ
        I2C_delay();
      if(SendByte&0x80)	//-�ж�����λֵ,Ȼ������
        SDA_H;  
      else 
        SDA_L;   
        SendByte<<=1;
        I2C_delay();
		    SCL_H;
        I2C_delay();
    }
    SCL_L;
}

BOOL I2C_Start(void)	//?�����Ĳ����е���ģ��I2C
{
	SDA_H;
	I2C_delay();
	SCL_H;
	I2C_delay();
	if(!SDA_read)return FALSE;	//SDA��Ϊ�͵�ƽ������æ,�˳�
	SDA_L;
	I2C_delay();
	if(SDA_read) return FALSE;	//SDA��Ϊ�ߵ�ƽ�����߳���,�˳�
	SDA_L;
	I2C_delay();
	return TRUE;
}

BOOL I2C_WaitAck(void) 	 //����Ϊ:=1��ACK,=0��ACK
{
	SCL_L;
	I2C_delay();
	SDA_H;			
	I2C_delay();
	SCL_H;
	I2C_delay();
	if(SDA_read)
	{
      SCL_L;
      return FALSE;
	}
	SCL_L;
	return TRUE;
}

void I2C_Stop(void)
{
	SCL_L;
	I2C_delay();
	SDA_L;
	I2C_delay();
	SCL_H;
	I2C_delay();
	SDA_H;
	I2C_delay();
}

u8 I2C_ReceiveByte(void)  //���ݴӸ�λ����λ//
{ 
    u8 i=8;
    u8 ReceiveByte=0;

    SDA_H;				
    while(i--)
    {
      ReceiveByte<<=1;      
      SCL_L;
      I2C_delay();
	    SCL_H;
      I2C_delay();	
      if(SDA_read)
      {
        ReceiveByte|=0x01;
      }
    }
    SCL_L;
    return ReceiveByte;
}

void I2C_NoAck(void)
{	
	SCL_L;
	I2C_delay();
	SDA_H;
	I2C_delay();
	SCL_H;
	I2C_delay();
	SCL_L;
	I2C_delay();
}

void I2C_Ack(void)
{	
	SCL_L;
	I2C_delay();
	SDA_L;
	I2C_delay();
	SCL_H;
	I2C_delay();
	SCL_L;
	I2C_delay();
}
*/
/*******************************************************************************
* Function Name  : I2C_EE_PageWrite
* Description    : Writes more than one byte to the EEPROM with a single WRITE
*                  cycle. The number of byte can't exceed the EEPROM page size.
* Input          : - pBuffer : pointer to the buffer containing the data to be 
*                    written to the EEPROM.
*                  - WriteAddr : EEPROM's internal address to write to.
*                  - NumByteToWrite : number of bytes to write to the EEPROM.
* Output         : None
* Return         : None
*******************************************************************************/
/*
void I2C_EE_PageWrite(BYTE* pBuffer, WORD WriteAddr, WORD NumByteToWrite)
{
	WORD i;
	
	GPIO_ResetBits(I2C_PORT, I2C_WP_PIN);

//-�����д����ʹ�õ��ǿ⺯��ʵ�ֵ�	
// 	//- Send START condition 
// 	I2C_GenerateSTART(I2C1, ENABLE);
// 	
// 	//- Test on EV5 and clear it 
// 	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT)); 
// 	
// 	//- Send EEPROM address for write 
// 	I2C_Send7bitAddress(I2C1, EEPROM_ADDRESS_WRITE, I2C_Direction_Transmitter);
// 	
// 	//- Test on EV6 and clear it 
// 	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));  
// 	
// 	//- Send the EEPROM's internal address to write to 
// 	//I2C_SendData(I2C1, WriteAddr);
// 	I2C_SendData(I2C1, (BYTE)(WriteAddr & 0xFF));
// 	I2C_SendData(I2C1, (BYTE)((WriteAddr >> 8) & 0xFF));
// 	
// 	//- Test on EV8 and clear it 
// 	while(! I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
// 	
// 	//- While there is data to be written 
// 	while(NumByteToWrite--)  
// 	{
// 		//- Send the current byte 
// 		I2C_SendData(I2C1, *pBuffer); 
// 		
// 		//- Point to the next byte to be written 
// 		pBuffer++; 
// 		
// 		//- Test on EV8 and clear it 
// 		while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
// 	}
// 	
// 	//- Send STOP condition 
// 	I2C_GenerateSTOP(I2C1, ENABLE);
	
	if (!I2C_Start())	//-�ж���������������״̬
	{
		GPIO_SetBits(I2C_PORT, I2C_WP_PIN);
		return;
	}
    I2C_SendByte(EEPROM_ADDRESS_WRITE);
    if (!I2C_WaitAck())	//-�ж��Ƿ�����Ҫ��״̬
	{
		I2C_Stop();		//-����ֹͣ״̬
		GPIO_SetBits(I2C_PORT, I2C_WP_PIN);
		return;
	}
	I2C_SendByte((u8)((WriteAddr >> 8) & 0x1F));
	I2C_WaitAck();
    I2C_SendByte((u8)(WriteAddr & 0xFF));
	I2C_WaitAck();
	
	while(NumByteToWrite--)
	{
		I2C_SendByte(* pBuffer);
		I2C_WaitAck();
		pBuffer++;
	}
	I2C_Stop();
	i = I2C_Delay_Stop;	//-��ʱ10���������ϣ���֤����д�����-
	while (i--)
	{
		I2C_delay();
	}
	GPIO_SetBits(I2C_PORT, I2C_WP_PIN);
}
*/
///////////////////////////////////ģ��I2C END//////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*
  ������Ҫ������������豸ͨ��ʱ����������һ������������������һ�����ʵ� I2C ��ַ��
  �������������������еĴ��豸��ʾ�����µ����ݽ��׵�ʱ�䡣����ָ�� I2C ��ַ�Ĵ���
  ����Ӧ���ء��������д��豸���������ϵĺ�������������ֱ����⵽��һ������������
  �����������ݽ��׷��� NACK ʱ�������᳢��ִ�����ݽ��ף�ֱ�����յ� ACK��
  ����һֱ���ʹ���ַ�Ͷ� / дλ��Ϣ���ظ�������ֱ����������ACK Ϊֹ��

*/


//-��������I2CӲ���ĳ�ʼ��
void I2C_Init_sub(void)
{
	  
	  I2C_DeInit(I2C1);
	  I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = OwnAddress1;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = 100000;//100K�ٶ�
		//-Enables or disables the specified I2C peripheral.
    I2C_Cmd(I2C1, ENABLE);
    I2C_Init(I2C1, &I2C_InitStructure);
    /*����1�ֽ�1Ӧ��ģʽ*/
	  I2C_AcknowledgeConfig(I2C1, ENABLE);
	  //-I2C_LowLevel_Init(I2C1);
	  
	  I2C_DeInit(I2C2);
	  I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = OwnAddress1;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = 100000;//100K�ٶ�
		//-Enables or disables the specified I2C peripheral.
    I2C_Cmd(I2C2, ENABLE);
    I2C_Init(I2C2, &I2C_InitStructure);
    /*����1�ֽ�1Ӧ��ģʽ*/
	  I2C_AcknowledgeConfig(I2C2, ENABLE);
	  //-I2C_LowLevel_Init(I2C2);


}


//-�ԼĴ���Ϊ��λ��д
//-����addrΪ�ӻ���ַ��flags�������ͨ�ŵı�־����������Ϊ0������������Ϊ I2C_M_RD��
//-lenΪ�˴�ͨ�ŵ������ֽ�����buf Ϊ���ͻ�������ݵ�ָ�롣���豸����������ͨ������ 
//-i2c-core ����Ľӿ� i2c_master_send �� i2c_master_recv �����ͻ����һ�����ݡ�

//- i2c_master_send �ӿڵ�����������client Ϊ�˴�������ͨ�ŵĴӻ���buf Ϊ���͵�����ָ�룬count Ϊ�������ݵ��ֽ�����
int i2c_master_send(struct i2c_client *client,u8 *buf ,int count)		//-CY8CMBR3xxx_WriteData();������ͬ�ȵ�λ
{
     int ret; 
     //-struct i2c_adapter *adap=client->adapter;  // ��ȡadapter��Ϣ ,,���Ŀǰ��ֲ����Ҫ
     struct i2c_msg msg;                        // ����һ����ʱ�����ݰ� 

     msg.addr = client->addr;                   // ���ӻ���ַд�����ݰ� 
     msg.flags = client->flags & I2C_M_TEN;     // ���ӻ���־�������ݰ� 
     msg.len = count;                           // ���˴η��͵������ֽ���д�����ݰ� 
     msg.buf = (u8 *)buf;                     // ����������ָ��д�����ݰ� 

     //-ret = i2c_transfer(adap, &msg, 1);         // ����ƽ̨�ӿڷ������� 
     ret = i2c_transfer(&msg, client->num);         // ����ƽ̨�ӿڷ������� ,,����Ĳ��������Ǹ���,Ҳ����Ԥ��λ�˿ں�

     /* If everything went ok (i.e. 1 msg transmitted), return #bytes transmitted, else error code. */ 
     return (ret == 1) ? count : ret;           // ������ͳɹ��ͷ����ֽ��� 
}

//- i2c_master_recv �ӿڵ�����������client Ϊ�˴�������ͨ�ŵĴӻ���buf Ϊ���յ�����ָ�룬
//-count Ϊ�������ݵ��ֽ��������ǿ�һ�� i2c_transfer �ӿڵĲ���˵����
int i2c_master_recv(struct i2c_client *client, u8 *buf ,int count) 
{
     //-struct i2c_adapter *adap=client->adapter;  // ��ȡadapter��Ϣ 
     struct i2c_msg msg;                        // ����һ����ʱ�����ݰ� 
     int ret; 
  
     msg.addr = client->addr;                   // ���ӻ���ַд�����ݰ� 
     msg.flags = client->flags & I2C_M_TEN;     // ���ӻ���־�������ݰ� 
     msg.flags |= I2C_M_RD;                     // ���˴�ͨ�ŵı�־�������ݰ� 
     msg.len = count;                           // ���˴ν��յ������ֽ���д�����ݰ� 
     msg.buf = buf; 
  
     ret = i2c_receive(&msg, client->num);         // ����ƽ̨�ӿڽ������� 
  
     /* If everything went ok (i.e. 1 msg transmitted), return #bytes
        transmitted, else error code. */ 
     return (ret == 1) ? count : ret;           // ������ճɹ��ͷ����ֽ��� 
} 
 

// I2C read x byte data from register ,һ���ֽ��ǲ��й�ϵ,Ȼ��ͨ��I2C����,��Ҫʹ�ô��н���
int i2c_read_data_Buffer(struct i2c_client *client,u8 numberOfBytes,u8 *data)
{
	//-u8 databuf[2]; 
	int res = 0;
	//-databuf[0]= *data;		//-��ʼ�ļĴ�����ַ
	

	res = i2c_master_recv(client,data,numberOfBytes);
	if(res <= 0)
	{
	//-	//-APS_ERR("i2c_master_recv function err\n");
		return res;
	}

	
	return 1;
}

// I2C read one byte data from register ,һ���ֽ��ǲ��й�ϵ,Ȼ��ͨ��I2C����,��Ҫʹ�ô��н���
int i2c_read_reg(struct i2c_client *client,u8 reg,u8 *data)
{
	u8 databuf[2]; 
	int res = 0;
	databuf[0]= reg;
	
	//-Ŀǰ������ʾ���ܻ���I2C������д����,��Ҫ��֤�϶���ȷ,������ּĴ������ò���,��ô�������޷�����
	res = i2c_master_send(client,databuf,0x1);	
	if(res <= 0)
	{
	//-	//-APS_ERR("i2c_master_send function err\n");
		return res;
	}
	//-while(res <= 0)
	//-{
  //-    delay_temp(10000);
	//-		res = i2c_master_send(client,databuf,0x1); //-ʵ��ʹ�ó���ʱ������ô����,�������ѭ�������ָܻ�
  //-    delay_temp(10000);              
	//-}
	
	
	res = i2c_master_recv(client,data,0x1);
	if(res <= 0)
	{
	//-	//-APS_ERR("i2c_master_recv function err\n");
		return res;
	}
	//-while(res <= 0)
	//-{
  //-    delay_temp(10000);
	//-		res = i2c_master_recv(client,databuf,0x1);		//-ʵ��ʹ�ó���ʱ������ô����,�������ѭ�������ָܻ�
	//-    delay_temp(10000);
  //-}
	
	
	return 1;
}

// I2C read x byte data from register ,һ���ֽ��ǲ��й�ϵ,Ȼ��ͨ��I2C����,��Ҫʹ�ô��н���
int i2c_read_reg_Buffer(struct i2c_client *client,u8 numberOfBytes,u8 *data)
{
	u8 databuf[2]; 
	int res = 0;
	databuf[0]= *data;		//-��ʼ�ļĴ�����ַ
	
	//-Ŀǰ������ʾ���ܻ���I2C������д����,��Ҫ��֤�϶���ȷ,������ּĴ������ò���,��ô�������޷�����
	res = i2c_master_send(client,databuf,0x1);	
	if(res <= 0)
	{
	//-	//-APS_ERR("i2c_master_send function err\n");
		return res;
	}
	
	res = i2c_master_recv(client,data,numberOfBytes);
	if(res <= 0)
	{
	//-	//-APS_ERR("i2c_master_recv function err\n");
		return res;
	}

	
	return 1;
}


int i2c_write_data(struct i2c_client *client,u8 reg,u8 value)		//-ֱ����I2C���豸��������,����λ���Ĵ�����,���ڲ����ļĴ������Բ���reg ����ν,Ĭ�ϸ�0 ����
{
	u8 databuf[2];	//-��ַ�����ݶ�����"�ֽ�"����ʽ���ͳ�ȥ��
	int res = 0;

	//-databuf[0] = reg;   
	databuf[0] = value;
	
	res = i2c_master_send(client,databuf,0x01);
	if (res <=0)
	{
		return res;
	//-	//-APS_ERR("i2c_master_send function err\n");
	}
	//-while(res <= 0)
	//-{
  //-    delay_temp(10000);
	//-	  res = i2c_master_send(client,databuf,0x2);
	//-    delay_temp(10000);
  //-}	
		
		return 1;
}

//-reg �Ĵ�����ַ;value ��д������
// I2C Write one byte data to register
//?��α�֤д�����ݾ�����ȷ,��������ⲿ���Ż������߹��϶���ȷִ�г���
//?ʹӰ����С
int i2c_write_reg(struct i2c_client *client,u8 reg,u8 value)
{
	u8 databuf[2];	//-��ַ�����ݶ�����"�ֽ�"����ʽ���ͳ�ȥ��
	int res = 0;

	databuf[0] = reg;   
	databuf[1] = value;
	
	res = i2c_master_send(client,databuf,0x02);
	if (res <=0)
	{
		return res;
	//-	//-APS_ERR("i2c_master_send function err\n");
	}
	//-while(res <= 0)
	//-{
  //-    delay_temp(10000);
	//-	  res = i2c_master_send(client,databuf,0x2);
	//-    delay_temp(10000);
  //-}	
		
		return 1;
}

//-������д����һ�������Ĵ���������
//-��������һ���ֽ��ǹ̶���ƫ�Ƶ�ַȻ�������е�����,���͵��ֽ���=��ַ+���ݸ���(��ַ���ǼĴ���ƫ���������1)
int i2c_write_reg_Buffer(struct i2c_client *client,u8 *writeBuffer,u8 numberOfBytes)
{
	//-u8 databuf[2];	//-��ַ�����ݶ�����"�ֽ�"����ʽ���ͳ�ȥ��
	int res = 0;

	//-databuf[0] = reg;   
	//-databuf[1] = value;
	
	res = i2c_master_send(client,writeBuffer,numberOfBytes);
	if (res <=0)
	{
		return res;
	}
		
		return 1;
}