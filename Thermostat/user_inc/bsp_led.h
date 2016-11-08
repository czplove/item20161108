


#ifndef __BSP_LED_H__
#define __BSP_LED_H__

//#define PowerLED(BitVal)  		GPIO_WriteBit(GPIOC,GPIO_Pin_10,BitVal)
//#define MenuLED(BitVal)  		GPIO_WriteBit(GPIOC,GPIO_Pin_12,BitVal)
//#define UpLED(BitVal)  		    GPIO_WriteBit(GPIOC,GPIO_Pin_11,BitVal)
//#define HeatLED(BitVal)  		GPIO_WriteBit(GPIOC,GPIO_Pin_1,BitVal)
//#define DownLED(BitVal)  		GPIO_WriteBit(GPIOC,GPIO_Pin_9,BitVal)
//#define WindLED(BitVal)  		GPIO_WriteBit(GPIOB,GPIO_Pin_9,BitVal)
//#define CoolLED(BitVal)  		GPIO_WriteBit(GPIOB,GPIO_Pin_10,BitVal)
//#define AirLED(BitVal)  		GPIO_WriteBit(GPIOB,GPIO_Pin_11,BitVal)


#define PowerLED(BitVal)  		GPIO_WriteBit(GPIOB,GPIO_Pin_4,BitVal)
#define UpLED(BitVal)  		    GPIO_WriteBit(GPIOA,GPIO_Pin_5,BitVal)
#define MenuLED(BitVal)  		GPIO_WriteBit(GPIOB,GPIO_Pin_9,BitVal)
#define DownLED(BitVal)  		GPIO_WriteBit(GPIOB,GPIO_Pin_5,BitVal)
#define WindLED(BitVal)  		GPIO_WriteBit(GPIOA,GPIO_Pin_4,BitVal)

#define AirLED(BitVal)  		GPIO_WriteBit(GPIOC,GPIO_Pin_11,BitVal) //-对应风扇上的灯
#define CoolLED(BitVal)  		GPIO_WriteBit(GPIOC,GPIO_Pin_10,BitVal)
#define HeatLED(BitVal)  		GPIO_WriteBit(GPIOB,GPIO_Pin_13,BitVal)






#endif // __BSP_LED_H__


