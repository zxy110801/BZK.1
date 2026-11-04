#ifndef __ULTRASONIC_H__
#define __ULTRASONIC_H__
#include "stm32f10x.h" // Device header
#include "TIM.h"
#include "Delay.h"
#include "stdio.h"
extern u32 Rising_Flag;
extern volatile u8 Falling_Flag;
extern u32 Time_Length;
extern double chao_Length;
void ultrasonic_Init(void);
void ultrasonic_Trig(void);

#endif
