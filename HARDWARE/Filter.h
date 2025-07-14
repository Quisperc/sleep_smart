#ifndef __FILTER_H__
#define __FILTER_H__

#ifdef __cplusplus
extern "C"
{
#endif
#define ADC_NUM 1200 // 30秒数据，40Hz采样率

	void Heart_filter(float *OriginalSignal, float *SignalFilter);
	void Breath_filter(float *OriginalSignal, float *SignalFilter);

#ifdef __cplusplus
}
#endif
#endif
