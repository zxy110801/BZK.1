#ifndef __USART_H__
#define __USART_H__
#include "stm32f10x.h" // Device header
#include "stdio.h"
#include "LED.h"
#include "Beep.h"
//±ê×¼¿â
void BZKusart_Init(void);
//¼Ä´æÆ÷
void JZQUSART_Init(void);




void usart1_Send(uint8_t *buff, uint8_t led);
int fputc(int ch, FILE *f);
#endif
