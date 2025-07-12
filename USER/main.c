#include "stm32f10x.h"
#include "fs800.h"
#include "AD.h"
#include "delay.h"
#include "TIM3.h"
#include "usart.h"
#include "DMA.h"



void Delay(u32 count)
{
	u32 i = 0;
	for (; i < count; i++);
}

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 分配中断分组
	delay_init();									// 延时函数初始化
	PWRInit();										// 初始化电源管理
	ADCIOInit();									// ADC初始化
	TIM3Init();										// TIM3中断初始化
	uart_init(115200);
	uart2_init(115200); // 串口初始化

	while (1)
	{
		GPIO_SetBits(GPIOA, GPIO_Pin_8);
		delay_ms(200);
		GPIO_ResetBits(GPIOA, GPIO_Pin_8);
	}
}
