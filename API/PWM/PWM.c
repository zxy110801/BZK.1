#include "PWM.h"
/**
  * 函    数：PWM初始化
  * 参    数：无
  * 返 回 值：无
  */
void TIM1_PWM_Init(void)
{
    //
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    // 配置GPIOE9为PWM输出
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	
    // 配置TIM1为PWM模式
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructure.TIM_Prescaler = 720 - 1;// 预分频为720
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;// 不分频
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数模式
    TIM_TimeBaseStructure.TIM_Period = 100 -1 ; // 自动重装载值ARR为100
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

    // 配置TIM1的通道1为PWM模式
    TIM_OCInitTypeDef TIM_OCInitStructure;
		TIM_OCStructInit(&TIM_OCInitStructure);	//机构体初始化,若结构体没有完整赋值,给结构体成员赋一个默认值
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_Pulse = 50;        
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
		TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OC1Init(TIM1, &TIM_OCInitStructure);
    // 中断使能
		TIM_CtrlPWMOutputs(TIM1,ENABLE);
		TIM_Cmd(TIM1, ENABLE);
 
 
		
}









void TIM3_PWM_Init(void)
{
	//实现led灯呼吸灯部分
		
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
		
		GPIO_InitTypeDef GPIO_InitStructure;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB,&GPIO_InitStructure);
		
		TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
		TIM_TimeBaseStructure.TIM_Prescaler = 72 - 1;
		TIM_TimeBaseStructure.TIM_Period = 1000 - 1;
		TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
		TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
		TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);
		
	
		TIM_OCInitTypeDef TIM_OCInitStructure;
		TIM_OCStructInit(&TIM_OCInitStructure);
		TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_Pulse = 0;        
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
		TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
		
    TIM_OC3Init(TIM3, &TIM_OCInitStructure);
		TIM_OC4Init(TIM3, &TIM_OCInitStructure);
		// 中断使能
		   
   	TIM_Cmd(TIM3, ENABLE);
}


void TIM3_BZKPWM_Init(void)
{
	//时钟源使能
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
	//配置为复用推挽输出
	GPIOB->CRL &= ~GPIO_CRL_CNF;
	GPIOB->CRL |= GPIO_CRL_CNF0_1;
	GPIOB->CRL &= ~GPIO_CRL_MODE;
	GPIOB->CRL |= GPIO_CRL_MODE0_1;
	GPIOB->CRL |= GPIO_CRL_MODE0_0;
	
	
	//1.TIM3通用定时器相关配置
	//1.1 预分频系数配置 (TIMx_PSC)
	//不分频
	TIM3->CR1 &= ~TIM_CR1_CKD;
	//预分频为72分频
	TIM3->PSC = 72 - 1;
	
	//1.2	自动重装载寄存器(TIMx_ARR) 
	TIM3->ARR = 1000 - 1;
	
	//1.3 PWM向上计数模式
	TIM3->CR1 &= ~TIM_CR1_CMS; 	//边沿对齐模式
	TIM3->CR1 &= ~TIM_CR1_DIR;	//向上计数模式
	
	//1.4 PWM1 在低于CCR的状态为有效电平
	TIM3->CCMR2 &= ~TIM_CCMR2_CC3S; //配置为输出比较模式
	//PWM1模式,在低于CCR的状态输出有效电平
	TIM3->CCMR2 &= ~TIM_CCMR2_OC3M;
	TIM3->CCMR2 &= ~TIM_CCMR2_OC3M_0;   
	TIM3->CCMR2 |= TIM_CCMR2_OC3M_1;
	TIM3->CCMR2 |= TIM_CCMR2_OC3M_2;
	//配置CCR为50
	TIM3->CCR3 = 50;
	//有效电平为低电平
	TIM3->CCER |= TIM_CCER_CC3P;
	//输出比较使能
	TIM3->CCER |= TIM_CCER_CC3E;
}
void TIM3_Start(void)
{
	//CNT使能
	TIM3->CR1 |= TIM_CR1_CEN;
}
void TIM3_Stop(void)
{
	TIM3->CR1 &= ~TIM_CR1_CEN;
}
void TIM3_SetDutyCycle(u8 dutyCycle)
{
	TIM3->CCR3 = dutyCycle * 10;
}
void TIM4_PWM_Init(void)
{
	//实现led灯Beep部分
		
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
		
		GPIO_InitTypeDef GPIO_InitStructure;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB,&GPIO_InitStructure);
		
		TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
		TIM_TimeBaseStructure.TIM_Prescaler = 72 - 1;
		TIM_TimeBaseStructure.TIM_Period = 1000 - 1;
		TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
		TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
		TIM_TimeBaseInit(TIM4,&TIM_TimeBaseStructure);
	
		TIM_OCInitTypeDef TIM_OCInitStructure;
		TIM_OCStructInit(&TIM_OCInitStructure);
		TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_Pulse = 0;        
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
		TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
		
    TIM_OC2Init(TIM4, &TIM_OCInitStructure);
		
		// 中断使能
   	TIM_Cmd(TIM4, ENABLE);
}



/**
  * 函    数：PWM设置CCR
  * 参    数：Compare 要写入的CCR的值，范围：0~100
  * 返 回 值：无
  * 注意事项：CCR和ARR共同决定占空比，此函数仅设置CCR的值，并不直接是占空比
  *      占空比Duty = CCR / (ARR + 1)
  */
void PWM1_SetComparel(uint16_t Compare)
{
	TIM_SetCompare1(TIM1,Compare);
}

/**
  * 函    数：PWM设置CCR
  * 参    数：Compare 要写入的CCR的值，范围：0~100
  * 返 回 值：无
  * 注意事项：CCR和ARR共同决定占空比，此函数仅设置CCR的值，并不直接是占空比
  *      占空比Duty = CCR / (ARR + 1)
  */
void PWM3_SetComparel(uint16_t Compare)
{
	TIM_SetCompare3(TIM3,Compare);
	TIM_SetCompare4(TIM3,Compare);
}
//实现呼吸灯
void PWM3_Scan(void)
{
	static uint32_t tick_led_old = 0;
		static uint16_t PWM_LED_CRR = 0;
		static uint8_t PWM_LED_flag = 0;
		if (Get_SysTick() - tick_led_old >= 50)
		{
			tick_led_old = Get_SysTick();
			
			if(PWM_LED_CRR >= 100)
			{
				PWM_LED_flag = 1;
			}
			else if(PWM_LED_CRR <= 0)
			{
				PWM_LED_flag = 0;
			}
			if(PWM_LED_flag == 1)
			{
				PWM_LED_CRR -= 5;
			}
			else if(PWM_LED_flag == 0)
			{
				PWM_LED_CRR += 5;
			}
			PWM3_SetComparel(PWM_LED_CRR);
		}
}


