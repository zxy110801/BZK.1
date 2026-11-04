/*
 * TIM8_sr04.c
 * 功能:基于 TIM1 输入捕获的 HC-SR04 超声波测距
 */
#include "TIM8_sr04.h"

/* 
	输入捕获方式测量超声波往返时间:
	PC9 TIM1_CH2 Trig  发波(触发输出)
	PA8 TIM1_CH1 Echo  回波(捕获输入)
*/
void TIM8_SR04_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_TIM1 | RCC_APB2Periph_GPIOC,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	// PC9 Trig 触发输出(拉高约 20us)
	// 配置 PC9 为推挽输出
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC,&GPIO_InitStructure);
	GPIO_ResetBits(GPIOC, GPIO_Pin_9);
	
	// PA8 TIM1_CH1 Echo 捕获输入
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	TIM_TimeBaseInitTypeDef TIM1_InitStructure;
	TIM1_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM1_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM1_InitStructure.TIM_Period = 65535 - 1;
	TIM1_InitStructure.TIM_Prescaler = 72 - 1;
	TIM1_InitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM1 , &TIM1_InitStructure);
	
	TIM_ICInitTypeDef TIM1_ICInitStructure;
	TIM1_ICInitStructure.TIM_Channel = TIM_Channel_1;
	TIM1_ICInitStructure.TIM_ICFilter = 0x03;
	TIM1_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM1_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM1_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;

	TIM_PWMIConfig(TIM1,&TIM1_ICInitStructure); // 只需配置一次
	
	TIM_CCxCmd(TIM1,TIM_Channel_1,TIM_CCx_Enable);
	TIM_CCxCmd(TIM1,TIM_Channel_2,TIM_CCx_Enable);  //开启通道2之后,输入捕获直接相连

	// 开启输入捕获与更新中断
	TIM_ITConfig(TIM1,TIM_IT_CC1,ENABLE);
	TIM_ITConfig(TIM1,TIM_IT_CC2,ENABLE);
	TIM_ITConfig(TIM1,TIM_IT_Update,ENABLE);
	
	NVIC_InitTypeDef NVIC_ICInitStructure;
	// TIM1 捕获比较中断
	NVIC_ICInitStructure.NVIC_IRQChannel = TIM1_CC_IRQn;
	NVIC_ICInitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_ICInitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_ICInitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_ICInitStructure);
	// TIM1 更新中断
	NVIC_ICInitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;
	NVIC_ICInitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_ICInitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_ICInitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_Init(&NVIC_ICInitStructure);

	TIM_Cmd(TIM1,ENABLE);
}



/* 发出一次超声波触发脉冲,并清零计时器 */
void HC_SR04_Trig(void)
{
	GPIO_SetBits(GPIOC, GPIO_Pin_9);	
	Delay_us(20);						// 等待大于 15us 确保超过 10us
	GPIO_ResetBits(GPIOC, GPIO_Pin_9); 
	TIM_SetCounter(TIM1, 0); 
}


void HC_SR04_Scan(void)
{
		HC_SR04_Trig();
		Delay_ms(300);
		if (HC_SR04_OverFlag == 1)
		{
			__disable_irq();
			uint32_t t = HC_SR04_Over;
			HC_SR04_OverFlag = 0;
			__enable_irq();

			chao_Length = 340.0f * t / 1e4f / 2.0f;
			printf("测距结果:%.3f cm\n", chao_Length);
		}
		else if(HC_SR04_OverFlag == 2)
		{
			printf("超出量程,为无效数据!\n");
		}
}







u16 HC_SR04_Start = 0;
volatile u32 HC_SR04_Over = 0;
volatile u8 HC_SR04_OverFlag = 0;
u8 HC_SR04_StartFlag = 0;
void TIM1_CC_IRQHandler(void)// 输入捕获中断
{
	if(TIM_GetITStatus(TIM1,TIM_IT_CC1))   // 上升沿中断
	{
		TIM_ClearITPendingBit(TIM1,TIM_IT_CC1);
		HC_SR04_Start = TIM_GetCapture1(TIM1);// 读取 CH1 捕获寄存器(上升沿时刻)
		HC_SR04_StartFlag = 1;
	}
	if(TIM_GetITStatus(TIM1,TIM_IT_CC2))   // 下降沿中断
	{
		TIM_ClearITPendingBit(TIM1,TIM_IT_CC2);
		if(HC_SR04_StartFlag == 1)
		{
			HC_SR04_Over = TIM_GetCapture2(TIM1) - HC_SR04_Start; // 读取 CH2 捕获寄存器(下降沿),减去起始值
			HC_SR04_OverFlag = 1;
			HC_SR04_StartFlag = 0;
		}
	}
}
/* 更新中断:超时未收到回波,标记超时 */
void TIM1_UP_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM1,TIM_IT_Update))
	{
		TIM_ClearITPendingBit(TIM1,TIM_IT_Update);
		if(HC_SR04_StartFlag == 1)
		{
			HC_SR04_OverFlag = 2;
			HC_SR04_StartFlag = 0;
		}
	}
}

