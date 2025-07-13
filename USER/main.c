#include "stm32f10x.h"
#include "fs800.h"
#include "AD.h"
#include "delay.h"
#include "TIM3.h"
#include "usart.h"
#include "DMA.h"
#include "Calculate.h"

#define SLIDE_STEP 120 // 滑动窗口步长
// 函数声明
void Send_Data_To_PC(void);
void Send_Heart_Rate_To_PC(int heart_rate);
void Slide_Window(void);

// 通过串口2发送无效消息
void Send_Invalid_Message(void);

// 全局变量
uint8_t slide_flag = 0;			   // 滑动窗口标志
extern uint16_t slide_counter;	   // 滑动计数器（在DMA.c中中断计数）
uint16_t data_buffer[ADC_NUM];	   // 数据缓冲区
float current_heart_rate = 100; // 心率
int current_breath_rate = 0;	   // 呼吸率

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
		// 检查30秒数据是否完成
		if (do_flag == 1)
		{
			// 复制数据到缓冲区
			int i;
			if (slide_flag == 0)
				for (i = 0; i < ADC_NUM; i++)
				{
					data_buffer[i] = p1[i];
				}
			// 计算心率
			Calculate((float*)data_buffer, &current_heart_rate, (float *)current_breath_rate);
			// 发送完整30秒数据
			//Send_Data_To_PC();
			Send_Heart_Rate_To_PC(current_heart_rate);

			// 开始滑动窗口模式
			slide_flag = 1;
			slide_counter = 0;

			do_flag = 0;
			index1 = 0; // 重置索引，准备滑动
		}

		// 滑动窗口模式
		if (slide_flag == 1)
		{
			// slide_counter++;
			if (slide_counter >= SLIDE_STEP) // 每3秒滑动一次
			{
				Slide_Window();

				// 计算新的心率
				// current_heart_rate = Calculate_Heart_Rate(data_buffer, ADC_NUM);
				Send_Heart_Rate_To_PC(current_heart_rate);

				slide_counter = 0;
			}
		}
	}
}
// 滑动窗口函数
void Slide_Window(void)
{
	int i;

	// 向前移动数据，删除前120个数据点
	for (i = 0; i < ADC_NUM - SLIDE_STEP; i++)
	{
		data_buffer[i] = data_buffer[i + SLIDE_STEP];
	}

	// 添加新的120个数据点
	for (i = 0; i < SLIDE_STEP; i++)
	{
		data_buffer[ADC_NUM - SLIDE_STEP + i] = p1[i];
	}

	USART_SendString(USART2, "Window slided - added 120 new data points\r\n");
}

// 通过串口2发送心率数据
void Send_Heart_Rate_To_PC(int heart_rate)
{
	char buffer[50];
	sprintf(buffer, "HR:%d\r\n", heart_rate);
	USART_SendString(USART2, buffer);
}

// 通过串口2发送无效消息
void Send_Invalid_Message(void)
{
	char buffer[50];
	sprintf(buffer, "Heart Rate: Invalid\r\n");
	USART_SendString(USART2, buffer);
}

// 通过串口2发送完整30秒数据到计算机
void Send_Data_To_PC(void)
{
	uint16_t i;

	USART_SendString(USART2, "30SEC_COMPLETE_DATA_START\r\n");

	for (i = 0; i < 120; i++)
	{
		USART_SendNumber(USART2, data_buffer[i]); // 发送数字
		USART_SendData(USART2, ',');			  // 分隔符
		while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);

		if ((i + 1) % 10 == 0)
		{
			USART_SendString(USART2, "\r\n");
		}
	}

	USART_SendString(USART2, "\r\n30SEC_COMPLETE_DATA_END\r\n");
}
