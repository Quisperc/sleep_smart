#ifndef _DMA_H
#define _DMA_H

// 宏定义
#define ADC_NUM 1200   // 30秒数据，40Hz采样率
#define SLIDE_STEP 120 // 3秒滑动步长

// 滤波器参数
#define FILTER_ORDER 4 // 4阶巴特沃斯滤波器

// 心率滤波器系数 (0.66Hz-2.5Hz)
extern float hr_b[FILTER_ORDER + 1];
extern float hr_a[FILTER_ORDER + 1];

// 呼吸滤波器系数 (0.13Hz-0.53Hz)
extern float br_b[FILTER_ORDER + 1];
extern float br_a[FILTER_ORDER + 1];

// 滤波器状态变量
extern float hr_x[FILTER_ORDER + 1]; // 心率输入历史
extern float hr_y[FILTER_ORDER + 1]; // 心率输出历史
extern float br_x[FILTER_ORDER + 1]; // 呼吸输入历史
extern float br_y[FILTER_ORDER + 1]; // 呼吸输出历史

// 外部变量声明
extern uint16_t ADC_Data1[2];			 // ADC数据缓冲区，改为数组
extern uint16_t p1[ADC_NUM];			 // 温度数据缓冲区
extern uint16_t index1;					 // 数据索引
extern uint8_t do_flag;					 // 数据传输完成标志

// 中断服务函数
void DMA1_Channel1_IRQHandler(void);
// 函数声明
void DMAx_Mode_Config(void); // DMA模式配置
void DMAx_NVIC_Config(void); // DMA中断配置
void Filter_Init(void); // 滤波器初始化
float HR_Filter(float input); // 心率滤波
float BR_Filter(float input); // 呼吸滤波
int Calculate_Heart_Rate(uint16_t *data, int length); // 计算心率
int Find_Peaks(float *data, int length, int *peaks, int max_peaks); // 寻找峰值

#endif
