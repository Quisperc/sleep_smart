#include "stm32f10x.h"
#include "DMA.h"

// DMA相关变量定义
uint16_t ADC_Data1[2]; // ADC数据缓冲区
uint16_t p1[ADC_NUM];  // 温度数据缓冲区
uint16_t index1 = 0;   // 数据索引
uint8_t do_flag = 0;   // 数据传输完成标志

void DMAx_Mode_Config(void)
{
	DMA_InitTypeDef DMA_InitStruct;											 // DMA结构体
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);						 // 使能DMA1时钟
	DMA_DeInit(DMA1_Channel1);												 // 复位DMA1通道1
	DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)(&(ADC1->DR));		 // 外设地址
	DMA_InitStruct.DMA_MemoryBaseAddr = ((uint32_t)ADC_Data1);				 // 内存地址
	DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;							 // 外设到内存
	DMA_InitStruct.DMA_BufferSize = 0x02;									 // 缓冲区大小
	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;			 // 外设地址自增
	DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;					 // 内存地址自增
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; // 外设数据大小
	DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;		 // 内存数据大小
	DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;							 // 循环模式
	DMA_InitStruct.DMA_Priority = DMA_Priority_High;						 // 优先级
	DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;								 // 内存到内存
	DMA_Init(DMA1_Channel1, &DMA_InitStruct);								 // 初始化
	DMA_Cmd(DMA1_Channel1, ENABLE);											 // 使能DMA1通道1
}

void DMAx_NVIC_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStruct;					   // NVIC结构体
	NVIC_InitStruct.NVIC_IRQChannel = DMA1_Channel1_IRQn;  // DMA1通道1中断
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1; // 抢占优先级
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;		   // 子优先级
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;		   // 中断使能
	NVIC_Init(&NVIC_InitStruct);						   // 初始化
	DMA_ClearITPendingBit(DMA1_IT_TC1);					   // 清除中断标志
	DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);		   // 使能DMA1通道1传输完成中断
}

void DMA1_Channel1_IRQHandler(void)
{
	// uint8_t i; // 循环变量
	// uint8_t char_dat[6]; // 数据缓冲区
	// uint16_t temp; // 临时变量
	if (DMA_GetITStatus(DMA1_IT_TC1) != RESET) // 传输完成中断
	{
		DMA_ClearITPendingBit(DMA1_IT_TC1); // 清除中断标志
		p1[index1] = ADC_Data1[0] * 0.806;	// 转换成温度值
		index1++;							// 数据指针增加
		if (index1 >= ADC_NUM)				// 数据指针超出范围
		{
			do_flag = 1; // 数据传输完成
		}
	}
}
