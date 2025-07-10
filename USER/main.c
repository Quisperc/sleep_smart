#include "stm32f10x.h"
void Delay(u32 count)
{
	u32 i = 0;
	for (; i < count; i++)
		;
}
void ADC1_Power_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* 1) 打开 GPIOA 和 ADC1、PWR 外设时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |
							   RCC_APB2Periph_ADC1,
						   ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

	/* 2) PA0 配置为模拟输入 （ADC IN0） */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* 3) PC14 作为电源状态检测脚，配置为下拉输入 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void ADC1_Init(void)
{
	ADC_InitTypeDef ADC_InitStructure;

	/* 4) ADC 去初始化，保证干净状态 */
	ADC_DeInit(ADC1);

	/* 5) ADC 模式配置 */
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	   // 单通道
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE; // 连续转换
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; // 右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 1;				   // 转换通道数
	ADC_Init(ADC1, &ADC_InitStructure);

	/* 6) 配置规则通道：通道 0，序列 1，采样时间 55.5 周期 */
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5);

	/* 7) 使能 ADC1 */
	ADC_Cmd(ADC1, ENABLE);

	/* 8) 校准 ADC */
	ADC_ResetCalibration(ADC1);
	while (ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);
	while (ADC_GetCalibrationStatus(ADC1));

	/* 9) 启动软件转换 */
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}
int main(void)
{
	uint16_t adc_val;

	/* 初始化 GPIO、ADC、PWR */
	ADC1_Power_GPIO_Init();
	ADC1_Init();

	while (1)
	{
		/* 读取 ADC1 的最新转换值 */
		adc_val = ADC_GetConversionValue(ADC1);

		/* TODO: 根据 PC14（PWR）输入判断电源状态 */
		if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_14) == Bit_RESET)
		{
			/* 电源正常或低电平态 */
		}
		else
		{
			/* 电源异常或高电平态 */
		}
	}
}
