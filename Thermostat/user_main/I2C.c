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

//-下面是硬件接口I2C
/*
  设定参数
  
*/

#define I2C_DIRECTION_TX	      1		//-为了编译的通过临时增加的定义,实际使用时需要修改
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
//-如果使用模拟I2C就采用下面的程序,而且是面向两个端口的
//-模拟的应该不存在死锁的问题,顶多是总线被抢占,这次操作不能成功,延时后再读写就是了
//-还有什么的话遇到再说
//-对于模拟时中断的处理,我觉得目前不需要关闭中断,模拟信号是同步时钟,只要不影响时序就行
//-下面的整体逻辑可以了(单个模块的),如果需要调整的话,也就是延时长短
#ifdef I2C_SOFTWARE_FLAG
//-由于这里可能会因为把IO定义为开漏输出,电压而被钳位,所以这里重新初始化而恢复电平

//-I2C是同步时钟,理论上是可以无限慢的,快了可能由于器件问题反应不过来
//-常态作为主机,一直处于发送状态,需要时切换为输入,用完切换为常态
u8 I2C1_START(void)			//-总线忙(数据线无法拉高);发送起始信号失败(数据线无法拉低)
{
	  //-首先需要保证总线是空闲
		I2C1_SDA_H;		//-全部拉高没有时序要求所以无需延时		
		I2C1_SCL_H;
		TWI_NOP;		
		//-if(!I2C1_SDA_read)		//-现在我来使用推挽模式,那么这个就先不读了否则反复切换
		//-{
		//-	////DebugPrint("TWI_START:BUSY\n");
		//-	return TWI_BUS_BUSY;
		//-}
		//-开始发送起始信号
		I2C1_SDA_L;
		TWI_NOP;
		I2C1_SCL_L;
		TWI_NOP;
		//-保证起始信号发送成功		
		//-if(I2C1_SDA_read)
		//-{
		//-	////DebugPrint("TWI_START:BUS ERROR\n");
		//-	return TWI_BUS_ERROR;
		//-}
		return TWI_READY;
}

void I2C1_STOP(void)
{
	  //-置高置低仅仅是为了保证条件,并不代表一定改变了状态,也许以前就是这个状态
		I2C1_SDA_L;
    I2C1_SCL_L;		//-可以保险的设置下如果原来是低的这里是没有影响的
		TWI_NOP;
		I2C1_SCL_H;
		TWI_NOP;
		//-开始发送结束信号
		I2C1_SDA_H;
		TWI_NOP;		
		//////DebugPrint("TWI_STOP\n");
}

void I2C1_SendACK(void)
{
	  I2C1_SCL_L;		//-这里没有也行,仅仅是为了可靠
		I2C1_SDA_L;		//-低电平时改变数据
		TWI_NOP;
		I2C1_SCL_H;		//-高电平时供采样
		TWI_NOP;
		I2C1_SCL_L;		//-结束时钟信号
		TWI_NOP;
		//////DebugPrint("TWI_SendACK\n");
}

void I2C1_SendNACK(void)
{
	  I2C1_SCL_L;		//-这里没有也行,仅仅是为了可靠
		I2C1_SDA_H;
		TWI_NOP;
		I2C1_SCL_H;
		TWI_NOP;
		I2C1_SCL_L;
		TWI_NOP;
		//////DebugPrint("TWI_SendNACK\n");
}

u8 I2C1_SendByte(u8 Data)	//-返回的值决定了从机应答的内容(ACK OR NACK)
{
		u8 i,ucErrTime=0;
		
		I2C1_SCL_L;		//-发送数据的第一步就是拉低时钟,这个时候才可以改变数据,高电平是不允许改变数据的
		for(i=0;i<8;i++)
		{
				//---------数据建立----------
				if(Data&0x80)
				{
					 I2C1_SDA_H;
				}
				else
				{
					 I2C1_SDA_L;
				}
				Data <<= 1;
				TWI_NOP;	//-延时到数据线上数据稳定
				//---数据建立保持一定延时----
				//----产生一个上升沿[正脉冲]
				I2C1_SCL_H;
				TWI_NOP;		//-高电平时间保证充足采样
				I2C1_SCL_L;
				TWI_NOP;//延时,防止SCL还没变成低时改变SDA,从而产生START/STOP信号
				//---------------------------
		}
		
		//接收从机的应答
		I2C1_SDA_IN();      //SDA设置为输入  
		I2C1_SDA_H;		//-这里的输出高电平其实是设定输入上拉或下拉
		//-I2C1_SDA_L;
		I2C1_SCL_H;		//-高电平期间读数据线上的数据
		TWI_NOP;
		while(I2C1_SDA_read)
	  {
			ucErrTime++;
			if(ucErrTime>20)		//-100长度待定
			{
				I2C1_SCL_L;		//-不应答就是出错,出错就该恢复空闲模式
				TWI_NOP;
				I2C1_SDA_OUT();//sda线输出
				return TWI_NACK;
			}
	  }
		
		//-低电平是应答信号
		I2C1_SCL_L;
		TWI_NOP;
		I2C1_SDA_OUT();//sda线输出
		//////DebugPrint("TWI_ACK!\n");
		return TWI_ACK;    
}

u8 I2C1_ReceiveByte(void)	//-返回的是8位数据值
{
		u8 i,Dat;
		
		I2C1_SDA_IN();//SDA设置为输入
		I2C1_SDA_H;	//-这里的目的不是输出电平,而是配置上拉,下拉
		//-I2C1_SDA_L;
		I2C1_SCL_L;	//-拉低是为了让数据线可以改变状态
		Dat=0;
		for(i=0;i<8;i++)
		{
				I2C1_SCL_H;//产生时钟上升沿[正脉冲],让从机准备好数据
				TWI_NOP;
				Dat<<=1;
				if(I2C1_SDA_read) //读引脚状态,,高电平时的状态就是数据线上有效的数据
				{
				   Dat|=0x01;
				}
				I2C1_SCL_L;//准备好再次接收数据
				TWI_NOP;//等待数据准备好
		}
		//////DebugPrint("TWI_Dat:%x\n",Dat);
		I2C1_SDA_OUT();		//-恢复到常态
		return Dat;
}

//-模拟I2C2
u8 I2C2_START(void)
{
	  //-首先需要保证总线是空闲
		I2C2_SDA_H;		//-全部拉高没有时序要求所以无需延时		
		I2C2_SCL_H;
		TWI_NOP;		
		//-if(!I2C1_SDA_read)		//-现在我来使用推挽模式,那么这个就先不读了否则反复切换
		//-{
		//-	////DebugPrint("TWI_START:BUSY\n");
		//-	return TWI_BUS_BUSY;
		//-}
		//-开始发送起始信号
		I2C2_SDA_L;
		TWI_NOP;
		I2C2_SCL_L;
		TWI_NOP;
		//-保证起始信号发送成功		
		//-if(I2C1_SDA_read)
		//-{
		//-	////DebugPrint("TWI_START:BUS ERROR\n");
		//-	return TWI_BUS_ERROR;
		//-}
		return TWI_READY;
}

void I2C2_STOP(void)
{
	  //-置高置低仅仅是为了保证条件,并不代表一定改变了状态,也许以前就是这个状态
		I2C2_SDA_L;
    I2C2_SCL_L;		//-可以保险的设置下如果原来是低的这里是没有影响的
		TWI_NOP;
		I2C2_SCL_H;
		TWI_NOP;
		//-开始发送结束信号
		I2C2_SDA_H;
		TWI_NOP;	
}

void I2C2_SendACK(void)
{
		I2C2_SCL_L;		//-这里没有也行,仅仅是为了可靠
		I2C2_SDA_L;		//-低电平时改变数据
		TWI_NOP;
		I2C2_SCL_H;		//-高电平时供采样
		TWI_NOP;
		I2C2_SCL_L;		//-结束时钟信号
		TWI_NOP;
}

void I2C2_SendNACK(void)
{
		I2C2_SCL_L;		//-这里没有也行,仅仅是为了可靠
		I2C2_SDA_H;
		TWI_NOP;
		I2C2_SCL_H;
		TWI_NOP;
		I2C2_SCL_L;
		TWI_NOP;
}

u8 I2C2_SendByte(u8 Data)	//-返回的值决定了从机应答的内容(ACK OR NACK)
{
		u8 i=0;
    u8 ucErrTime=0;
		
		I2C2_SCL_L;		//-发送数据的第一步就是拉低时钟,这个时候才可以改变数据,高电平是不允许改变数据的
		for(i=0;i<8;i++)
		{
				//---------数据建立----------
				if(Data&0x80)
				{
					 I2C2_SDA_H;
				}
				else
				{
					 I2C2_SDA_L;
				}
				Data <<= 1;
				TWI_NOP;	//-延时到数据线上数据稳定
				//---数据建立保持一定延时----
				//----产生一个上升沿[正脉冲]
				I2C2_SCL_H;
				TWI_NOP;		//-高电平时间保证充足采样
				I2C2_SCL_L;
				TWI_NOP;//延时,防止SCL还没变成低时改变SDA,从而产生START/STOP信号
				//---------------------------
		}
		
		//接收从机的应答
		I2C2_SDA_IN();      //SDA设置为输入  
		I2C2_SDA_H;		//-这里的输出高电平其实是设定输入上拉或下拉
		I2C2_SCL_H;		//-高电平期间读数据线上的数据
		TWI_NOP;
		while(I2C2_SDA_read)
	  {
			ucErrTime++;
			if(ucErrTime>20)		//-长度待定,这里值为100时大约等待46uS
			{
				I2C2_SCL_L;		//-不应答就是出错,出错就该恢复空闲模式,但是这里需要首先完成一个时钟周期,所以需要拉低
				TWI_NOP;
				I2C2_SDA_OUT();//sda线输出
				return TWI_NACK;
			}
	  }
		
		//-低电平是应答信号
		I2C2_SCL_L;
		TWI_NOP;
		I2C2_SDA_OUT();//sda线输出
		//////DebugPrint("TWI_ACK!\n");
		return TWI_ACK;  
}

u8 I2C2_ReceiveByte(void)	//-返回的是8位数据值
{
		u8 i,Dat;
		
		I2C2_SDA_IN();//SDA设置为输入
		I2C2_SDA_H;	//-这里的目的不是输出电平,而是配置上拉,下拉
		I2C2_SCL_L;	//-拉低是为了让数据线可以改变状态
		Dat=0;
		for(i=0;i<8;i++)
		{
				I2C2_SCL_H;//产生时钟上升沿[正脉冲],让从机准备好数据
				TWI_NOP;
				Dat<<=1;
				if(I2C2_SDA_read) //读引脚状态,,高电平时的状态就是数据线上有效的数据
				{
				   Dat|=0x01;
				}
				I2C2_SCL_L;//准备好再次接收数据
				TWI_NOP;//等待数据准备好
		}
		//////DebugPrint("TWI_Dat:%x\n",Dat);
		I2C2_SDA_OUT();		//-恢复到常态
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
//-实现了硬件的读数据,有三种方式供选择DMA POLLING IT,现在仅仅默认是POLLING方式  
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
            SlaveAddress |= OAR1_ADD0_Set;		//-读写就是最后一位值决定的,这里就确保了值
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
            //-__disable_irq();     //?思考下关闭中断的事情,有没有总中断关闭
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
            NVIC_SETPRIMASK();          //-等效关闭总中断
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
//-硬件写一系列数据,参数给对了,这里就实现了硬件的操作
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
        I2Cx->CR1 |= CR1_START_Set;	//-不需要直接控制引脚状态,通过硬件接口寄存器就可以
        /* Wait until SB flag is set: EV5 */
        while ((I2Cx->SR1&0x0001) != 0x0001)	//-成功发送起始条件后继续操作
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
        while (NumByteToWrite--)	//-把有的数据发送出去
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
#else		//-如果使用模拟I2C就使用下面的程序
//-接收数据的时候仅仅指定从机地址(主机发送),然后读取出现在总线上的数据就行
//-下面采用的是模拟I2C读写数据,具体时序和操作需要验证
//-由于模拟I2C存在一个同时存在的可能性,所以这里把Mode作为端口选择参数
//-出错总共就是两个问题一个是总线死锁了,一个就是下面的装置没有应答.
int I2C_Master_BufferRead(I2C_TypeDef* I2Cx, UINT8* pBuffer,  UINT32 NumByteToRead, UINT8 Mode, UINT8 SlaveAddress)
{
    int res;
    
    if(Mode == 1)
    {	
			  if (NumByteToRead == 1)
		    {
		        // Send START condition 
		        res = I2C1_START();
		        if(res == TWI_READY)	//-判断总线是否空闲
		        {
		        	 i2c1_bus_error_cn = 0;
		        	 // Send slave address 
		           // Reset the address bit0 for read 
		           SlaveAddress |= OAR1_ADD0_Set;		//-读写就是最后一位值决定的,这里就确保了值
		           Address = SlaveAddress;
		           // Send the slave address 
		           res = I2C1_SendByte(Address);
		           if(res == TWI_ACK)
		           {//-从机应答后,就可以开始读取总线上的数据了,这里读完数据后不需要发送ACK是发送NACK
		           	// Read the data
		           	  *pBuffer = I2C1_ReceiveByte();
		           	  I2C1_SendNACK();
		           	  I2C1_STOP();
		           	  return Success;		//-接收成功
		           }
		        }	
		        else
		        	 i2c1_bus_error_cn++;                                                    
		    }
		    else if (NumByteToRead == 2)
		    {//-目前没有一次读两个字节的内容
		    	  // Send START condition 
		        res = I2C1_START();
		        if(res == TWI_READY)	//-判断总线是否空闲
		        {
		        	 i2c1_bus_error_cn = 0;
		        	 // Send slave address 
		           // Reset the address bit0 for read 
		           SlaveAddress |= OAR1_ADD0_Set;		//-读写就是最后一位值决定的,这里就确保了值
		           Address = SlaveAddress;
		           // Send the slave address 
		           res = I2C1_SendByte(Address);
		           if(res == TWI_ACK)
		           {
		           	  // Read first data,,这里总线上是否出现了有效数据需要我人为去考虑,这是模拟的,不能等待主机检测
		           	  *pBuffer = I2C1_ReceiveByte();
		           	  I2C1_SendACK();
		           	  pBuffer++;
		           	  // Read second data 
		              *pBuffer = I2C1_ReceiveByte();
		              I2C1_SendNACK();
		           	  I2C1_STOP();
		           	  return Success;		//-接收成功
		           }
		        }	
		        else
		        	 i2c1_bus_error_cn++;    	                           
		    }
		    else
		    {    
		        // Send START condition 
		        res = I2C1_START();
		        if(res == TWI_READY)	//-判断总线是否空闲
		        {
		        	 i2c1_bus_error_cn = 0;
		        	 // Send slave address 
		           // Reset the address bit0 for read 
		           SlaveAddress |= OAR1_ADD0_Set;		//-读写就是最后一位值决定的,这里就确保了值
		           Address = SlaveAddress;
		           // Send the slave address 
		           res = I2C1_SendByte(Address);
		           if(res == TWI_ACK)
		           {
		           	  // While there is data to be read 
		              while (NumByteToRead)
		              {
		        	         // Receive bytes from first byte until byte N-3
		        	         if (NumByteToRead != 3)	//-硬件读I2C最后三个字节需要特别处理,因为这些寄存器配置需要修改才能配置出最后结束的效果,前面的都是一样的,一个数据一个应答
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
			                    //-I2Cx->CR1 |= CR1_STOP_Set;		//-这里搞个停止位什么意思
			                    /* Read DataN-1 */
			                    *pBuffer = I2C1_ReceiveByte();
			                    I2C1_SendACK();
			                    // Increment 
			                    pBuffer++;
			                    // Wait until RXNE is set (DR contains the last data)
			                    //-while ((I2Cx->SR1 & 0x00040) != 0x000040);		//-模拟的时候就直接读了,没有这样的标志
			                    // Read DataN 
			                    *pBuffer = I2C1_ReceiveByte();
			                    I2C1_SendNACK();
			                    // Reset the number of bytes to be read by master 
			                    NumByteToRead = 0;	
			                 }
		              }
		              
		           	  I2C1_STOP();
		           	  return Success;		//-接收成功
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
		        if(res == TWI_READY)	//-判断总线是否空闲
		        {
		        	 i2c2_bus_error_cn = 0; 
		        	 // Send slave address 
		           // Reset the address bit0 for read 
		           SlaveAddress |= OAR1_ADD0_Set;		//-读写就是最后一位值决定的,这里就确保了值
		           Address = SlaveAddress;
		           // Send the slave address 
		           res = I2C2_SendByte(Address);
		           if(res == TWI_ACK)
		           {//-从机应答后,就可以开始读取总线上的数据了,这里读完数据后不需要发送ACK是发送NACK
		           	// Read the data
		           	  *pBuffer = I2C2_ReceiveByte();
		           	  I2C2_SendNACK();
		           	  I2C2_STOP();
		           	  return Success;		//-接收成功
		           }
		        }	  
		        else
		        	 i2c2_bus_error_cn++;                                                 
		    }
		    else if (NumByteToRead == 2)
		    {//-目前没有一次读两个字节的内容
		    	  // Send START condition 
		        res = I2C2_START();
		        if(res == TWI_READY)	//-判断总线是否空闲
		        {
		        	 i2c2_bus_error_cn = 0;
		        	 // Send slave address 
		           // Reset the address bit0 for read 
		           SlaveAddress |= OAR1_ADD0_Set;		//-读写就是最后一位值决定的,这里就确保了值
		           Address = SlaveAddress;
		           // Send the slave address 
		           res = I2C2_SendByte(Address);
		           if(res == TWI_ACK)
		           {
		           	  // Read first data,,这里总线上是否出现了有效数据需要我人为去考虑,这是模拟的,不能等待主机检测
		           	  *pBuffer = I2C2_ReceiveByte();
		           	  I2C2_SendACK();
		           	  pBuffer++;
		           	  // Read second data 
		              *pBuffer = I2C2_ReceiveByte();
		              I2C2_SendNACK();
		           	  I2C2_STOP();
		           	  return Success;		//-接收成功
		           }
		        }
		        else
		        	 i2c2_bus_error_cn++;	    	                           
		    }
		    else
		    {    
		        // Send START condition 
		        res = I2C2_START();
		        if(res == TWI_READY)	//-判断总线是否空闲
		        {
		        	 i2c2_bus_error_cn = 0;
		        	 // Send slave address 
		           // Reset the address bit0 for read 
		           SlaveAddress |= OAR1_ADD0_Set;		//-读写就是最后一位值决定的,这里就确保了值
		           Address = SlaveAddress;
		           // Send the slave address 
		           res = I2C2_SendByte(Address);
		           if(res == TWI_ACK)
		           {
		           	  // While there is data to be read 
		              while (NumByteToRead)
		              {
		        	         // Receive bytes from first byte until byte N-3
		        	         if (NumByteToRead != 3)	//-硬件读I2C最后三个字节需要特别处理,因为这些寄存器配置需要修改才能配置出最后结束的效果,前面的都是一样的,一个数据一个应答
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
			                    //-I2Cx->CR1 |= CR1_STOP_Set;		//-这里搞个停止位什么意思
			                    /* Read DataN-1 */
			                    *pBuffer = I2C2_ReceiveByte();
			                    I2C2_SendACK();
			                    // Increment 
			                    pBuffer++;
			                    // Wait until RXNE is set (DR contains the last data)
			                    //-while ((I2Cx->SR1 & 0x00040) != 0x000040);		//-模拟的时候就直接读了,没有这样的标志
			                    // Read DataN 
			                    *pBuffer = I2C2_ReceiveByte();
			                    I2C2_SendNACK();
			                    // Reset the number of bytes to be read by master 
			                    NumByteToRead = 0;	
			                 }
		              }
		              
		           	  I2C2_STOP();
		           	  return Success;		//-接收成功
		           }
		        } 
		        else
		        	 i2c2_bus_error_cn++;                   
		    }
    }
        	       
    return Error;			//-值为0
}
//-从机地址是SlaveAddress,数组pBuffer中不包含从机地址
int I2C_Master_BufferWrite(I2C_TypeDef* I2Cx, UINT8* pBuffer,  UINT32 NumByteToWrite, UINT8 Mode, UINT8 SlaveAddress )
{
	 int res;
   
   if(Mode == 1)
   {
		   res = I2C1_START();
		   if(res == TWI_READY)
		   {//-只有起始条件成功了才继续发送
		   	  i2c1_bus_error_cn = 0;
		   	  //- Send slave address 
		      //- Reset the address bit0 for write
		      SlaveAddress &= OAR1_ADD0_Reset;
		      Address = SlaveAddress;
		      //- Send the slave address 
		      //-Wait until ADDR is set: EV6 
		      res = I2C1_SendByte(Address);	//-会返回应答值ACK或NACK
		      if(res == TWI_ACK)
		      {
		      	 //- Write the first data in DR register (EV8_1) 
		         res = I2C1_SendByte(*pBuffer);		//-硬件I2C中这里等待数据发送结束后再发送下一个数据,这样可能进入死循环(万一人家不回答的话比如坏了),现在模拟I2C,
		         //- Increment              //-使用处理,对方不能成功ACK的话也认为没有接收成功
		         pBuffer++;
		         //- Decrement the number of bytes to be written 
		         NumByteToWrite--;
		         //- While there is data to be written,这里不再考虑是否应答成功的问题了,直接时序发送,默认是好的 
		         while (NumByteToWrite--)	//-把有的数据发送出去
		         {
		             // Poll on BTF to receive data because in polling mode we can not guarantee the
		             // EV8 software sequence is managed before the current byte transfer completes 
		             //-while ((I2Cx->SR1 & 0x00004) != 0x000004);
		             //-由于模拟的肯定发送出去了,所以不需要想硬件的那样等待标志位
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
				         // Make sure that the STOP bit is cleared by Hardware ,我觉的这里不需要等待,如果是模拟的话
				         //-while ((I2Cx->CR1&0x200) == 0x200);
				         return Success;
		         }
		      }	
		            
		   }
		   else
		   	  i2c1_bus_error_cn++;		//-这个目前不能加加了,前期这个判断功能已经去掉了
   }
   else if(Mode == 2)
   {
   	   res = I2C2_START();
		   if(res == TWI_READY)
		   {//-只有起始条件成功了才继续发送
		   	  i2c2_bus_error_cn = 0;
		   	  //- Send slave address 
		      //- Reset the address bit0 for write
		      SlaveAddress &= OAR1_ADD0_Reset;
		      Address = SlaveAddress;
		      //- Send the slave address 
		      //-Wait until ADDR is set: EV6 
		      res = I2C2_SendByte(Address);	//-会返回应答值ACK或NACK
		      if(res == TWI_ACK)
		      {
		      	 //- Write the first data in DR register (EV8_1) 
		         res = I2C2_SendByte(*pBuffer);		//-硬件I2C中这里等待数据发送结束后再发送下一个数据,这样可能进入死循环(万一人家不回答的话比如坏了),现在模拟I2C,
		         //- Increment              //-使用处理,对方不能成功ACK的话也认为没有接收成功
		         pBuffer++;
		         //- Decrement the number of bytes to be written 
		         NumByteToWrite--;
		         //- While there is data to be written,这里不再考虑是否应答成功的问题了,直接时序发送,默认是好的 
		         while (NumByteToWrite--)	//-把有的数据发送出去
		         {
		             // Poll on BTF to receive data because in polling mode we can not guarantee the
		             // EV8 software sequence is managed before the current byte transfer completes 
		             //-while ((I2Cx->SR1 & 0x00004) != 0x000004);
		             //-由于模拟的肯定发送出去了,所以不需要想硬件的那样等待标志位
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
				         // Make sure that the STOP bit is cleared by Hardware ,我觉的这里不需要等待,如果是模拟的话
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

//-调用平台接口读写数据,上层和硬件平台的关系在这里转化
//-msg  信息体数据;num  硬件端口号
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
		
		if(num == 1)	//-判断是那个硬件端口
		//-需要的参数从机地址,编程模式不一定需要,填写个数,数据数组,I2C端口号
    	 ret = I2C_Master_BufferWrite( I2C1, pBuffer, NumByteToWrite, num, SlaveAddress );
    else if(num == 2)
    	 ret = I2C_Master_BufferWrite( I2C2, pBuffer, NumByteToWrite, num, SlaveAddress );
    else if(num == 3)
    	 ret = I2C_Master_BufferWrite( I2C2, pBuffer, NumByteToWrite, num, SlaveAddress );	//-由于硬件仅仅支持两个I2C接口,所以第一个参数对于超过2的没有意义
    else if(num == 4)
    	 ret = I2C_Master_BufferWrite( I2C2, pBuffer, NumByteToWrite, num, SlaveAddress );
    else if(num == 5)
    	 ret = I2C_Master_BufferWrite( I2C2, pBuffer, NumByteToWrite, num, SlaveAddress );	 	 	 
    
    return ret;
}

//-底层I2C接收的处理函数
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
		
		if(num == 1)	//-判断是那个硬件端口
		//-需要的参数从机地址,编程模式不一定需要,填写个数,数据数组,I2C端口号
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
//-下面的I2C是模拟I2C  
void I2C_delay(void)
{	
   u8 i = I2C_Delay_Wait; //这里可以优化速度	，经测试最低到5还能写入
   while(i) 
   { 
     i--; 
   } 
}  

void I2C_SendByte(u8 SendByte) //数据从高位到低位//
{
    u8 i=8;
    while(i--)
    {
        SCL_L;	//-把GPIO对应引脚为设定为需要的电平
        I2C_delay();
      if(SendByte&0x80)	//-判断数据位值,然后至数
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

BOOL I2C_Start(void)	//?这样的操作有点像模拟I2C
{
	SDA_H;
	I2C_delay();
	SCL_H;
	I2C_delay();
	if(!SDA_read)return FALSE;	//SDA线为低电平则总线忙,退出
	SDA_L;
	I2C_delay();
	if(SDA_read) return FALSE;	//SDA线为高电平则总线出错,退出
	SDA_L;
	I2C_delay();
	return TRUE;
}

BOOL I2C_WaitAck(void) 	 //返回为:=1有ACK,=0无ACK
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

u8 I2C_ReceiveByte(void)  //数据从高位到低位//
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

//-下面读写操作使用的是库函数实现的	
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
	
	if (!I2C_Start())	//-判断现在总线所处的状态
	{
		GPIO_SetBits(I2C_PORT, I2C_WP_PIN);
		return;
	}
    I2C_SendByte(EEPROM_ADDRESS_WRITE);
    if (!I2C_WaitAck())	//-判断是否是需要的状态
	{
		I2C_Stop();		//-发送停止状态
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
	i = I2C_Delay_Stop;	//-延时10个毫秒以上，保证数据写入完成-
	while (i--)
	{
		I2C_delay();
	}
	GPIO_SetBits(I2C_PORT, I2C_WP_PIN);
}
*/
///////////////////////////////////模拟I2C END//////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*
  当主控要在总线上与从设备通信时，它将发送一个启动条件，紧跟着一个合适的 I2C 地址。
  启动条件向总线上所有的从设备提示启动新的数据交易的时间。带有指定 I2C 地址的从设
  备回应主控。其他所有从设备忽略总线上的后续数据流量，直至检测到下一个启动条件。
  当器件向数据交易发送 NACK 时，主机会尝试执行数据交易，直到它收到 ACK。
  主机一直发送带地址和读 / 写位信息的重复启动，直至器件发送ACK 为止。

*/


//-进行整体I2C硬件的初始化
void I2C_Init_sub(void)
{
	  
	  I2C_DeInit(I2C1);
	  I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = OwnAddress1;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = 100000;//100K速度
		//-Enables or disables the specified I2C peripheral.
    I2C_Cmd(I2C1, ENABLE);
    I2C_Init(I2C1, &I2C_InitStructure);
    /*允许1字节1应答模式*/
	  I2C_AcknowledgeConfig(I2C1, ENABLE);
	  //-I2C_LowLevel_Init(I2C1);
	  
	  I2C_DeInit(I2C2);
	  I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = OwnAddress1;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = 100000;//100K速度
		//-Enables or disables the specified I2C peripheral.
    I2C_Cmd(I2C2, ENABLE);
    I2C_Init(I2C2, &I2C_InitStructure);
    /*允许1字节1应答模式*/
	  I2C_AcknowledgeConfig(I2C2, ENABLE);
	  //-I2C_LowLevel_Init(I2C2);


}


//-以寄存器为单位读写
//-其中addr为从机地址；flags则是这次通信的标志，发送数据为0，接收数据则为 I2C_M_RD；
//-len为此次通信的数据字节数；buf 为发送或接收数据的指针。在设备驱动中我们通常调用 
//-i2c-core 定义的接口 i2c_master_send 和 i2c_master_recv 来发送或接收一次数据。

//- i2c_master_send 接口的三个参数：client 为此次与主机通信的从机，buf 为发送的数据指针，count 为发送数据的字节数。
int i2c_master_send(struct i2c_client *client,u8 *buf ,int count)		//-CY8CMBR3xxx_WriteData();和这是同等地位
{
     int ret; 
     //-struct i2c_adapter *adap=client->adapter;  // 获取adapter信息 ,,这个目前移植不需要
     struct i2c_msg msg;                        // 定义一个临时的数据包 

     msg.addr = client->addr;                   // 将从机地址写入数据包 
     msg.flags = client->flags & I2C_M_TEN;     // 将从机标志并入数据包 
     msg.len = count;                           // 将此次发送的数据字节数写入数据包 
     msg.buf = (u8 *)buf;                     // 将发送数据指针写入数据包 

     //-ret = i2c_transfer(adap, &msg, 1);         // 调用平台接口发送数据 
     ret = i2c_transfer(&msg, client->num);         // 调用平台接口发送数据 ,,后面的参数可以是个数,也可以预留位端口号

     /* If everything went ok (i.e. 1 msg transmitted), return #bytes transmitted, else error code. */ 
     return (ret == 1) ? count : ret;           // 如果发送成功就返回字节数 
}

//- i2c_master_recv 接口的三个参数：client 为此次与主机通信的从机，buf 为接收的数据指针，
//-count 为接收数据的字节数。我们看一下 i2c_transfer 接口的参数说明：
int i2c_master_recv(struct i2c_client *client, u8 *buf ,int count) 
{
     //-struct i2c_adapter *adap=client->adapter;  // 获取adapter信息 
     struct i2c_msg msg;                        // 定义一个临时的数据包 
     int ret; 
  
     msg.addr = client->addr;                   // 将从机地址写入数据包 
     msg.flags = client->flags & I2C_M_TEN;     // 将从机标志并入数据包 
     msg.flags |= I2C_M_RD;                     // 将此次通信的标志并入数据包 
     msg.len = count;                           // 将此次接收的数据字节数写入数据包 
     msg.buf = buf; 
  
     ret = i2c_receive(&msg, client->num);         // 调用平台接口接收数据 
  
     /* If everything went ok (i.e. 1 msg transmitted), return #bytes
        transmitted, else error code. */ 
     return (ret == 1) ? count : ret;           // 如果接收成功就返回字节数 
} 
 

// I2C read x byte data from register ,一个字节是并行关系,然后通过I2C处理,需要使用串行交流
int i2c_read_data_Buffer(struct i2c_client *client,u8 numberOfBytes,u8 *data)
{
	//-u8 databuf[2]; 
	int res = 0;
	//-databuf[0]= *data;		//-开始的寄存器地址
	

	res = i2c_master_recv(client,data,numberOfBytes);
	if(res <= 0)
	{
	//-	//-APS_ERR("i2c_master_recv function err\n");
		return res;
	}

	
	return 1;
}

// I2C read one byte data from register ,一个字节是并行关系,然后通过I2C处理,需要使用串行交流
int i2c_read_reg(struct i2c_client *client,u8 reg,u8 *data)
{
	u8 databuf[2]; 
	int res = 0;
	databuf[0]= reg;
	
	//-目前测试显示可能会有I2C数据填写出错,需要保证肯定正确,否则出现寄存器配置不对,那么器件就无法工作
	res = i2c_master_send(client,databuf,0x1);	
	if(res <= 0)
	{
	//-	//-APS_ERR("i2c_master_send function err\n");
		return res;
	}
	//-while(res <= 0)
	//-{
  //-    delay_temp(10000);
	//-		res = i2c_master_send(client,databuf,0x1); //-实际使用程序时不能这么处理,会出现死循环而不能恢复
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
	//-		res = i2c_master_recv(client,databuf,0x1);		//-实际使用程序时不能这么处理,会出现死循环而不能恢复
	//-    delay_temp(10000);
  //-}
	
	
	return 1;
}

// I2C read x byte data from register ,一个字节是并行关系,然后通过I2C处理,需要使用串行交流
int i2c_read_reg_Buffer(struct i2c_client *client,u8 numberOfBytes,u8 *data)
{
	u8 databuf[2]; 
	int res = 0;
	databuf[0]= *data;		//-开始的寄存器地址
	
	//-目前测试显示可能会有I2C数据填写出错,需要保证肯定正确,否则出现寄存器配置不对,那么器件就无法工作
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


int i2c_write_data(struct i2c_client *client,u8 reg,u8 value)		//-直接向I2C从设备发送数据,不定位到寄存器上,由于不关心寄存器所以参数reg 无所谓,默认给0 就行
{
	u8 databuf[2];	//-地址和数据都是以"字节"的形式发送出去的
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

//-reg 寄存器地址;value 填写的数据
// I2C Write one byte data to register
//?如何保证写入数据绝对正确,如果由于外部干扰或者总线故障而正确执行程序
//?使影响最小
int i2c_write_reg(struct i2c_client *client,u8 reg,u8 value)
{
	u8 databuf[2];	//-地址和数据都是以"字节"的形式发送出去的
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

//-这里填写的是一个连续寄存器的内容
//-缓冲区第一个字节是固定的偏移地址然后是所有的数据,发送的字节数=地址+数据个数(地址就是寄存器偏移量恒等于1)
int i2c_write_reg_Buffer(struct i2c_client *client,u8 *writeBuffer,u8 numberOfBytes)
{
	//-u8 databuf[2];	//-地址和数据都是以"字节"的形式发送出去的
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