
#ifndef findpeak_H
#define findpeak_H
#define SAMPLE_MAX 1200
#define PV_MAX 100
typedef struct _tag_FindPV
{
    int Pos_Peak[PV_MAX];   // 波峰位置存储
    int Pos_Valley[PV_MAX]; // 波谷位置存储
    int Pcnt;               // 所识别的波峰计数
    int Vcnt;               // 所识别的波谷计数
} SFindPV;

void initialFindPV(SFindPV *pFindPV);
void FindPV(SFindPV *pFindPV, float *Sample);
#endif
