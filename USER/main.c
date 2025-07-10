#include "stm32f10x.h"
#include "fs800.h"
#include "AD.h"
#include "delay.h"
void Delay(u32 count)
{
	u32 i = 0;
	for (; i < count; i++)
		;
}

int main(void)
{
	delay_init();
	PWRInit(); // 初始化电源管理
	ADCIOInit();

	while (1)
	{
		GPIO_SetBits(GPIOA, GPIO_Pin_0);
		delay_ms(200);
		GPIO_ResetBits(GPIOA, GPIO_Pin_8);
		GPIO_Init();
	}
}
