#ifndef __FINDPEAK_H__
#define __FINDPEAK_H__

#ifdef __cplusplus
extern "C"
{
#endif
#include"stm32f10x.h"
#include "stdio.h"
#define SAMPLE_MAX 1200
#define PV_MAX 1200
    typedef struct _tag_FindPV
    {
        int Pos_Peak[PV_MAX];   // 波峰位置存储
        int Pos_Valley[PV_MAX]; // 波谷位置存储
        int Pcnt;               // 所识别的波峰计数
        int Vcnt;               // 所识别的波谷计数
    } SFindPV;

    void initialFindPV(SFindPV *stFindPV);
    void FindPV(SFindPV *pFindPV, float *Sample);
    float get_heart(SFindPV *pFindPV, float *Sample, uint8_t *count);
    float get_breath(SFindPV *pFindPV, float *Sample, uint8_t *count);

#ifdef __cplusplus
}
#endif
#endif
