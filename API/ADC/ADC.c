#include "ADC.h"
/*
	初始化一个单次连续转换的ADC通道采集
	开启PC0 对应的是ADC1 的通道10
*/
void ADC1_Init(void)
{
	//1.时钟配置
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
	//CFGR:ADCPRE - 10  ,6分频,得到12MHZ
	RCC->CFGR &= ~RCC_CFGR_ADCPRE;
	RCC->CFGR |= RCC_CFGR_ADCPRE_DIV6;
	
	//2.GPIO工作模式,PC0 模拟输入
	GPIOC->CRL &= ~(GPIO_CRL_MODE0 | GPIO_CRL_CNF0);
	
	//3.ADC配置
	//3.1 工作模式:
	//关闭扫描模式  ADC_CR1_SCAN     0 关闭 1 开启
	ADC1->CR1 &= ~ADC_CR1_SCAN;  
	
	//3.2 开启连续转换模式 ADC_CR2_CONT  0 关闭 1 开启 
	ADC1->CR2 |= ADC_CR2_CONT;
	
	//3.3 转换结果数据对齐模式 ADC_CR2_ALIGN	 0 左对齐 1右对齐
	ADC1->CR2 &= ~ADC_CR2_ALIGN;
		
	//3.4 设置通道10的采样时间 ADC采样时间寄存器 ADC_SMPR1       
	//选择通道x的采样时间,两个寄存器18个通道各自的采样时间,SMPR2为低9个 
	//加上ADC转换时间,时钟周期为 7.5 + 12.5 = 20
	ADC1->SMPR1 &= ~ADC_SMPR1_SMP10;    	// 如果单单清零的话,采样周期为1.5个周期
	ADC1->SMPR1 |= ADC_SMPR1_SMP10_0; 		//采样周期为7.5个周期   ,001  
//	ADC1->SMPR1 |= ADC_SMPR1_SMP10_1;     作为演示,这个采样周期为13.5采样周期 ,为010
	
	//3.5 规则组通道序列配置
	//3.5.1 规则组中的通道个数L
	//3.5.2 第一个通道是哪个通道
	/*
		ADC规则序列寄存器 ADC_SQR  用来存储通道位次   
		SQR1 L代表多少个ADC通道,共16个通道,次序为SQ1->SQ2->...->SQ16
		SQR3低5位为第一个转换,5位数据值为ADCx的第几个通道
	*/	
	ADC1->SQR1 &= ~ADC_SQR1_L;     //选择一个通道
	ADC1->SQR3 &= ~ADC_SQR3_SQ1;
	ADC1->SQR3 |= (10 << 0);			//这一个通道为哪个通道
	
	
	
	//3.6 选择软件触发AD转换
	ADC1->CR2 |= ADC_CR2_EXTTRIG;   //开启外部事件
	ADC1->CR2 |= ADC_CR2_EXTSEL;    //选择的就是SWSTART控制AD转换
	
}

void ADC1_StartConvert(void)
{
	
	/*
		ADC控制寄存器 ADC_CR2 
		关键点: 当ADC_CR2_ADON为0时,将该位置1将从断电模式中唤醒
		当ADC_CR2_ADON为1时,再置1为启动转换,但只能开启规则组的ADC转换
	*/
	//1. 上电唤醒
	//ADC_CR2_ADON为开关AD转换器 0关闭 1开启
	ADC1->CR2 |= ADC_CR2_ADON;    //开启AD转换器,软件开启,上电唤醒
	
	//2. 执行校准
	//ADC_CR2_CAL为AD校准  0是校准完成 1为开始校准
	ADC1->CR2 |= ADC_CR2_CAL;   //开启校准
	//等待校准完成
	while(ADC1->CR2 & ADC_CR2_CAL);
	
	//3. 启动转换
	ADC1->CR2 |= ADC_CR2_SWSTART;
	// 启动转换,可以在初始化阶段不选择触发方式,只能用来触发规则组的AD转换
	//ADC1->CR2 |= ADC_CR2_ADON;  
	
	//4. 等待转换完成
	while((ADC1->SR & ADC_SR_EOC) == 0);
	
	
}
//返回ADC转换完的值
double ADC1_ReadV(void)
{
	return ADC1->DR * 3.30 / 4095;
}



void ADC1_DMA_Init(void)
{
	//1.时钟配置
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
	//CFGR:ADCPRE - 10  ,6分频,得到12MHZ
	RCC->CFGR &= ~RCC_CFGR_ADCPRE;
	RCC->CFGR |= RCC_CFGR_ADCPRE_DIV6;
	
	//2.GPIO工作模式,PC0 模拟输入
	GPIOC->CRL &= ~(GPIO_CRL_MODE0 | GPIO_CRL_CNF0);
	//GPIO工作模式,PA1 模拟输入
	GPIOA->CRL &= ~(GPIO_CRL_MODE1 | GPIO_CRL_CNF1);
	
	//3.ADC配置
	//3.1 工作模式:
	//关闭扫描模式  ADC_CR1_SCAN     0 关闭 1 开启
	ADC1->CR1 |= ADC_CR1_SCAN;  
	
	//3.2 开启连续转换模式 ADC_CR2_CONT  0 关闭 1 开启 
	ADC1->CR2 |= ADC_CR2_CONT;
	
	//3.3 转换结果数据对齐模式 ADC_CR2_ALIGN	 0 右对齐 1 左对齐
	ADC1->CR2 &= ~ADC_CR2_ALIGN;
	
	//3.4 设置通道10的采样时间 ADC采样时间寄存器 ADC_SMPR1       
	//选择通道x的采样时间,两个寄存器18个通道各自的采样时间,SMPR2为低9个 
	//加上ADC转换时间,时钟周期为 7.5 + 12.5 = 20
	ADC1->SMPR1 &= ~ADC_SMPR1_SMP10;    	// 如果单单清零的话,采样周期为1.5个周期
	ADC1->SMPR1 |= ADC_SMPR1_SMP10_0; 		//采样周期为7.5个周期   ,001  
//	ADC1->SMPR1 |= ADC_SMPR1_SMP10_1;     作为演示,这个采样周期为13.5采样周期 ,为010
	ADC1->SMPR2 &= ~ADC_SMPR2_SMP1;    	// 如果单单清零的话,采样周期为1.5个周期
	ADC1->SMPR2 |= ADC_SMPR2_SMP1_1; 		//采样周期为7.5个周期   ,001  
	
	//3.5 规则组通道序列配置
	//3.5.1 规则组中的通道个数L
	//3.5.2 第一个通道是哪个通道
	/*
		ADC规则序列寄存器 ADC_SQR  用来存储通道位次   
		SQR1 L代表多少个ADC通道,共16个通道,次序为SQ1->SQ2->...->SQ16
		SQR3低5位为第一个转换,5位数据值为ADCx的第几个通道
	*/	
	ADC1->SQR1 &= ~ADC_SQR1_L;       //为清零,当L为0000 的时候为1个通道
	ADC1->SQR1 |= ADC_SQR1_L_0;     //选择2个通道   当L为0001的时候为两个通道
	//规则组通道1清零
	ADC1->SQR3 &= ~ADC_SQR3_SQ1;
	//通道1 为ADC1_IN10
	ADC1->SQR3 |= (10 << 0);			
	//规则组通道2清零
	ADC1->SQR3 &= ~ADC_SQR3_SQ2;
	//通道1 为ADC1_IN1
	ADC1->SQR3 |= (1 << 5);
	
	//3.6 选择软件触发AD转换
	ADC1->CR2 |= ADC_CR2_EXTTRIG;   //开启外部事件
	ADC1->CR2 |= ADC_CR2_EXTSEL;    //选择的就是SWSTART控制AD转换
	
	
	/*       DMA相关初始化    */
	//1.开启DMA模块时钟
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	
	//2.设置通道1的传输方向,DIR = 0
	DMA1_Channel1->CCR &= ~DMA_CCR1_DIR;
	
	//3.数据宽度,16位 SIZE 01
	DMA1_Channel1->CCR &= ~DMA_CCR1_PSIZE_1;
	DMA1_Channel1->CCR |= DMA_CCR1_PSIZE_0;
	DMA1_Channel1->CCR &= ~DMA_CCR1_MSIZE_1;
	DMA1_Channel1->CCR |= DMA_CCR1_MSIZE_0;
	
	//4.地址自增,外设不增,内存地址要增
	DMA1_Channel1->CCR &= ~DMA_CCR1_PINC;
	DMA1_Channel1->CCR |= DMA_CCR1_MINC;
	
	//5.开启循环模式
	DMA1_Channel1->CCR |= DMA_CCR1_CIRC;
	
	//6.开启DMA模式
	ADC1->CR2 |= ADC_CR2_DMA;
}
void ADC1_DMA_StartConvert(u32 destAddr , u8 len)
{
	//0.DMA配置源地址和目的地址,以及数据长度
	//外设数据的地址
	DMA1_Channel1->CPAR = (u32)&(ADC1->DR);
	//内存的地址
	DMA1_Channel1->CMAR = destAddr;
	//数据的长度
	DMA1_Channel1->CNDTR = len;
	
	
	
	/*
		ADC控制寄存器 ADC_CR2 
		关键点: 当ADC_CR2_ADON为0时,将该位置1将从断电模式中唤醒
		当ADC_CR2_ADON为1时,再置1为启动转换,但只能开启规则组的ADC转换
	*/
	//1. 上电唤醒
	//ADC_CR2_ADON为开关AD转换器 0关闭 1开启
	ADC1->CR2 |= ADC_CR2_ADON;    //开启AD转换器,软件开启,上电唤醒
	
	//使能DMA
	DMA1_Channel1->CCR |= DMA_CCR1_EN;
	
	//2. 执行校准
	//ADC_CR2_CAL为AD校准  0是校准完成 1为开始校准
	ADC1->CR2 |= ADC_CR2_CAL;   //开启校准
	//等待校准完成
	while(ADC1->CR2 & ADC_CR2_CAL);
	
	//3. 启动转换
	ADC1->CR2 |= ADC_CR2_SWSTART;
	// 启动转换,可以在初始化阶段不选择触发方式,只能用来触发规则组的AD转换
	//ADC1->CR2 |= ADC_CR2_ADON;  
	
	//4. 等待转换完成
	while((ADC1->SR & ADC_SR_EOC) == 0);
	
}
void ADC1_BZK_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 , ENABLE);
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	
	GPIO_InitTypeDef GPIO_ADC1Init;
	GPIO_ADC1Init.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_ADC1Init.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOC,&GPIO_ADC1Init);
	
	ADC_InitTypeDef ADC1_InitStructure;
	ADC_DeInit(ADC1);
	ADC1_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC1_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC1_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC1_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC1_InitStructure.ADC_NbrOfChannel =  1;
	ADC1_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_Init(ADC1,&ADC1_InitStructure);
	
	ADC_RegularChannelConfig(ADC1,ADC_Channel_10,1,ADC_SampleTime_7Cycles5);
	
	ADC_Cmd(ADC1,ENABLE);
	
	ADC_ResetCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1));
	
	ADC_SoftwareStartConvCmd(ADC1,ENABLE);
	while(ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC) == RESET);
}

double ADC1_Read(void)
{
	return ADC_GetConversionValue(ADC1) / 4095 * 3.3;
}

