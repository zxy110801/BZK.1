#ifndef __ADC_H__
#define __ADC_H__
#include "stm32f10x.h"                  // Device header
//单通道连续转换相关函数

void ADC1_Init(void);
//ADC校准开始转换
void ADC1_StartConvert(void);
//获取ADC值
double ADC1_ReadV(void);

void ADC1_DMA_Init(void);
void ADC1_DMA_StartConvert(u32 destAddr , u8 len);
void ADC1_BZK_Init(void);
double ADC1_Read(void);
#endif
