#include "TIM.h"

/*
	TIM6
	
*/
void TIM6_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6,ENABLE);
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period = 65535 - 1;
	TIM_TimeBaseInitStructure.TIM_Prescaler = 72 - 1;
	TIM_TimeBaseInit(TIM6,&TIM_TimeBaseInitStructure);
	TIM_ITConfig(TIM6,TIM_IT_Update,ENABLE);
	
	NVIC_InitTypeDef NVIC_TIM6InitStructure;
	NVIC_TIM6InitStructure.NVIC_IRQChannel = TIM6_IRQn;
	NVIC_TIM6InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_TIM6InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_TIM6InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_Init(&NVIC_TIM6InitStructure);
	TIM_Cmd(TIM6, ENABLE);
}
void TIM7_Init(void)
{
	RCC->APB1ENR |= (1 << 5);
	
	TIM7->PSC = 72 - 1;     //预分频器为72 - 1
	TIM7->ARR = 1000 - 1;    //自动重装载值为1000 - 1
	//产生一次更新事件
	TIM7->EGR |= 1;
	//清一次更新标志
	TIM7->SR &= ~1;
	
	//使能更新中断
	TIM7->DIER |= 1;
           
	
	//配置NVIC
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn ;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);
	
	//使能
	TIM7->CR1 |= (0x05<<0);
}

//配置32系统主频为48Mhz
void System_Clock_48MHz(void)
{
    /* ========== 1. 复位 RCC 到默认状态 ========== */
    RCC->CR |= 0x00000001;              // HSION = 1，先确保 HSI 开着（安全）
    RCC->CFGR &= 0xF8FF0000;            // 清 SW/HPRE/PPRE1/PPRE2/ADCPRE/MCO
    RCC->CR &= 0xFEF6FFFF;              // 清 HSEON/CSSON/PLLON
    RCC->CR &= 0xFFFBFFFF;              // 清 HSEBYP
    RCC->CFGR &= 0xFF80FFFF;            // 清 PLLSRC/PLLXTPRE/PLLMUL/USBPRE

    /* ========== 2. 启动 HSE ========== */
    RCC->CR |= 0x00010000;              // HSEON = 1
    while(!(RCC->CR & 0x00020000));     // 等 HSERDY = 1

    /* ========== 3. 配置 Flash（48MHz 需 1 等待周期）========== */
    FLASH->ACR &= ~0x00000007;          // 清 LATENCY 位
    FLASH->ACR |=  0x00000001;          // LATENCY = 1（48MHz 对应 1 周期）
    FLASH->ACR |=  0x00000010;          // PRFTBE = 1，使能预取缓冲
    // FLASH->ACR |= 0x00000100;        // PRFTBS 只读，别写

    /* ========== 4. 配置分频 HPRE / PPRE1 / PPRE2 ========== */
    // CFGR 位：HPRE[7:4]，PPRE1[10:8]，PPRE2[13:11]
    RCC->CFGR &= ~(0xF << 4);           // 清 HPRE
    RCC->CFGR |=  (0x0 << 4);           // HPRE = 0000，HCLK = SYSCLK（不分频）

    RCC->CFGR &= ~(0x7 << 8);           // 清 PPRE1
    RCC->CFGR |=  (0x4 << 8);           // PPRE1 = 100，APB1 = HCLK/2 = 24MHz 

    RCC->CFGR &= ~(0x7 << 11);          // 清 PPRE2
    RCC->CFGR |=  (0x0 << 11);          // PPRE2 = 000，APB2 = HCLK = 48MHz 

    /* ========== 5. 配置 PLL：HSE × 6 = 48MHz ========== */
    RCC->CFGR &= ~(0x1 << 16);          // PLLSRC = 0，PLL 源选 HSE
    // PLLXTPRE = 0（HSE 不分频），保持默认
    RCC->CFGR &= ~(0xF << 18);          // 清 PLLMUL[21:18]
    RCC->CFGR |=  (0x4 << 18);          // PLLMUL = 0100，×6 → 8×6=48MHz

    /* ========== 6. 使能 PLL 并等待锁定 ========== */
    RCC->CR |= 0x01000000;              // PLLON = 1
    while(!(RCC->CR & 0x02000000));     // 等 PLLRDY = 1

    /* ========== 7. 切换 SYSCLK 到 PLL ========== */
    RCC->CFGR &= ~0x3;                  // 清 SW
    RCC->CFGR |=  0x2;                  // SW = 10，选 PLL 作系统时钟
    while((RCC->CFGR & (0x3 << 2)) != (0x2 << 2));  // 等 SWS = 10，确认切换成功

    /* ========== 8. 更新全局变量（如果用标准库的延时/串口）========== */
    SystemCoreClock = 48000000;
}   














volatile uint64_t TIM6_time;

uint32_t Rising_Flag = 0;
volatile uint8_t Falling_Flag = 0;
uint32_t TimeLength_UP = 0;
uint32_t Time_Length = 0;


void TIM6_IRQHandler(void)
{
		if(TIM_GetITStatus(TIM6,TIM_IT_Update))
		{
			TIM_ClearITPendingBit(TIM6,TIM_IT_Update);
		}
}
uint32_t TIM7_Time = 0;

void TIM7_IRQHandler(void)
{
    if(TIM7->SR & 1)                
    {
        TIM7->SR &= ~1;     
				TIM7_Time++;
			
        if(TIM7_Time > 1000)
				{
					TIM7_Time = 0;
					ultrasonic_Trig();
				}
				
    }
}
