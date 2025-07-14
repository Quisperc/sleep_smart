#ifndef _DMA_H
#define _DMA_H
#include "stm32f10x.h"
// 宏定义
#define ADC_NUM 1200   // 30秒数据，40Hz采样率
#define SLIDE_STEP 120 // 滑动步长（3秒 × 40Hz）
// 外部变量声明
extern uint32_t ADC_value;		// ADC数据缓冲区
extern float adc_data[ADC_NUM]; // 数据缓冲区
extern uint16_t index1;			// 数据索引
extern uint8_t do_flag;			// 数据传输完成标志

// 中断服务函数
void DMA1_Channel1_IRQHandler(void);
// 函数声明
void DMAx_Mode_Config(void); // DMA模式配置
void DMAx_NVIC_Config(void); // DMA中断配置

// 添加数据到环形缓冲区
void AddData(float sample);
// 获取环形缓冲区中的数据
void GetWindow(float *output, int size);
#endif
