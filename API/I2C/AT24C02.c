#include "AT24C02.h"

void AT24C02_Init(void)
{
	I2C1_Init();
}


void AT24C02_WriteByte(uint8_t innerAddr, uint8_t byte)
{
	I2C_Start();
	I2C_SendByte(W_ADDR);
	if (I2C_WaitAck() != ACK)
	{
		I2C_Stop();
		return;
	}

	I2C_SendByte(innerAddr);
	if (I2C_WaitAck() != ACK)
	{
		I2C_Stop();
		return;
	}

	I2C_SendByte(byte);
	if (I2C_WaitAck() != ACK)
	{
		I2C_Stop();
		return;
	}

	I2C_Stop();
	Delay_ms(5); /* 等待内部写周期 */
}

uint8_t AT24C02_ReadByte(uint8_t innerAddr)
{
	uint8_t byte = 0;

	/* 先写：指定要读的内部地址 */
	I2C_Start();
	I2C_SendByte(W_ADDR);
	if (I2C_WaitAck() != ACK)
	{
		I2C_Stop();
		return 0xFF;
	}

	I2C_SendByte(innerAddr);
	if (I2C_WaitAck() != ACK)
	{
		I2C_Stop();
		return 0xFF;
	}

	/* 再开始：切换为读 */
	I2C_Start();
	I2C_SendByte(R_ADDR);
	if (I2C_WaitAck() != ACK)
	{
		I2C_Stop();
		return 0xFF;
	}

	byte = I2C_ReadByte();
	I2C_NACK();
	I2C_Stop();
	return byte;
}

void AT24C02_WriteBytes(uint8_t innerAddr, uint8_t *bytes, uint8_t size)
{
	/* AT24C02 页大小 8 字节，跨页会回卷，这里限制单次不超过页剩余空间 */
	uint8_t page_left = 8 - (innerAddr % 8);
	if (size > page_left)
		size = page_left;

	I2C_Start();
	I2C_SendByte(W_ADDR);
	if (I2C_WaitAck() != ACK)
	{
		I2C_Stop();
		return;
	}

	I2C_SendByte(innerAddr);
	if (I2C_WaitAck() != ACK)
	{
		I2C_Stop();
		return;
	}

	for (uint8_t i = 0; i < size; i++)
	{
		I2C_SendByte(bytes[i]);
		if (I2C_WaitAck() != ACK)
		{
			I2C_Stop();
			return;
		}
	}

	I2C_Stop();
	Delay_ms(5);
}

void AT24C02_ReadBytes(uint8_t innerAddr, uint8_t *buffer, uint8_t size)
{
	I2C_Start();
	I2C_SendByte(W_ADDR);
	if (I2C_WaitAck() != ACK)
	{
		I2C_Stop();
		return;
	}

	I2C_SendByte(innerAddr);
	if (I2C_WaitAck() != ACK)
	{
		I2C_Stop();
		return;
	}

	I2C_Start();
	I2C_SendByte(R_ADDR);
	if (I2C_WaitAck() != ACK)
	{
		I2C_Stop();
		return;
	}

	for (uint8_t i = 0; i < size; i++)
	{
		buffer[i] = I2C_ReadByte();
		if (i == size - 1)
			I2C_NACK();
		else
			I2C_ACK();
	}

	I2C_Stop();
}
