#ifndef _CALCULATE_H
#define _CALCULATE_H
#include "stm32f10x.h"
#include "findpeak.h"
// 计算心率
void Calculate(float *data, float *my_heart, float *my_breath);
// 获取心率
float get_heart(SFindPV *pFindPV, float *Sample);
// 获取呼吸率
float get_breath(SFindPV *pFindPV, float *Sample);
#endif // _CALCULATE_H
