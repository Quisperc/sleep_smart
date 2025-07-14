#include "filter.h"
#define ADC_NUM 1200
int i;
// 心率滤波器系数 (0.65Hz-2.5Hz, 40Hz采样率)
float hr_b[5] = {0.01743180677, 0.0000, -0.03486361355, 0.0000, 0.01743180677};
float hr_a[5] = {1, -3.521024704, 4.719986916, -2.860702038, 0.663058579};

// // 呼吸滤波器系数 (0.13Hz-0.45Hz, 40Hz采样率)
float br_b[5] = {0.0006098547019, 0, -0.001219709404, 0, 0.0006098547019};
float br_a[5] = {1, -3.926106453, 5.783695698, -3.788968801, 0.9313817024};

// 呼吸滤波
void Breath_filter(float *OriginalSignal, float *SignalFilter)
{
	SignalFilter[0] = br_b[0] * OriginalSignal[0];
	SignalFilter[1] = br_b[0] * OriginalSignal[1] + br_b[1] * OriginalSignal[0] - br_a[1] * SignalFilter[0];
	SignalFilter[2] = br_b[0] * OriginalSignal[2] + br_b[1] * OriginalSignal[1] + br_b[2] * OriginalSignal[0] - br_a[1] * SignalFilter[1] - br_a[2] * SignalFilter[0];
	SignalFilter[3] = br_b[0] * OriginalSignal[3] + br_b[1] * OriginalSignal[2] + br_b[2] * OriginalSignal[1] + br_b[3] * OriginalSignal[0] - br_a[1] * SignalFilter[2] - br_a[2] * SignalFilter[1] - br_a[3] * SignalFilter[0];

	for (i = 4; i < ADC_NUM; i++)
	{
		SignalFilter[i] = br_b[0] * OriginalSignal[i] + br_b[1] * OriginalSignal[i - 1] + br_b[2] * OriginalSignal[i - 2] + br_b[3] * OriginalSignal[i - 3] + br_b[4] * OriginalSignal[i - 4] - br_a[1] * SignalFilter[i - 1] - br_a[2] * SignalFilter[i - 2] - br_a[3] * SignalFilter[i - 3] - br_a[4] * SignalFilter[i - 4];
	}
}

// 心率滤波
void Heart_filter(float *OriginalSignal, float *SignalFilter)
{
	SignalFilter[0] = hr_b[0] * OriginalSignal[0];
	SignalFilter[1] = hr_b[0] * OriginalSignal[1] + hr_b[1] * OriginalSignal[0] - hr_a[1] * SignalFilter[0];
	SignalFilter[2] = hr_b[0] * OriginalSignal[2] + hr_b[1] * OriginalSignal[1] + hr_b[2] * OriginalSignal[0] - hr_a[1] * SignalFilter[1] - hr_a[2] * SignalFilter[0];
	SignalFilter[3] = hr_b[0] * OriginalSignal[3] + hr_b[1] * OriginalSignal[2] + hr_b[2] * OriginalSignal[1] + hr_b[3] * OriginalSignal[0] - hr_a[1] * SignalFilter[2] - hr_a[2] * SignalFilter[1] - hr_a[3] * SignalFilter[0];

	for (i = 4; i < ADC_NUM; i++)
	{
		SignalFilter[i] = hr_b[0] * OriginalSignal[i] + hr_b[1] * OriginalSignal[i - 1] + hr_b[2] * OriginalSignal[i - 2] + hr_b[3] * OriginalSignal[i - 3] + hr_b[4] * OriginalSignal[i - 4] - hr_a[1] * SignalFilter[i - 1] - hr_a[2] * SignalFilter[i - 2] - hr_a[3] * SignalFilter[i - 3] - hr_a[4] * SignalFilter[i - 4];
	}
}
