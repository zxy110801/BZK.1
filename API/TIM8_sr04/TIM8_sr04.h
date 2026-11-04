#ifndef __TIM8_SR04_H__
#define __TIM8_SR04_H__
#include "stm32f10x.h"                  // Device header
#include "stdio.h"
#include "TIM.h"
void TIM8_SR04_Init(void);
void HC_SR04_Trig(void);
void HC_SR04_Scan(void);
volatile extern u32 HC_SR04_Over;
volatile extern u8 HC_SR04_OverFlag;

#endif
