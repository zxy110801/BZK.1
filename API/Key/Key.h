#ifndef __KEY_H__
#define __KEY_H__
#include "stm32f10x.h"

/*
 * ========== 按键模块思路（讲解用）==========
 * 1. 不用外部中断(EXTI)，由 SysTick 中断每 1ms 调用一次 Key_Scan()。
 *    好处：节拍固定、不依赖主循环快慢；中断里只做短扫描，业务仍在主循环读标志。
 * 2. 每个按键是一台小“状态机”，用 key_jude 记住当前走到哪一步。
 * 3. 消抖：电平要连续稳定 KEY_DEBOUNCE_MS 才算真按下/真松开。
 * 4. 单击 / 长按：
 *    - 按下后未满 KEY_LONG_MS 就松开 → 置 key_dan_flg（单击）
 *    - 按住超过 KEY_LONG_MS 再松开 → 置 key_long_flg（长按）
 * 5. 驱动只负责“识别事件并置标志”；真正干什么（亮灯等）交给主循环读标志。
 * ==========================================
 */

/* 单个按键的全部运行时信息 */
typedef struct
{
	uint8_t key_jude;			   /* 状态机当前状态，见下方注释 */
	uint16_t key_time;			   /* 已确认按下后累计的时间(ms)，用来判断长按 */
	uint8_t db_cnt;				   /* 消抖计数器：连续多少次扫描电平都稳定 */
	uint8_t key_start;			   /* 本轮读到的电平：0=按下，1=松开（低电平有效） */
	volatile uint8_t key_dan_flg;  /* 单击事件标志：1=发生了单击，业务读完后务必清 0 */
	volatile uint8_t key_long_flg; /* 长按事件标志：1=发生了长按，业务读完后务必清 0 */
} keys_t;

/*
 * key_jude 状态含义（讲状态机时按这个顺序画图即可）：
 *   0  空闲         —— 等待按下
 *   1  按下消抖     —— 刚看到低电平，数满 20ms 才算真按下
 *   2  已确认按下   —— 累计 key_time；松开则去 3，超时则去 4
 *   3  单击松手消抖 —— 确认松开后置 key_dan_flg，回 0
 *   4  已达长按     —— 等用户松手
 *   5  长按松手消抖 —— 确认松开后置 key_long_flg，回 0
 */

extern keys_t key[2]; /* key[0] ? PD2，key[1] ? PD3 */

void Key_Init(void); /* 初始化按键 GPIO */
void Key_Scan(void); /* 扫描一次（要求约每 1ms 调用 1 次） */

#endif
