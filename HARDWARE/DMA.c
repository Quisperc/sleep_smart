
#include "DMA.h"
#include "math.h"


// DMA相关变量定义
uint32_t ADC_value[2];		   // ADC数据缓冲区
float data_sample[SLIDE_STEP]; // 滑动数据缓冲区
extern uint8_t timecount;
float adc_data[ADC_NUM] = {0.0}; // ADC数据缓冲区
uint16_t index1 = 0;			 // 数据索引
int slide_ready = 0;			 // 达到滑动处理条件标志
// uint8_t do_flag = 0;		// 数据传输完成标志
uint16_t slide_counter = 0; // 滑动计数器

void DMAx_Mode_Config(void)
{
	DMA_InitTypeDef DMA_InitStruct;											 // DMA结构体
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);						 // 使能DMA1时钟
	DMA_DeInit(DMA1_Channel1);												 // 复位DMA1通道1
	DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)(&(ADC1->DR));		 // 外设地址
	DMA_InitStruct.DMA_MemoryBaseAddr = ((uint32_t)&ADC_value);				 // 内存地址
	DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;							 // 外设到内存
	DMA_InitStruct.DMA_BufferSize = 0x02;									 // 缓冲区大小
	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;			 // 外设地址自增
	DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;					 // 内存地址自增
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; // 外设数据大小
	DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;		 // 内存数据大小
	DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;							 // 循环模式，自动重新开始
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
	if (DMA_GetITStatus(DMA1_IT_TC1) != RESET)
	{
		DMA_ClearITPendingBit(DMA1_IT_TC1);
		// 采样数据转换为电压值
		float sample = (float)ADC_value[0] * 0.806f;
		data_sample[slide_counter++] = sample;
		// 初始采样阶段，满1200个后开始滑动处理
		if (slide_counter % 40 == 0)
			timecount++;
		if (index1 < ADC_NUM)
			index1++;
		if (index1 >= ADC_NUM && slide_counter >= SLIDE_STEP)
		{
			slide_ready = 1;
			slide_counter = 0;
		}
	}
}
