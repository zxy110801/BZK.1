/*
 * Key.c —— PD2 / PD3 按键驱动
 *
 * 【整体流程】  
 *   SysTick_Handler 每 1ms → 调一次 Key_Scan()
 *   Key_Scan 更新 key[] 状态机，必要时置位单击/长按标志
 *   业务代码：if (key[0].key_dan_flg) { ...; key[0].key_dan_flg = 0; }
 *
 * 【为什么 1ms 扫一次】
 *   消抖、长按计时都以“扫描次数 ≈ 毫秒”来算，周期稳定最简单。
 */
#include "Key.h"

keys_t key[2] = {0}; /* 两个按键各自一套状态，互不影响 */

#define KEY_DEBOUNCE_MS 20 /* 消抖门限：连续稳定约 20ms */
#define KEY_LONG_MS 2000   /* 长按门限：有效按下满约 2000ms */

/* 引脚：浮空输入，按下为低电平；此处不用 EXTI */
void Key_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
}

/*
 * Key_Scan：一次扫描走完“所有按键”各一步状态迁移
 * 注意：本函数内部不要 Delay，保持非阻塞，否则会拖死主循环。
 */
void Key_Scan(void)
{
	/* 先采电平，再统一跑状态机（本周期内电平快照一致） */
	key[0].key_start = GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_2);
	key[1].key_start = GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_3);

	for (uint8_t i = 0; i < 2; i++)
	{
		switch (key[i].key_jude)
		{
		case 0: /* ---- 空闲：等按下 ---- */
			if (key[i].key_start == 0)
			{
				key[i].key_jude = 1; /* 疑似按下，进入消抖 */
				key[i].db_cnt = 0;
			}
			break;

		case 1: /* ---- 按下消抖：滤掉机械抖动 ---- */
			if (key[i].key_start == 0)
			{
				key[i].db_cnt++;
				if (key[i].db_cnt >= KEY_DEBOUNCE_MS)
				{
					/* 连续低电平够久 → 确认按下 */
					key[i].key_jude = 2;
					key[i].key_time = 0; /* 从此开始算按住多久 */
					key[i].db_cnt = 0;
				}
			}
			else
			{
				/* 消抖期间又变高：当毛刺，回空闲 */
				key[i].key_jude = 0;
			}
			break;

		case 2: /* ---- 已确认按下：计时，区分单击/长按 ---- */
			if (key[i].key_start == 0)
			{
				/* 仍按着：累加有效按下时间 */
				if (key[i].key_time < 0xFFFF)
					key[i].key_time++;
				if (key[i].key_time >= KEY_LONG_MS)
				{
					/* 已够长按时间，先进入“等松手”态（标志等松手后再置） */
					key[i].key_jude = 4;
					key[i].db_cnt = 0;
				}
			}
			else
			{
				/* 未到长按就松开 → 走单击松手消抖 */
				key[i].key_jude = 3;
				key[i].db_cnt = 0;
			}
			break;

		case 3: /* ---- 单击：松开消抖 ---- */
			if (key[i].key_start == 1)
			{
				key[i].db_cnt++;
				if (key[i].db_cnt >= KEY_DEBOUNCE_MS)
				{
					key[i].key_dan_flg = 1; /* 通知上层：发生单击 */
					key[i].key_jude = 0;	/* 回到空闲，等下次 */
				}
			}
			else
			{
				/* 松手消抖时又变低：当抖一下，回到按下态，保留 key_time */
				key[i].key_jude = 2;
				key[i].db_cnt = 0;
			}
			break;

		case 4: /* ---- 已达长按：只等用户松手 ---- */
			if (key[i].key_start == 1)
			{
				key[i].key_jude = 5; /* 开始松手消抖 */
				key[i].db_cnt = 0;
			}
			break;

		case 5: /* ---- 长按：松开消抖 ---- */
			if (key[i].key_start == 1)
			{
				key[i].db_cnt++;
				if (key[i].db_cnt >= KEY_DEBOUNCE_MS)
				{
					key[i].key_long_flg = 1; /* 通知上层：发生长按 */
					key[i].key_jude = 0;
				}
			}
			else
			{
				/* 消抖中又按下：回到等松手 */
				key[i].key_jude = 4;
				key[i].db_cnt = 0;
			}
			break;

		default:
			key[i].key_jude = 0;
			break;
		}
	}
}
