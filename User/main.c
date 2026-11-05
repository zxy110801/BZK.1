#include "stm32f10x.h"
#include "Delay.h"
#include "LED.h"
#include "Key.h"
#include "stdio.h"
#include "USART.h"
#include "Systick.h"
#include "Beep.h"
#include "tim.h"
#include "ULTRASONIC.h"
#include "PWM.h"
#include "TIM8_SR04.h"
#include "ADC.h"
#include "DMA.h"
#include "lcd.h"
#include "lcd_init.h"
#include "pic.h"
#include "AT24C02.h"
u8 USART_User_Flag = 0;
u8 ledAll_flag = 0;
u8 led_flag[3] = {0};
double chao_Length = 0;

u8 lcd_jude = 0;

// 定义数组,保存转换后的信号值
u16 data[2] = {0};
// 定义变量数组,放置在RAM中,用来存储接收到的数据
u8 Src[4] = {'a', 'b', 'c', 'd'};

/* 主函数:外设初始化与主循环 */
int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	Key_Init();
	LED_Init();
	BZKusart_Init();
	Systick_Init();
	DMA1_Init();

	LCD_Init();
	LCD_Fill(0, 0, LCD_W, LCD_H, LGRAYBLUE);
	ADC1_DMA_Init();

	AT24C02_Init();
	printf("初始化完成!!!\n");

	// 依次向EEPROM上写入单个字符
	AT24C02_WriteByte(0x00, 'a');
	AT24C02_WriteByte(0x01, 'b');
	AT24C02_WriteByte(0x02, 'c');

	uint8_t byte1 = AT24C02_ReadByte(0x00);
	uint8_t byte2 = AT24C02_ReadByte(0x01);
	uint8_t byte3 = AT24C02_ReadByte(0x02);

	/* 期望: 0x32 0x62 0x63  即 50 98 99 */
	printf("byte1=0x%02X(%c) byte2=0x%02X(%c) byte3=0x%02X(%c)\n",
		   byte1, byte1, byte2, byte2, byte3, byte3);

	/* PC0=CH10 -> data[0], PA1=CH1 -> data[1] */
	ADC1_DMA_StartConvert((u32)data, 2);

	while (1)
	{
		if (key[0].key_dan_flg == 1)
		{
			key[0].key_dan_flg = 0;
			lcd_jude++;
			lcd_jude = lcd_jude % 2;
			LCD_Fill(0, 0, LCD_W, LCD_H, LGRAYBLUE);
		}
		switch (lcd_jude)
		{
		case 0:
		{

			LCD_ShowString(10, 10, (const u8 *)"ADC DMA", RED, LGRAYBLUE, 24, 0);
			LCD_ShowString(10, 50, (const u8 *)"V1:", BLACK, LGRAYBLUE, 24, 0);
			LCD_ShowString(10, 90, (const u8 *)"V2:", BLACK, LGRAYBLUE, 24, 0);
			LCD_ShowString(10, 140, (const u8 *)"RAW1:", BLACK, LGRAYBLUE, 16, 0);
			LCD_ShowString(100, 140, (const u8 *)"RAW2:", BLACK, LGRAYBLUE, 16, 0);
			float v1 = data[0] * 3.3f / 4095.0f;
			float v2 = data[1] * 3.3f / 4095.0f;

			//printf("V1:%.2fV V2:%.2fV\n", v1, v2);

			/* 电压：两位小数，len=3 对应 x.xx */
			LCD_ShowFloatNum1(60, 50, v1, 3, RED, LGRAYBLUE, 24);
			LCD_ShowString(60 + 4 * 12, 50, (const u8 *)"V", RED, LGRAYBLUE, 24, 0);
			LCD_ShowFloatNum1(60, 90, v2, 3, RED, LGRAYBLUE, 24);
			LCD_ShowString(60 + 4 * 12, 90, (const u8 *)"V", RED, LGRAYBLUE, 24, 0);

			/* 原始 ADC 码值 0~4095 */
			LCD_ShowIntNum(60, 140, data[0], 4, BLUE, LGRAYBLUE, 16);
			LCD_ShowIntNum(140, 140, data[1], 4, BLUE, LGRAYBLUE, 16);
		}
		break;
		case 1:
		{

			//LCD_ShowPicture(100, 40, 100, 98, gImage_1);
			//LCD_ShowPicture(100, 40, 98, 100, gImage_2);
		}
		break;
		}

		Delay_ms(10);
	}
}
