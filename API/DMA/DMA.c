#include "DMA.h"
//初始化
void DMA1_Init(void)
{
	//1.开启时钟
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	
	//2.DMA相关配置
	//2.1数据传输方向:存储器到外设
	DMA1_Channel4->CCR |= DMA_CCR4_DIR;
	
	//2.2数据宽度: 8位 -00
	DMA1_Channel4->CCR &= ~DMA_CCR4_PSIZE; //外设数据宽度
	DMA1_Channel4->CCR &= ~DMA_CCR4_MSIZE;	//存储器数据宽度
	
	//2.3 地址自增: 开启自增     //串口地址不自增
	DMA1_Channel4->CCR &= ~DMA_CCR4_PINC;
	DMA1_Channel4->CCR |= DMA_CCR4_MINC;
	
	//2.4 开启数据传输完成中断标志
	DMA1_Channel4->CCR |= DMA_CCR4_TCIE;
	
	
	//2.5 使能串口的DMA传输 使能发送
	USART1->CR3 = USART_CR3_DMAT;
	
	//2.6 开启循环传输模式
	//DMA1_Channel4->CCR |= DMA_CCR1_CIRC;
	
	//3. NCIC配置
	NVIC_SetPriority(DMA1_Channel4_IRQn , 2);
	NVIC_EnableIRQ(DMA1_Channel4_IRQn);
	
}

//数据传输
void DMA1_Transmit(u32 srcAddr , u32 destAddr , u16 datalen)
{
	//1.外设地址 , 也就是源地址
	DMA1_Channel4->CPAR = destAddr;
	
	//2.存储器地址
	DMA1_Channel4->CMAR = srcAddr;
	
	//3.设置传输的数据量
	DMA1_Channel4->CNDTR = datalen;
	
	//4.开启通道,开始传输数据
	DMA1_Channel4->CCR |= DMA_CCR4_EN;
}
void DMA1_Channel4_IRQHandler(void)
{
	//判断中断标志位
	if(DMA1->ISR & DMA_ISR_TCIF4)
	{
		//清除中断标志位    CGIF为中断标志清除寄存器
		DMA1->IFCR |= DMA_IFCR_CGIF4;
		
		//关闭DMA通道
		DMA1_Channel4->CCR &= ~DMA_CCR4_EN;
	}
}

