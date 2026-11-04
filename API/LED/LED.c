#include "LED.h"

void LED_Init()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_0 |GPIO_Pin_1|GPIO_Pin_2);
}
void LED_ON(uint8_t LED_Set)
{
	switch(LED_Set)
	{
		case 0:GPIO_ResetBits(GPIOB,GPIO_Pin_0);break;
		case 1:GPIO_ResetBits(GPIOB,GPIO_Pin_1);break;
		case 2:GPIO_ResetBits(GPIOB,GPIO_Pin_2);break;
		default:GPIO_ResetBits(GPIOB,GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2);break;
	}
}
void LED_OFF(uint8_t LED_Set)
{
	switch(LED_Set)
	{
		case 0:GPIO_SetBits(GPIOB,GPIO_Pin_0);break;
		case 1:GPIO_SetBits(GPIOB,GPIO_Pin_1);break;
		case 2:GPIO_SetBits(GPIOB,GPIO_Pin_2);break;
		default:GPIO_SetBits(GPIOB,GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2);break;
	}
}
void LED_FZ(uint8_t LED_set)
{
	switch(LED_set)
	{
		case 0:GPIO_WriteBit(GPIOB,GPIO_Pin_0,GPIO_ReadOutputDataBit(GPIOB,GPIO_Pin_0)? Bit_RESET:Bit_SET);break;
		case 1:GPIO_WriteBit(GPIOB,GPIO_Pin_1,GPIO_ReadOutputDataBit(GPIOB,GPIO_Pin_1)? Bit_RESET:Bit_SET);break;
		case 2:GPIO_WriteBit(GPIOB,GPIO_Pin_2,GPIO_ReadOutputDataBit(GPIOB,GPIO_Pin_2)? Bit_RESET:Bit_SET);break;
		default:
		{		
			GPIO_WriteBit(GPIOB,GPIO_Pin_0,GPIO_ReadOutputDataBit(GPIOB,GPIO_Pin_0)? Bit_RESET:Bit_SET);
			GPIO_WriteBit(GPIOB,GPIO_Pin_1,GPIO_ReadOutputDataBit(GPIOB,GPIO_Pin_1)? Bit_RESET:Bit_SET);
			GPIO_WriteBit(GPIOB,GPIO_Pin_2,GPIO_ReadOutputDataBit(GPIOB,GPIO_Pin_2)? Bit_RESET:Bit_SET);
		}break;
	}
}
void LED_flash(uint8_t LED_set,uint16_t flash_time)
{
	switch(LED_set)
	{
		case 0:{
			LED_FZ(0);
			Delay_ms(flash_time);
		};break;
		case 1:{
			LED_FZ(1);
			Delay_ms(flash_time);
		};break;
		case 2:{
			LED_FZ(2);
			Delay_ms(flash_time);
		};break;
		default:
		{		
			LED_FZ(3);
			Delay_ms(flash_time);
		}break;
	}
}
void LED_Show()
{
	LED_ON(0);
	Delay_ms(500);
	LED_OFF(0);
	LED_ON(1);
	Delay_ms(500);
	LED_OFF(1);
	LED_ON(2);
	Delay_ms(500);
	LED_OFF(2);
	Delay_ms(500);
}
void LED_ShowPWM(void)
{
	int PWM_flag;
	for(PWM_flag = 0; PWM_flag<1000;PWM_flag++)
	{
		LED_ON(3);
		Delay_us(PWM_flag);
		LED_OFF(3);
		Delay_us(1000 - PWM_flag);
	}
	for(PWM_flag = 1000; PWM_flag > 1;PWM_flag--)
	{
		LED_ON(3);
		Delay_us(PWM_flag);
		LED_OFF(3);
		Delay_us(1000 - PWM_flag);
	}
}

