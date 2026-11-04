#include "BEEP.h"
void Beep_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void Beep_ON(void)
{
	GPIO_SetBits(GPIOB, GPIO_Pin_7);
}
void Beep_OFF(void)
{
	GPIO_ResetBits(GPIOB, GPIO_Pin_7);
}
void Beep_FZ(void)
{
	uint8_t Beep_Flag = GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_7);
	GPIO_WriteBit(GPIOB, GPIO_Pin_7, (Beep_Flag == Bit_SET) ? Bit_RESET : Bit_SET);
	// BEEP_TOGGLE();
}
