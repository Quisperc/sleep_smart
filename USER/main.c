#include "stm32f10x.h"
#include "fs800.h"
#include "AD.h"
#include "delay.h"
#include "TIM3.h"
#include "usart.h"
#include "DMA.h"
#include "Filter.h"
#include "findpeak.h"
#include <string.h>
#define ADC_NUM 1200	  // 窗口大小（30秒 × 40Hz）
#define SLIDE_STEP 120	  // 滑动步长（3秒 × 40Hz）
#define DeviceIDLength 10 // 设备ID长度

// 全局变量
extern uint16_t slide_counter;				// 滑动计数器（在DMA.c中中断计数）
uint8_t timecount = 0;						// 时间计数
SFindPV stFindPV;							// 波峰波谷
uint8_t count = 0;							// 波峰波谷计数
float SignalFilter_heart[ADC_NUM] = {0.0};	// 滤波后的数据缓冲区
float SignalFilter_breath[ADC_NUM] = {0.0}; // 滤波后的数据缓冲区
extern int slide_ready;						// 滑动处理完成标志
uint8_t DeviceID[DeviceIDLength] = {0};		// 设备ID
uint8_t SendTime = 10;						// 发送报告时间间隔
uint8_t report_data[120] = {0};				// 数据报告

uint8_t heart_beat[3] = {0};  // 心率
uint8_t breath_beat[3] = {0}; // 呼吸率
uint8_t heart_freq = 0, breath_freq = 0;
uint8_t pose_state = 0;
uint8_t index2 = 0;		// 心率、呼吸率索引
uint8_t ready_flag = 0; // 心率、呼吸率数据准备标志

// 函数声明
void Send2Report(int current_heart_rate, int current_breath_rate);
void refresh_data(void);

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
			refresh_data();
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
}

void refresh_data(void)
{

	// 数据滑动：前移 1080 个点
	memmove(adc_data, adc_data + SLIDE_STEP, sizeof(float) * (ADC_NUM - SLIDE_STEP));
	// 复制新数据到末尾
	memcpy(adc_data + (ADC_NUM - SLIDE_STEP), data_sample, sizeof(float) * SLIDE_STEP);
	int i;

	// 滤波
	Heart_filter(adc_data, SignalFilter_heart);
	initialFindPV(&stFindPV);
	FindPV(&stFindPV, SignalFilter_heart);
	heart_freq = (uint8_t)get_heart(&stFindPV, SignalFilter_heart, &count);

	Breath_filter(adc_data, SignalFilter_breath);
	initialFindPV(&stFindPV);
	FindPV(&stFindPV, SignalFilter_breath);
	breath_freq = (uint8_t)get_breath(&stFindPV, SignalFilter_breath, &count);

	Send2Report(heart_freq, breath_freq);

	// 姿态判断（心率过快）
	if (heart_freq > 39)
	{
		pose_state = 1;
	}

	// 平滑滤波（3次滑动窗口平均）
	heart_beat[index2] = heart_freq;
	breath_beat[index2] = breath_freq;
	index2++;
	if (index2 >= 3)
	{
		ready_flag = 1;
		index2 = 0;
	}

	if (ready_flag)
	{
		uint16_t sum_heart = 0, sum_breath = 0;
		for (i = 0; i < 3; i++)
		{
			sum_heart += heart_beat[i];
			sum_breath += breath_beat[i];
		}
		heart_freq = sum_heart / 3;
		breath_freq = sum_breath / 3;
		// Send2Report(heart_freq, breath_freq);

		ready_flag = 0;

		// 生成报文构造（帧头+数据+校验）
		report_data[0] = 0x42; // 帧头 B
		for (int i = 0; i < DeviceIDLength; i++)
		{
			report_data[1 + i] = DeviceID[i];
		}
		report_data[1 + DeviceIDLength] = 0x7C; // 间隔 |
		report_data[1 + DeviceIDLength + 1] = heart_freq;
		report_data[1 + DeviceIDLength + 2] = 0x7C; // 间隔 |
		report_data[1 + DeviceIDLength + 3] = breath_freq;
		report_data[1 + DeviceIDLength + 4] = 0x7C; // 间隔 |
		report_data[1 + DeviceIDLength + 5] = pose_state;
		report_data[1 + DeviceIDLength + 6] = 0x7C; // 间隔 |
		report_data[1 + DeviceIDLength + 7] = 0x30; // 间隔 |
		report_data[1 + DeviceIDLength + 8] = 0x7C; // 间隔 |
		report_data[1 + DeviceIDLength + 9] = 0x45; // 帧尾 E

		// 校验
		report_data[1 + DeviceIDLength + 10] = '\r';
		report_data[1 + DeviceIDLength + 11] = '\n';
		report_data[1 + DeviceIDLength + 12] = '\0';
	}
	// 定时发送

	if (timecount >= SendTime)
	{
		USART_OUT(USART1, report_data, DeviceIDLength + 14);
		USART_OUT(USART2, report_data, DeviceIDLength + 14);
		timecount = 0;
	}
}
