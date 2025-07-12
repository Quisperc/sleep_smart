#include "stm32f10x.h"
#include "TIM3.h"
void TIM3Init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;				// 定义TIM3初始化结构体
	NVIC_InitTypeDef NVIC_InitStructure;						// 中断配置结构体
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);		// 使能TIM3时钟
	TIM_TimeBaseStructure.TIM_Period = 249;						// 自动重装值
	TIM_TimeBaseStructure.TIM_Prescaler = 7199;					// 预分频
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		// 不分频
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // TIM向上计数
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);				// 初始化TIM3

	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;			  // TIM3 中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // 先占优先级 0 级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		  // 从优先级 3 级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQ 通道被使能
	NVIC_Init(&NVIC_InitStructure);							  // ④初始化 NVIC 寄存器

	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE); // 使能TIM3更新中断

	TIM_Cmd(TIM3, ENABLE); // 使能TIM3
}

void TIM3_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) // 检查 TIM3 更新中断发生与否
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update); // 清除 TIM3 更新中断标志
		;
	}
}
