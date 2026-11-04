#ifndef __PWM_H__
#define __PWM_H__
#include "stm32f10x.h" // Device header
#include "stdio.h"
#include "Systick.h"
void TIM1_PWM_Init(void);
void PWM1_SetComparel(uint16_t Compare);
void TIM3_PWM_Init(void);
void TIM3_BZKPWM_Init(void);
void PWM3_SetComparel(uint16_t Compare);
void TIM4_PWM_Init(void);
void TIM3_Start(void);
void TIM3_Stop(void);
void TIM3_SetDutyCycle(u8 dutyCycle);
#endif
