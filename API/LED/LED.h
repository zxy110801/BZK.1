#ifndef __LED_H__
#define __LED_H__
#include "Delay.h"
#include "stm32f10x.h"                  // Device header
#include "LED.h"
#include "KEY.h"
void LED_Init(void);
void LED_ON(uint8_t LED_Set);
void LED_OFF(uint8_t LED_Set);
void LED_Show(void);
void LED_ShowPWM(void);
void LED_FZ(uint8_t LED_set);
void LED_flash(uint8_t LED_set,uint16_t flash_time);
#endif
