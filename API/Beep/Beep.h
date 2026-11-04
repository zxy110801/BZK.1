#ifndef __BEEP_H__
#define __BEEP_H__
#include "stm32f10x.h" // Device header
#include "Key.h"
void Beep_Init(void);
void Beep_ON(void);
void Beep_OFF(void);
void Beep_FZ(void);

#endif
