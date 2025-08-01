#include "stm32f10x.h"
#include "fs800.h"
void PWRInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); // 使能GPIOE时钟
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;			  // D8-->PE.0 端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	  // 推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	  // IO口速度为50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}
