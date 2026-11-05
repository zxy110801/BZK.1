#include "I2C.h"
// SDA PD4    SCL PD5

void I2C1_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; /* 开漏 + 外部上拉 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	SCL_HIGH;
	SDA_HIGH;
}

/* START / Repeated START：SCL 为高时 SDA 下降，然后拉低 SCL 再发数据 */
void I2C_Start(void)
{
	SDA_HIGH;
	SCL_HIGH;
	I2C_Delay;

	SDA_LOW; /* START：SCL=1 时 SDA↓ */
	I2C_Delay;

	SCL_LOW; /* 进入可改数据区间，供随后 SendByte */
	I2C_Delay;
}

/* STOP：SCL 为高时 SDA 上升 */
void I2C_Stop(void)
{
	SCL_LOW;
	SDA_LOW;
	I2C_Delay;

	SCL_HIGH;
	I2C_Delay;

	SDA_HIGH; /* STOP：SCL=1 时 SDA↑ */
	I2C_Delay;
}

void I2C_ACK(void)
{
	SCL_LOW;
	SDA_LOW; /* 应答：SDA=0 */
	I2C_Delay;

	SCL_HIGH; /* 从机采样 ACK */
	I2C_Delay;

	SCL_LOW;
	SDA_HIGH; /* 释放 SDA */
	I2C_Delay;
}

void I2C_NACK(void)
{
	SCL_LOW;
	SDA_HIGH; /* 非应答：SDA=1 */
	I2C_Delay;

	SCL_HIGH; /* 从机采样 NACK（原先缺少这一拍） */
	I2C_Delay;

	SCL_LOW;
	I2C_Delay;
}

uint8_t I2C_WaitAck(void)
{
	SCL_LOW;
	SDA_HIGH; /* 释放 SDA，交给从机 */
	I2C_Delay;

	SCL_HIGH;
	I2C_Delay;

	uint8_t ack = READ_SDA ? NACK : ACK;

	SCL_LOW;
	I2C_Delay;

	return ack;
}

/* 先保证 SCL 低，再改 SDA，再拉高 SCL 采样 */
void I2C_SendByte(uint8_t byte)
{
	for (uint8_t i = 0; i < 8; i++)
	{
		SCL_LOW;
		I2C_Delay;

		if (byte & 0x80)
			SDA_HIGH;
		else
			SDA_LOW;
		I2C_Delay;

		SCL_HIGH; /* 从机采样 */
		I2C_Delay;

		SCL_LOW;
		I2C_Delay;

		byte <<= 1;
	}
	SDA_HIGH; /* 发完释放，准备收 ACK */
}

uint8_t I2C_ReadByte(void)
{
	uint8_t byte = 0;

	SDA_HIGH; /* 释放 SDA，由从机驱动 */

	for (uint8_t i = 0; i < 8; i++)
	{
		SCL_LOW;
		I2C_Delay;

		SCL_HIGH;
		I2C_Delay;

		byte <<= 1;
		if (READ_SDA)
			byte |= 0x01;

		SCL_LOW;
		I2C_Delay;
	}

	return byte;
}
