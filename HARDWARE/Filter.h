#ifndef _FILTER_H
#define _FILTER_H
// 外部变量
#define ADC_NUM 1200 // 30秒数据，40Hz采样率

//void Breath_filter(float *SignalFilter,float *OriginalSignal);
//void Heart_filter(float *SignalFilter,float *OriginalSignal);

void Breath_filter(float *OriginalSignal);
void Heart_filter(float *OriginalSignal);
#endif
