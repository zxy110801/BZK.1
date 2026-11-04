#include "Systick.h"
#include "Key.h"

void Systick_Init(void)
{
	
	if (SysTick_Config(72000000 / 1000))
	{
		while (1)
			;
	}
}

volatile uint64_t SysTick_time = 0;

void SysTick_Handler(void)
{
	SysTick_time++;
	Key_Scan(); /* 1ms 一次，在中断里推进按键状态机 */
}

uint64_t Get_SysTick(void)
{
	return SysTick_time;
}
