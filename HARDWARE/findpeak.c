#include <stdlib.h>
#include "findPeak.h"
#include "stdint.h"

#define heart_max 150
int SampleDiff[SAMPLE_MAX] = {0};
int heart_peak[heart_max] = {0};
int hx_peak[heart_max] = {0};

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

    /*for (i = 0; i < SAMPLE_MAX; i++)
    {
        //printf("diff[%d] = %d \t", i, SampleDiff[i]);
        if ( ((i + 1) & 0x03) == 0)
        {
           // printf("\n");
        }
    }*/

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
    // printf("\n");
    /*  for (i = 0; i < SAMPLE_MAX; i++)
      {
          //printf("diff2[%d] = %d \t", i, SampleDiff[i]);
          if ( ((i + 1) & 0x03) == 0)
          {
              //printf("\n");
          }
      }*/
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
    /*for (i = 0; i < SAMPLE_MAX; i++)
    {
        //printf("pFindPV->Pos_Valley[%d] = %d \t", i, pFindPV->Pos_Valley[i]);
        //printf("pFindPV->Pos_Peak[%d] = %d \t", i, pFindPV->Pos_Peak[i]);
        if ( ((i + 1) & 0x01) == 0)
        {
            //printf("\n");
        }
    }*/
}

/**
 * @brief 从采样数据中计算心率值
 * @param pFindPV 包含峰值位置信息的结构体指针
 * @param Sample 采样数据数组
 * @param count 用于返回有效峰值的计数
 * @return 计算得到的心率值(单位：次/分钟)，如果计算失败返回0
 */
float get_heart(SFindPV *pFindPV, float *Sample, uint8_t *count)
{
    int k, kk, i, cnt;
    float period, freq, door_limit, avage, sum;
    int delta, max = 0, min = 150, dis; // 初始化min为150(假设采样率下最大可能间隔)

    // 初始化变量
    door_limit = 0.0;
    kk = 0;
    max = 0.0;
    sum = 0.0;

    // 第一阶段：筛选有效峰值并计算平均值
    // 遍历所有检测到的峰值位置
    for (k = 0; k < pFindPV->Pcnt; k++)
    {
        // 只考虑幅度大于阈值l5的峰值
        if (Sample[pFindPV->Pos_Peak[k]] > 15)
        {
            sum = sum + Sample[pFindPV->Pos_Peak[k]];     // 累加峰值幅度
            pFindPV->Pos_Peak[kk] = pFindPV->Pos_Peak[k]; // 重新排列有效峰值位置
            kk++;
        }
    }

    avage = sum / kk;   // 计算平均峰值幅度
    pFindPV->Pcnt = kk; // 更新有效峰值数量

    // 设置动态阈值门限，用于进一步筛选峰值
    door_limit = avage * 0.95 + 67; // 经验公式，根据平均幅度调整

    // 第二阶段：使用动态阈值筛选峰值
    kk = 0;
    for (k = 0; k < pFindPV->Pcnt; k++)
    {
        // 只保留幅度大于动态阈值的峰值
        if (Sample[pFindPV->Pos_Peak[k]] >= door_limit)
        {
            pFindPV->Pos_Peak[kk] = pFindPV->Pos_Peak[k];
            kk++;
        }
    }
    pFindPV->Pcnt = kk; // 更新筛选后的峰值数量

    // 计算平均峰值间隔(采样点数量)
    avage = (pFindPV->Pos_Peak[pFindPV->Pcnt - 1] - pFindPV->Pos_Peak[0]) / (pFindPV->Pcnt - 1);

    // 第三阶段：分析峰值间隔，排除异常值
    kk = 0;
    delta = 0;
    i = 0;
    for (k = 0; k < pFindPV->Pcnt - 1; k++)
    {
        dis = pFindPV->Pos_Peak[k + 1] - pFindPV->Pos_Peak[k]; // 计算相邻峰值间隔

        // 判断是否为异常间隔(太短或太长)
        if ((dis < 15) || (dis > 70))
        {
            delta = delta + dis; // 累加异常间隔值
            kk = kk + 1;         // 异常间隔计数
        }
        else
        {
            i = i + 1;
            heart_peak[i] = dis; // 存储正常间隔

            // 更新最大和最小正常间隔
            if (max < dis)
            {
                max = dis;
            }
            if (min > dis)
            {
                min = dis;
            }
        }
    }

    *count = pFindPV->Pcnt - kk; // 返回有效峰值对数

    heart_peak[0] = i; // 存储正常间隔数量

    cnt = pFindPV->Pcnt - kk - 3; // 计算用于最终心率计算的间隔数量

    // 如果有足够多的有效间隔，计算心率
    if (cnt > 1)
    {
        // 计算平均周期(秒)，排除异常值和最大最小值
        period = (pFindPV->Pos_Peak[pFindPV->Pcnt - 1] - pFindPV->Pos_Peak[0] - delta - max - min) * 0.025 / cnt;

        // 转换为心率值(次/分钟)
        freq = 60 / period;
        return freq;
    }

    // 如果有效数据不足，返回0
    return 0;
}

/**
 * @brief 从采样数据中计算呼吸率值
 * @param pFindPV 包含峰值位置信息的结构体指针
 * @param Sample 采样数据数组
 * @param count 用于返回有效峰值的计数
 * @return 计算得到的呼吸率值(单位：次/分钟)，如果计算失败返回0
 */
float get_breath(SFindPV *pFindPV, float *Sample, uint8_t *count)
{
    int k, kk, i, cnt;
    float period, freq, door_limit, avage, sum;
    int delta, max = 0, min = 150, dis; // 初始化min为150(假设采样率下最大可能间隔)

    // 初始化变量
    door_limit = 0.0;
    kk = 0;
    max = 0.0;
    sum = 0.0;

    // 第一阶段：筛选有效峰值并计算平均值
    // 遍历所有检测到的峰值位置
    for (k = 0; k < pFindPV->Pcnt; k++)
    {
        // 只考虑幅度大于阈值5的峰值(呼吸信号通常比心率信号弱)
        if (Sample[pFindPV->Pos_Peak[k]] > 5)
        {
            sum = sum + Sample[pFindPV->Pos_Peak[k]];     // 累加峰值幅度
            pFindPV->Pos_Peak[kk] = pFindPV->Pos_Peak[k]; // 重新排列有效峰值位置
            kk++;
        }
    }

    avage = sum / kk;   // 计算平均峰值幅度
    pFindPV->Pcnt = kk; // 更新有效峰值数量

    // 设置动态阈值门限，用于进一步筛选峰值
    door_limit = avage * 0.85 + 10; // 经验公式，根据平均幅度调整(呼吸信号通常较弱)

    // 第二阶段：使用动态阈值筛选峰值
    kk = 0;
    for (k = 0; k < pFindPV->Pcnt; k++)
    {
        // 只保留幅度大于动态阈值的峰值
        if (Sample[pFindPV->Pos_Peak[k]] >= door_limit)
        {
            pFindPV->Pos_Peak[kk] = pFindPV->Pos_Peak[k];
            kk++;
        }
    }
    pFindPV->Pcnt = kk; // 更新筛选后的峰值数量

    // 计算平均峰值间隔(采样点数量)
    avage = (pFindPV->Pos_Peak[pFindPV->Pcnt - 1] - pFindPV->Pos_Peak[0]) / (pFindPV->Pcnt - 1);

    // 第三阶段：分析峰值间隔，排除异常值
    // 呼吸率的正常范围约为12-60次/分钟，对应的采样间隔(假设采样率为40Hz)约为40-200个采样点
    kk = 0;
    delta = 0;
    i = 0;
    for (k = 0; k < pFindPV->Pcnt - 1; k++)
    {
        dis = pFindPV->Pos_Peak[k + 1] - pFindPV->Pos_Peak[k]; // 计算相邻峰值间隔

        // 判断是否为异常间隔(太短或太长)
        if ((dis < 40) || (dis > 200))
        {
            delta = delta + dis; // 累加异常间隔值
            kk = kk + 1;         // 异常间隔计数
        }
        else
        {
            i = i + 1;
            hx_peak[i] = dis; // 存储正常间隔

            // 更新最大和最小正常间隔
            if (max < dis)
            {
                max = dis;
            }
            if (min > dis)
            {
                min = dis;
            }
        }
    }

    *count = pFindPV->Pcnt - kk; // 返回有效峰值对数

    hx_peak[0] = i; // 存储正常间隔数量

    cnt = pFindPV->Pcnt - kk - 3; // 计算用于最终呼吸率计算的间隔数量

    // 如果有足够多的有效间隔，计算呼吸率
    if (cnt > 1)
    {
        // 计算平均周期(秒)，排除异常值和最大最小值
        // 假设采样率为40Hz，每个采样点代表0.025秒
        period = (pFindPV->Pos_Peak[pFindPV->Pcnt - 1] - pFindPV->Pos_Peak[0] - delta - max - min) * 0.025 / cnt;

        // 转换为呼吸率值(次/分钟)
        freq = 60 / period;
        return freq;
    }

    // 如果有效数据不足，返回0
    return 0;
}
