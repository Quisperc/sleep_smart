#include "stm32f10x.h"
#include "DMA.h"
#include "math.h"

// DMA相关变量定义
uint16_t ADC_Data1[2];		// ADC数据缓冲区
uint16_t p1[ADC_NUM];		// 温度数据缓冲区
uint16_t index1 = 0;		// 数据索引
uint8_t do_flag = 0;		// 数据传输完成标志
uint16_t slide_counter = 0; // 滑动计数器

// 滤波器缓冲区
float hr_filtered[ADC_NUM]; // 心率滤波后数据
float br_filtered[ADC_NUM]; // 呼吸滤波后数据

// 心率滤波器系数 (0.66Hz-2.5Hz, 40Hz采样率)
float hr_b[FILTER_ORDER + 1] = {0.0067, 0.0000, -0.0135, 0.0000, 0.0067};
float hr_a[FILTER_ORDER + 1] = {1.0000, -2.9754, 3.8055, -2.1978, 0.5683};

// 呼吸滤波器系数 (0.13Hz-0.53Hz, 40Hz采样率)
float br_b[FILTER_ORDER + 1] = {0.0008, 0.0000, -0.0017, 0.0000, 0.0008};
float br_a[FILTER_ORDER + 1] = {1.0000, -3.6493, 5.2695, -3.4925, 0.8852};

// 滤波器状态变量
float hr_x[FILTER_ORDER + 1] = {0}; // 心率输入历史
float hr_y[FILTER_ORDER + 1] = {0}; // 心率输出历史
float br_x[FILTER_ORDER + 1] = {0}; // 呼吸输入历史
float br_y[FILTER_ORDER + 1] = {0}; // 呼吸输出历史

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
	// uint8_t i;
	// uint8_t char_dat[6];
	// uint16_t temp;
	if (DMA_GetITStatus(DMA1_IT_TC1) != RESET)
	{
		DMA_ClearITPendingBit(DMA1_IT_TC1);
		p1[index1] = ADC_Data1[0] * 0.806;
		index1++;
		slide_counter++;
		if (index1 >= ADC_NUM)
		{
			do_flag = 1;
		}
	}
}

void Filter_Init(void)
{
	int i;
	// 初始化滤波器状态
	for (i = 0; i <= FILTER_ORDER; i++)
	{
		hr_x[i] = 0;
		hr_y[i] = 0;
		br_x[i] = 0;
		br_y[i] = 0;
	}
}

// 心率滤波器 (0.66Hz-2.5Hz)
float HR_Filter(float input)
{
	int i;
	float output = 0;

	// 移位输入历史
	for (i = FILTER_ORDER; i > 0; i--)
	{
		hr_x[i] = hr_x[i - 1];
	}
	hr_x[0] = input;

	// 计算输出
	for (i = 0; i <= FILTER_ORDER; i++)
	{
		output += hr_b[i] * hr_x[i];
	}

	for (i = 1; i <= FILTER_ORDER; i++)
	{
		output -= hr_a[i] * hr_y[i];
	}

	// 移位输出历史
	for (i = FILTER_ORDER; i > 0; i--)
	{
		hr_y[i] = hr_y[i - 1];
	}
	hr_y[0] = output;

	return output;
}

// 呼吸滤波器 (0.13Hz-0.53Hz)
float BR_Filter(float input)
{
	int i;
	float output = 0;

	// 移位输入历史
	for (i = FILTER_ORDER; i > 0; i--)
	{
		br_x[i] = br_x[i - 1];
	}
	br_x[0] = input;

	// 计算输出
	for (i = 0; i <= FILTER_ORDER; i++)
	{
		output += br_b[i] * br_x[i];
	}

	for (i = 1; i <= FILTER_ORDER; i++)
	{
		output -= br_a[i] * br_y[i];
	}

	// 移位输出历史
	for (i = FILTER_ORDER; i > 0; i--)
	{
		br_y[i] = br_y[i - 1];
	}
	br_y[0] = output;

	return output;
}

// 寻峰算法
int Find_Peaks(float *data, int length, int *peaks, int max_peaks)
{
	int i, peak_count = 0;
	float sum = 0, mean = 0;
	float threshold;

	// 计算平均值
	for (i = 0; i < length; i++)
	{
		sum += data[i];
	}
	mean = sum / length;

	// 设置阈值：平均值的0.7倍 + 固定阈值40
	threshold = mean * 0.7 + 40;

	// 寻找峰值
	for (i = 1; i < length - 1; i++)
	{
		// 检查是否为局部最大值且超过阈值
		if (data[i] > data[i - 1] && data[i] > data[i + 1] && data[i] > threshold)
		{
			// 检查与前一个峰值的间隔（至少15个采样点，最多75个采样点）
			if (peak_count == 0 || (i - peaks[peak_count - 1]) >= 15)
			{
				if (peak_count == 0 || (i - peaks[peak_count - 1]) <= 75)
				{
					peaks[peak_count] = i;
					peak_count++;
					if (peak_count >= max_peaks)
						break;
				}
			}
		}
	}

	return peak_count;
}

// 计算心率
int Calculate_Heart_Rate(uint16_t *data, int length)
{
	int i, j;
	int peaks[100]; // 最多100个峰值
	int peak_count;
	float sum_interval = 0;
	float avg_interval;
	float heart_rate;

	// 对原始数据进行心率滤波
	for (i = 0; i < length; i++)
	{
		hr_filtered[i] = HR_Filter((float)data[i]);
	}

	// 寻找峰值
	peak_count = Find_Peaks(hr_filtered, length, peaks, 100);

	if (peak_count < 2)
	{
		return 0; // 峰值太少，无法计算心率
	}

	// 计算峰值间隔平均值
	for (i = 1; i < peak_count; i++)
	{
		sum_interval += (peaks[i] - peaks[i - 1]);
	}

	avg_interval = sum_interval / (peak_count - 1);

	// 将采样点间隔转换为时间间隔（秒）
	avg_interval = avg_interval * 0.025; // 25ms采样间隔

	// 计算心率 (次/分钟)
	heart_rate = 60.0 / avg_interval;

	// 合理性检查
	if (heart_rate < 30 || heart_rate > 200)
	{
		return 0; // 心率不在合理范围内
	}

	return (int)heart_rate;
}
