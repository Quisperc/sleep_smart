#include "stm32f10x.h"
#include "fs800.h"
#include "AD.h"
#include "delay.h"
#include "TIM3.h"
#include "usart.h"
#include "DMA.h"
#include "Filter.h"
#include "findpeak.h"
#define ADC_NUM 1200   // 窗口大小（30秒 × 40Hz）
#define SLIDE_STEP 120 // 滑动步长（3秒 × 40Hz）

// 全局变量
extern uint16_t slide_counter;				// 滑动计数器（在DMA.c中中断计数）
float data_buffer[ADC_NUM];					// 数据缓冲区
float current_heart_rate = 0;				// 心率
float current_breath_rate = 0;				// 呼吸率
SFindPV stFindPV;							// 波峰波谷
uint8_t count = 0;							// 波峰波谷计数
float SignalFilter_heart[ADC_NUM] = {0.0};	// 滤波后的数据缓冲区
float SignalFilter_breath[ADC_NUM] = {0.0}; // 滤波后的数据缓冲区
extern int slide_ready;						// 滑动处理完成标志

// 函数声明
void Send_Data_To_PC(int current_heart_rate, int current_breath_rate);
void Slide_Window(void);
// 通过串口2发送无效消息
void Send_Invalid_Message(void);

// 延时
void Delay(u32 count)
{
	u32 i = 0;
	for (; i < count; i++)
		;
}

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 分配中断分组
	delay_init();									// 延时函数初始化
	PWRInit();										// 初始化电源管理

	// 初始化串口
	uart_init(115200);
	uart2_init(115200);

	USART_SendString(USART2, "Sliding Window Heart Rate Monitor System\r\n");
	USART_SendString(USART2, "Sampling Rate: 40Hz (25ms interval)\r\n");
	USART_SendString(USART2, "Heart Rate Filter: 0.66Hz-2.5Hz\r\n");
	USART_SendString(USART2, "Breathing Filter: 0.13Hz-0.53Hz\r\n");
	USART_SendString(USART2, "Initial Window: 30 seconds (1200 data points)\r\n");
	USART_SendString(USART2, "Slide Step: 3 seconds (120 data points)\r\n");

	// 初始化ADC相关
	ADCx_GPIO_Config();	 // ADC GPIO配置
	ADC_Configuration(); // ADC配置

	// 初始化DMA
	DMAx_Mode_Config(); // DMA模式配置
	DMAx_NVIC_Config(); // DMA中断配置

	// 初始化定时器（25ms定时）
	TIM3Init();

	USART_SendString(USART2, "System initialization completed\r\n");
	USART_SendString(USART2, "Waiting for initial 30-second data collection...\r\n");

	// 主循环
	while (1)
	{
		if (slide_ready)
		{
			// 拷贝当前窗口数据
			// GetWindow(data_buffer, ADC_NUM);

			// 计算心率
			Heart_filter(data_buffer, SignalFilter_heart);
			initialFindPV(&stFindPV);
			FindPV(&stFindPV, SignalFilter_heart);
			current_heart_rate = get_heart(&stFindPV, SignalFilter_heart, &count);

			// 计算呼吸率
			Breath_filter(data_buffer, SignalFilter_breath);
			initialFindPV(&stFindPV);
			FindPV(&stFindPV, SignalFilter_breath);
			current_breath_rate = get_breath(&stFindPV, SignalFilter_breath, &count);

			// 发送结果
			Send_Data_To_PC((int)current_heart_rate, (int)current_breath_rate);

			slide_ready = 0; // 重置处理标志
		}
	}
}

// 发送心率数据给计算机和U7
void Send_Data_To_PC(int current_heart_rate, int current_breath_rate)
{
	char buffer[50];
	sprintf(buffer, "%d|%d|E\r\n", current_heart_rate, current_breath_rate);
	USART_SendString(USART2, buffer);
	USART_SendString(USART1, buffer);
}

// 通过串口2发送无效消息
void Send_Invalid_Message(void)
{
	char buffer[50];
	sprintf(buffer, "Heart Rate: Invalid\r\n");
	USART_SendString(USART2, buffer);
}
