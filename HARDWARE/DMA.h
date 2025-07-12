#ifndef _DMA_H
#define _DMA_H

// 宏定义
#define ADC_NUM 100 // ADC数据缓冲区大小

// 外部变量声明
extern uint16_t ADC_Data1[2]; // ADC数据缓冲区，改为数组
extern uint16_t p1[ADC_NUM];  // 温度数据缓冲区
extern uint16_t index1;		  // 数据索引
extern uint8_t do_flag;		  // 数据传输完成标志

// 函数声明
void DMAx_Mode_Config(void);
void DMAx_NVIC_Config(void);
void DMA1_Channel1_IRQHandler(void);
#endif
