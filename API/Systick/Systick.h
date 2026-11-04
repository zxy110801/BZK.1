#ifndef __SYSTICK_H__
#define __SYSTICK_H__
#include "stm32f10x.h"

void Systick_Init(void);
uint64_t Get_SysTick(void);

#endif
