#ifndef __DMA_H__
#define __DMA_H__
#include "stm32f10x.h" // Device header

extern u8 isFinished;


//初始化
void DMA1_Init(void);
//数据传输
void DMA1_Transmit(u32 srcAddr , u32 destAddr , u16 datalen);



#endif
