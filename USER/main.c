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
void Send2Report(int current_heart_rate, int current_breath_rate);

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
			Send2Report((int)current_heart_rate, (int)current_breath_rate);

			slide_ready = 0; // 重置处理标志
		}
	}
}

// 发送心率数据给计算机和U7
void Send2Report(int current_heart_rate, int current_breath_rate)
{
	char buffer[50];
	sprintf(buffer, "%d|%d|E\r\n", current_heart_rate, current_breath_rate);
	USART_SendString(USART2, buffer);
	USART_SendString(USART1, buffer);
}

// void refresh_data(void)
// {
// 	uint16_t cnt;
// 	uint8_t ecc = 0;
// 	uint8_t pose_state = 0;
// 	uint8_t heart_freq, breath_freq;
// 	int i;
// 	cnt = 0;
// 	for (i = TIM_NUM; i < ADC_NUM; i++)
// 	{
// 		p1[cnt] = p2[i];
// 		cnt++;
// 	}
// 	calculate(p2, &heart_data[0], &breath_data[0], &heart_cnt[0]);
// 	heart_freq = (uint8_t)heart_data[0];
// 	breath_freq = (uint8_t)breath_data[0];
// 	pose_state = 0;
// 	if (heart_freq > 39)
// 	{
// 		pose_state = 1;
// 	}
// 	heart_beat[index2] = heart_freq;
// 	breath_beat[index2] = breath_freq;
// 	index2++;
// 	if (index2 >= 3)
// 	{
// 		ready_flag = 1;
// 		index2 = index2 % 3;
// 	}
// 	if (ready_flag)
// 	{
// 		heart_freq = (uint8_t)((heart_beat[0] + heart_beat[1] + heart_beat[2] + heart_beat[3] + heart_beat[4] + heart_beat[5]) / 6);
// 		breath_freq = (uint8_t)((breath_beat[0] + breath_beat[1] + breath_beat[2] + breath_beat[3] + breath_beat[4] + breath_beat[5]) / 6);
// 		report_data[0] = 0xfd;
// 		report_data[1] = 0x00;
// 		report_data[2] = 0x04;
// 		report_data[3] = heart_freq;
// 		report_data[4] = breath_freq;
// 		report_data[5] = pose_state;
// 		report_data[6] = 0;
// 		if (sample_cnt >= 10)
// 		{
// 			report_data[6] = 1;
// 		}
// 		ecc = 0;
// 		for (i = 0; i < 7; i++)
// 		{
// 			ecc = ecc ^ report_data[i];
// 		}
// 		if (sample_cnt >= 10)
// 		{
// 			if (pose_state == 1)
// 			{
// 				report_data[7] = heart_peak[0];
// 				ecc = ecc ^ report_data[7];
// 				for (i = 0; i < 7 + heart_peak[0]; i++)
// 				{
// 					report_data[8 + i] = heart_peak[i + 1];
// 					ecc = ecc ^ report_data[8 + i];
// 				}
// 				report_data[8 + i] = ecc;
// 				USART_OUT(USART1, report_data, 9 + i);
// 			}
// 			sample_cnt = 0;
// 		}
// 		else
// 		{
// 			report_data[7] = ecc;
// 			USART_OUT(USART1, report_data, 8);
// 		}
// 		memset(report_data, 0, 120);
// 	}
// }

void refresh_data(void)
{
	static uint8_t heart_beat[6] = {0};
	static uint8_t breath_beat[6] = {0};
	static uint8_t index2 = 0;
	static uint8_t ready_flag = 0;
	uint8_t heart_freq = 0, breath_freq = 0;
	uint8_t pose_state = 0;
	uint8_t ecc = 0;
	char report_data[20] = {0}; // 简化版，原本是120字节
	int i;

	// 滤波
	Heart_filter(data_buffer, SignalFilter_heart);
	initialFindPV(&stFindPV);
	FindPV(&stFindPV, SignalFilter_heart);
	heart_freq = (uint8_t)get_heart(&stFindPV, SignalFilter_heart, &count);

	Breath_filter(data_buffer, SignalFilter_breath);
	initialFindPV(&stFindPV);
	FindPV(&stFindPV, SignalFilter_breath);
	breath_freq = (uint8_t)get_breath(&stFindPV, SignalFilter_breath, &count);

	// 姿态判断（心率过快）
	if (heart_freq > 39)
	{
		pose_state = 1;
	}

	// 平滑滤波（6次滑动窗口平均）
	heart_beat[index2] = heart_freq;
	breath_beat[index2] = breath_freq;
	index2++;
	if (index2 >= 6)
	{
		ready_flag = 1;
		index2 = 0;
	}

	if (ready_flag)
	{
		uint16_t sum_heart = 0, sum_breath = 0;
		for (i = 0; i < 6; i++)
		{
			sum_heart += heart_beat[i];
			sum_breath += breath_beat[i];
		}
		heart_freq = sum_heart / 6;
		breath_freq = sum_breath / 6;

		// 报文构造（简版：帧头+数据+校验）
		report_data[0] = 0xFD; // 帧头
		report_data[1] = heart_freq;
		report_data[2] = breath_freq;
		report_data[3] = pose_state;

		// 校验
		ecc = report_data[0] ^ report_data[1] ^ report_data[2] ^ report_data[3];
		report_data[4] = ecc;

		// 发送
		USART_OUT(USART1, (uint8_t *)report_data, 5);
	}
}
