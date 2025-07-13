#include "Calculate.h"
#include "Filter.h"

float data2[ADC_NUM];
int heart_peak[PV_MAX];
// 计算心率和呼吸率
// void Calculate(float *data, float *my_heart, float *my_breath, uint8_t *count)
void Calculate(float *data, uint16_t *my_heart, float *my_breath)
{
	// for (int i = 0; i < ADC_NUM;i++) data2[i] = data[i];
	Heart_filter(data);
	SFindPV stFindPV;
	initialFindPV(&stFindPV);
	FindPV(&stFindPV, data);
	// *my_heart = get_heart(stFindPV, SignalFilter, count);
	*my_heart = get_heart(&stFindPV, data);
	//*my_heart=200;

	// Breath_filter(data2);
	// initialFindPV(&stFindPV);
	// FindPV(&stFindPV, data2);
	// *my_breath = get_breath(&stFindPV, data2);
}

// 获取心率
// float get_heart(SFindPV *pFindPV, float *Sample, uint8_t *count)
float get_heart(SFindPV *pFindPV, float *Sample)
{
	int k, kk, i, cnt;
	float period, freq, door_limit, avage, sum;
	int delta, max = 0, min = 150, dis;
	door_limit = 0.0;
	kk = 0;
	max = 0.0;
	sum = 0.0;
	//return pFindPV->Pcnt;
	for (k = 0; k < pFindPV->Pcnt; k++)
	{
		if (Sample[pFindPV->Pos_Peak[k]] > 15)
		{
			sum = sum + Sample[pFindPV->Pos_Peak[k]];
			pFindPV->Pos_Peak[kk] = pFindPV->Pos_Peak[k];
			kk++;
		}
	}
	avage = sum / kk;
	pFindPV->Pcnt = kk;
	door_limit = avage * 0.95 + 67;
	kk = 0;
	for (k = 0; k < pFindPV->Pcnt; k++)
		if (Sample[pFindPV->Pos_Peak[k]] >= door_limit)
		{
			pFindPV->Pos_Peak[kk] = pFindPV->Pos_Peak[k];
			kk++;
		}
	pFindPV->Pcnt = kk;
	avage = (pFindPV->Pos_Peak[pFindPV->Pcnt - 1] - pFindPV->Pos_Peak[0]) / (pFindPV->Pcnt - 1);
	kk = 0;
	delta = 0;
	i = 0;
	for (k = 0; k < pFindPV->Pcnt - 1; k++)
	{
		dis = pFindPV->Pos_Peak[k + 1] - pFindPV->Pos_Peak[k];
		if ((dis < 15) || (dis > 70))
		{
			delta = delta + dis;
			kk = kk + 1;
		}
		else
		{
			i = i + 1;
			heart_peak[i] = dis;
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
	// *count = pFindPV->Pcnt - kk;
	heart_peak[0] = i;
	cnt = pFindPV->Pcnt - kk - 3;
	return cnt;
	if (cnt > 1)
	{
		period = (pFindPV->Pos_Peak[pFindPV->Pcnt - 1] - pFindPV->Pos_Peak[0] - delta - max - min) * 0.025 / (cnt);
		freq = 60 / period;
		return freq;
	}
	return 0;
}

// 获取呼吸率
float get_breath(SFindPV *pFindPV, float *Sample)
{
	return 0.0;
}
