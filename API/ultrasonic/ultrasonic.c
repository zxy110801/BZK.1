#include "ULTRASONIC.h"

void ultrasonic_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
	// PB12为Trig
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// PB11为Echo
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_WriteBit(GPIOB, GPIO_Pin_12, Bit_RESET);

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource11);

	EXTI_InitTypeDef EXTI_InitStructure;
	EXTI_InitStructure.EXTI_Line = EXTI_Line11;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_Init(&EXTI_InitStructure);

	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStructure);
}

void ultrasonic_Trig(void)
{
	Rising_Flag = 0;
	Falling_Flag = 0;
	Time_Length = 0;

	GPIO_SetBits(GPIOB, GPIO_Pin_12);	// 拉高PB12
	Delay_us(20);						// 等待至少15us，确保>10us
	GPIO_ResetBits(GPIOB, GPIO_Pin_12); // 给完高电平自动拉低PB12

	TIM_SetCounter(TIM6, 0); // 清零TIM6
}

void EXTI15_10_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line11) == SET)
	{
		if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == SET)
		{
			Rising_Flag = TIM_GetCounter(TIM6);
			Falling_Flag = 0; // 新一次测量开始
		}
		else
		{
			if (Rising_Flag != 0 || /* 允许 0 值 */ 1)
			{
				Time_Length = TIM_GetCounter(TIM6) - Rising_Flag;
				Falling_Flag = 1;
			}
		}
		EXTI_ClearITPendingBit(EXTI_Line11);
	}
}



void ultrasonic_Trig_Scan(void)
{
	if (key[0].key_dan_flg == 1)
    		{
			key[0].key_dan_flg = 0;
			LED_FZ(3);
			ultrasonic_Trig();
		}
		if (Falling_Flag == 1)
		{
			__disable_irq();
			uint32_t t = Time_Length;
			Falling_Flag = 0;
			__enable_irq();

			chao_Length = 340.0f * t / 1e4f / 2.0f;
			printf("超声波测量距离:%.3f cm\n", chao_Length);
		}
	
}


