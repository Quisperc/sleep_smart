#include "stm32f10x.h"
#include "NVIC.h"
void NVICInit(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;					  // 中断配置结构体
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;			  // TIM3 中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // 先占优先级 0 级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		  // 从优先级 3 级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQ 通道被使能
	NVIC_Init(&NVIC_InitStructure);							  // ④初始化 NVIC 寄存器
}
