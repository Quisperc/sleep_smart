#include <stdio.h>
#include <stdlib.h>
#include "stm32f10x.h"
#include "findpeak.h"
float SampleDiff[SAMPLE_MAX] = {0};

/********************************************
 *  Fuction : initialFindPV
 *  Note    : 初始化相关数据
 *******************************************/
void initialFindPV(SFindPV *stFindPV)
{
    int Index = 0;

    for (Index = 0; Index < SAMPLE_MAX; Index++)
    {
        SampleDiff[Index] = 0;
    }

    for (Index = 0; Index < PV_MAX; Index++)
    {
        stFindPV->Pos_Peak[Index] = -1;
        stFindPV->Pos_Valley[Index] = -1;
    }
    stFindPV->Pcnt = 0;
    stFindPV->Vcnt = 0;
}

/********************************************
 *  Fuction : FindPV
 *  Note    : 找波峰波谷
 *******************************************/
void FindPV(SFindPV *pFindPV, float *Sample)
{
    int i = 0;

    // step 1 :首先进行前向差分，并归一化
    for (i = 0; i < SAMPLE_MAX - 1; i++)
    {
        if (Sample[i + 1] - Sample[i] > 0)
        {
            SampleDiff[i] = 1;
        }
        else if (Sample[i + 1] - Sample[i] < 0)
        {
            SampleDiff[i] = -1;
        }
        else
        {
            SampleDiff[i] = 0;
        }
    }

    // step 2 :对相邻相等的点进行领边坡度处理
    for (i = 0; i < SAMPLE_MAX - 1; i++)
    {
        if (SampleDiff[i] == 0)
        {
            if (i == (SAMPLE_MAX - 2))
            {
                if (SampleDiff[i - 1] >= 0)
                {
                    SampleDiff[i] = 1;
                }
                else
                {
                    SampleDiff[i] = -1;
                }
            }
            else
            {
                if (SampleDiff[i + 1] >= 0)
                {
                    SampleDiff[i] = 1;
                }
                else
                {
                    SampleDiff[i] = -1;
                }
            }
        }
    }
    // step 3 :对相邻相等的点进行领边坡度处理
    for (i = 0; i < SAMPLE_MAX - 1; i++)
    {
        if (SampleDiff[i + 1] - SampleDiff[i] == -2) // 波峰识别
        {
            pFindPV->Pos_Peak[pFindPV->Pcnt] = i + 1;
            pFindPV->Pcnt++;
        }
        else if (SampleDiff[i + 1] - SampleDiff[i] == 2) // 波谷识别
        {
            pFindPV->Pos_Valley[pFindPV->Vcnt] = i + 1;
            pFindPV->Vcnt++;
        }
    }
}
