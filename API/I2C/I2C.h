#ifndef __I2C_H__
#define __I2C_H__
#include "stm32f10x.h" // Device header
#include "Delay.h"
// SDA PD4    SCL PD5

#define ACK 0
#define NACK 1

// 控制输出电平拉低拉高
#define SDA_HIGH (GPIOD->ODR |= GPIO_Pin_4)
#define SDA_LOW (GPIOD->ODR &= ~GPIO_Pin_4)
#define SCL_HIGH (GPIOD->ODR |= GPIO_Pin_5)
#define SCL_LOW (GPIOD->ODR &= ~GPIO_Pin_5)
// 读取引脚电平,接收SDA信息
#define READ_SDA (GPIOD->IDR & GPIO_Pin_4)

// 定义操作的基本延迟
#define I2C_Delay Delay_us(10)

// 初始化
void I2C1_Init(void);
// 发出起始信号
void I2C_Start(void);
// 发出停止信号
void I2C_Stop(void);
// 发出应答信号
void I2C_ACK(void);
// 主机发送非应答信号
void I2C_NACK(void);
// 主机等待从设备发来应答信号
uint8_t I2C_WaitAck(void);
// 主机发送一个字节的数据
void I2C_SendByte(uint8_t byte);
// 主机从EEProm接收一个字节的数据(读取)
uint8_t I2C_ReadByte(void);

#endif

/*
	I2C协议的知识点总结:
	I2C传输信息的基础:数据线外接上拉电阻,将电平拉高,为弱上拉 电平概念为"1",在没有信息的时间,信号线为高
	I2C开始标识为SDA由高转低
	I2C在时钟线SCL为高电平期间,读取SDA上的数据,在SCL为低电平时间,交换SDA上的数据
	I2C的传输的速率分为 100kb/s 和 快速400kb/s
	100kb/s情况延时10us

	地址为7位地址,后接一位读写命令位,当发送一字节的数据,会有一个应答位
	当收到的应答为低时,继续接收
	应答为高,表示接收方未确认,

*/
