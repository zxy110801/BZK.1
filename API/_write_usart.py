# -*- coding: utf-8 -*-
content = r'''#include "USART.h"

// USART1 库函数初始化：PA9=TX，PA10=RX，115200，接收中断
void Myusart_Init(void)
{
	// PA9 复用推挽输出（TX）
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	// PA10 浮空输入（RX）
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART1,&USART_InitStructure);
	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); // 接收非空中断
	
	USART_Cmd(USART1,ENABLE);
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_Init(&NVIC_InitStructure);
}


// 寄存器版初始化（开时钟、配 GPIO、波特率、收发与 RXNE、NVIC）
void MYUSART_Init(void)
{
	RCC->APB2ENR |= 1<<2;   // GPIOA
	RCC->APB2ENR |= 1<<14;  // USART1
	
	GPIOA->CRH &= ~(0x0F<<4);  // PA9 复用推挽输出
	GPIOA->CRH |= 0x0B<<4;
	
	GPIOA->CRH &= ~(0x0F<<8);  // PA10 浮空输入
	GPIOA->CRH |= 0x04<<8;  
	
	USART1->BRR = 0x271;    // 波特率 115200（72MHz 下约 39.0625）
	
	USART1->CR1 &= ~(0xFFFF<<0);
	USART1->CR1 |= 0x202C << 0; // UE/TE/RE/RXNEIE 等
	
	USART1->CR1 &= ~(1<<12);      // 8 数据位
	USART1->CR2 &= ~(0x03<<12);   // 1 停止位
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_Init(&NVIC_InitStructure);
}


/*
 * 协议帧缓存（书写时用空格间隔，线上是连续二进制字节）：
 * Port[0]     = 0xAA 帧头
 * Port[1]     = 命令
 * Port[2]     = 数据域长度 N
 * Port[3..2+N]= 数据
 * Port[3+N]   = 校验（命令+长度+数据 的低8位和）
 * 下一字节    = 0xBB 帧尾
 * 示例：AA 01 02 01 01 05 BB  -> 开 LED1（01+02+01+01=05）
 */
uint8_t Port[10];
uint8_t i = 0;            // 当前写入下标

uint8_t usart1_data = 0;  // 本次中断读到的字节
uint8_t USART1_Flag = 0;  // 0=等帧头，1=正在收帧

uint8_t SUM_flag = 0;     // 运行中校验累加和

extern uint8_t USART_User_Flag; // 主循环里置位后调用 USART_User
extern uint8_t LED_Show_Flag;

// 按灯编号执行控制命令：led_id 0/1/2=LED1/2/3，3=全部
static void LED_Cmd(uint8_t led_id, uint8_t cmd)
{
	switch(cmd)
	{
		case 0x01: LED_ON(led_id); break;          // 开
		case 0x02: LED_OFF(led_id); break;         // 关
		case 0x03: LED_Show_Flag = 1; break;       // 开流水灯
		case 0x04: LED_Show_Flag = 0; break;       // 关流水灯
		case 0x05: break; // 闪烁（待实现）
		case 0x06: break; // 快闪（待实现）
		case 0x07: break; // 慢闪（待实现）
		default: break;
	}
}

// 根据已收完整帧执行命令（在主循环中调用）
void USART_User(void)
{
	switch(Port[1]) // 命令字
	{
		case 0x01:{ // 控制 LED
			switch(Port[2]) // 数据域长度
			{
				case 0x02:{ // 2 字节：灯编号 + 控制命令
					// Port[3]: 0x00全部 0x01 LED1 0x02 LED2 0x03 LED3
					// Port[4]: 控制命令
					switch(Port[3])
					{
						case 0x00: LED_Cmd(3, Port[4]); break;
						case 0x01: LED_Cmd(0, Port[4]); break; // LED1 -> LED_ON(0)
						case 0x02: LED_Cmd(1, Port[4]); break; // LED2
						case 0x03: LED_Cmd(2, Port[4]); break; // LED3
						default: break;
					}
				}break;
				case 0x03:{ // 3 字节：编号 + 命令 + 参数（暂未实现）
					
				}break;
			}
		}break;
		case 0x02:{ // 控制蜂鸣器
			
		}break;
		case 0x03:{ // 读取 LED 状态
			
		}break;
	}
}

/*
 * USART1 接收中断：按长度拆帧并校验
 * 帧格式（空格仅便于书写）: AA 命令 长度N 数据[N] 校验 BB
 * 校验 = 命令 + 长度 + 数据 的低8位累加和（不含 AA 校验 BB）
 * 例：AA 01 02 01 01 05 BB ，其中 01+02+01+01=05
 */
void USART1_IRQHandler(void)
{
	uint8_t data_len;   // 数据域长度 Port[2]
	uint8_t sum_index;  // 校验字节下标 = 3 + N
	uint8_t end_index;  // 帧尾下标     = 4 + N

	usart1_data = (uint8_t)USART1->DR; // 读 DR 清 RXNE

	// 未收帧：只认帧头 0xAA
	if(USART1_Flag == 0)
	{
		if(usart1_data == 0xAA)
		{
			USART1_Flag = 1;
			i = 0;
			SUM_flag = 0;
			Port[i++] = usart1_data;
		}
		return; // 帧头处理完立刻返回，避免同一字节再走后面逻辑
	}

	// 缓冲区溢出，丢弃本帧
	if(i >= 10)
	{
		USART1_Flag = 0;
		i = 0;
		SUM_flag = 0;
		return;
	}

	Port[i] = usart1_data;

	// Port[1]：命令，计入校验
	if(i == 1)
	{
		SUM_flag += usart1_data;
		i++;
		return;
	}

	// Port[2]：长度，计入校验
	if(i == 2)
	{
		data_len = usart1_data;
		// 整帧 = AA CMD LEN DATA SUM BB，Port[10] 下 DATA 最多 5 字节
		if(data_len > 5)
		{
			USART1_Flag = 0;
			i = 0;
			SUM_flag = 0;
			return;
		}
		SUM_flag += usart1_data;
		i++;
		return;
	}

	// 已知长度后，算出校验位、帧尾下标
	data_len = Port[2];
	sum_index = (uint8_t)(3 + data_len);
	end_index = (uint8_t)(4 + data_len);

	// 数据域：计入校验
	if(i < sum_index)
	{
		SUM_flag += usart1_data;
		i++;
		return;
	}

	// 校验字节：只保存，不累加
	if(i == sum_index)
	{
		i++;
		return;
	}

	// 帧尾 0xBB：比对校验，成功则通知主循环
	if(i == end_index)
	{
		if(usart1_data == 0xBB && SUM_flag == Port[sum_index])
		{
			USART_User_Flag = 1;
		}
		// 无论对错都复位，准备下一帧
		USART1_Flag = 0;
		i = 0;
		SUM_flag = 0;
		return;
	}

	// 异常路径：状态错乱则复位
	USART1_Flag = 0;
	i = 0;
	SUM_flag = 0;
}




// 阻塞发送 1 字节（等 TXE）
void USART1_Send(uint8_t data)
{
	while((USART1->SR & (1<<7)) == 0);
	USART1->DR = data;   
}
'''

path = r'E:/Desktop/Project/bzk.EXTI/API/USART.c'
# Keil 中文工程常用 GBK
open(path, 'w', encoding='gbk', newline='\r\n').write(content)
print('written gbk ok')
