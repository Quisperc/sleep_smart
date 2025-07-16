#include "sys.h"
#include "usart.h"

// 如果使用ucos,则包括下面的头文件即可.
// #if SYSTEM_SUPPORT_OS
// #include "includes.h" //ucos 使用
// #endif

// 加入以下代码,支持printf函数,而不需要选择use MicroLIB
#if 1
#pragma import(__use_no_semihosting)
// 标准库需要的支持函数
struct __FILE
{
	int handle;
};

FILE __stdout;
// 定义_sys_exit()以避免使用半主机模式
void _sys_exit(int x)
{
	x = x;
}
// 重定义fputc函数
int fputc(int ch, FILE *f)
{
	while ((USART1->SR & 0X40) == 0)
		; // 循环发送,直到发送完毕
	USART1->DR = (u8)ch;
	return ch;
}
#endif

// #if EN_USART1_RX // 如果使能了接收
// 串口1中断服务程序
// 注意,读取USARTx->SR能避免莫名其妙的错误
u8 USART_RX_BUF[USART_REC_LEN]; // 接收缓冲,最大USART_REC_LEN个字节.
// 接收状态
// bit15，	接收完成标志
// bit14，	接收到0x0d
// bit13~0，	接收到的有效字节数目
u8 USART_RX_STA = 0; // 接收状态标记

void uart_init(u32 bound)
{
	// GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE); // 使能USART1，GPIOA时钟

	// USART1_TX   GPIOA.9
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; // PA.9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // 复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure);			// 初始化GPIOA.9

	// USART1_RX	  GPIOA.10初始化
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;			  // PA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; // 浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);				  // 初始化GPIOA.10

	// Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; // 抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		  // 子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);							  // 根据指定的参数初始化VIC寄存器

	// USART 初始化设置

	USART_InitStructure.USART_BaudRate = bound;										// 串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						// 字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							// 一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;								// 无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					// 收发模式

	USART_Init(USART1, &USART_InitStructure);	   // 初始化串口1
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); // 开启串口接受中断
	USART_Cmd(USART1, ENABLE);					   // 使能串口1
}

void USART1_IRQHandler(void) // 串口1中断服务程序
{
	u8 Res;
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) // 接收中断
	{
		Res = USART_ReceiveData(USART1); // 读取接收到的数据

		if (USART_RX_STA < 200) // 防止越界
		{
			USART_RX_BUF[USART_RX_STA++] = Res;

			// 如果接收到 '\n' 并且前一个字符是 '\r'，认为一帧接收完成
			if (USART_RX_STA >= 2 &&
				USART_RX_BUF[USART_RX_STA - 2] == '\r' &&
				USART_RX_BUF[USART_RX_STA - 1] == '\n')
			{
				// 在此处可以封装处理完整的一帧数据
				// 比如发送到USART2或者设置标志位供主程序处理
				USART_OUT(USART2, USART_RX_BUF, sizeof(USART_RX_BUF)); // 示例：直接将完整数据回显

				USART_RX_STA = 0; // 清空接收状态
			}
		}
		else // 缓冲区溢出，重置
		{
			USART_RX_STA = 0;
		}
	}
}

void uart2_init(u32 bound)
{
	// GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE); // 使能USART2，GPIOA时钟

	// USART1_TX   GPIOA.2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; // PA.2
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // 复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure);			// 初始化GPIOA.2

	// USART1_RX	  GPIOA.3初始化
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;			  // PA.3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; // 浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);				  // 初始化GPIOA.3

	// Usart2 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; // 抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		  // 子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);							  // 根据指定的参数初始化VIC寄存器

	// USART 初始化设置
	USART_InitStructure.USART_BaudRate = bound;										// 串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						// 字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							// 一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;								// 无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					// 收发模式

	USART_Init(USART2, &USART_InitStructure);	   // 初始化串口2
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE); // 开启串口接受中断
	USART_Cmd(USART2, ENABLE);					   // 使能串口2
}

// #endif 与前面的#if匹配
void USART2_IRQHandler(void) // 串口2中断服务程序
{
	u8 Res;
	if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) // 接收中断
	{
		Res = USART_ReceiveData(USART2); // 读取接收到的数据

		if (USART_RX_STA < 200) // 防止越界
		{
			USART_RX_BUF[USART_RX_STA++] = Res;

			// 如果接收到 '\n' 并且前一个字符是 '\r'，认为一帧接收完成
			if (USART_RX_STA >= 2 &&
				USART_RX_BUF[USART_RX_STA - 2] == '\r' &&
				USART_RX_BUF[USART_RX_STA - 1] == '\n')
			{
				// 在此处可以封装处理完整的一帧数据
				// 比如发送到USART2或者设置标志位供主程序处理
				// 配置4G模块 AD
				if (USART_RX_BUF[0] == 0x41 && USART_RX_BUF[1] == 0x44)
				{
					// USART_OUT(USART1, USART_RX_BUF, sizeof(USART_RX_BUF)); // 直接将完整数据回显
				}
				// 获取设备ID AA
				else if (USART_RX_BUF[0] == 0x41 && USART_RX_BUF[1] == 0x41)
				{
					for (int i = 0; i < DeviceIDLength; i++)
						DeviceID[i] = USART_RX_BUF[2 + i];
					// USART_OUT(USART1, USART_RX_BUF, sizeof(USART_RX_BUF)); // 直接将完整数据回显
				}
				// 发送时间 AE
				else if (USART_RX_BUF[0] == 0x41 && USART_RX_BUF[1] == 0x45)
				{
					SendTime = (USART_RX_BUF[2] - '0') * 10 + (USART_RX_BUF[3] - '0');
					// USART_OUT(USART1, USART_RX_BUF, sizeof(USART_RX_BUF)); // 直接将完整数据回显
				}
				USART_OUT(USART1, USART_RX_BUF, sizeof(USART_RX_BUF)); // 直接将完整数据回显
				USART_RX_STA = 0;									   // 清空接收状态
			}
		}
		else // 缓冲区溢出，重置
		{
			USART_RX_STA = 0;
		}
	}
}

// 发送单个字符
void USART_SendChar(USART_TypeDef *USARTx, char c)
{
	USART_SendData(USARTx, (uint16_t)c);
	while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET)
		;
}

// 逐字符发送字符串
void USART_SendString(USART_TypeDef *USARTx, const char *str)
{
	while (*str)
	{
		USART_SendChar(USARTx, *str);
		str++;
	}
}

void USART_SendUint(USART_TypeDef *USARTx, uint8_t *str)
{
	while (*str)
	{
		USART_SendData(USARTx, (uint16_t)*str);
		while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET)
			;
		str++;
	}
}
void USART_OUT(USART_TypeDef *USARTx, uint8_t *buf, uint8_t len)
{
	uint8_t i;
	for (i = 0; i < len; i++)
	{
		USART_SendData(USARTx, (uint8_t)buf[i]);
		while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET)
			;
	}
}
// 将uint16_t数字转换为字符串并发送
void USART_SendNumber(USART_TypeDef *USARTx, uint16_t num)
{
	char buffer[6]; // 最大65535，5位数字 + \0
	int i = 0;

	// 特殊情况0
	if (num == 0)
	{
		// USART_SendData(USARTx, '0');
		// while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);
		USART_SendChar(USARTx, '0');
		return;
	}

	// 反向填充字符串
	while (num > 0)
	{
		buffer[i++] = (num % 10) + '0';
		num /= 10;
	}

	// 正向发送
	while (i--)
	{
		// USART_SendData(USARTx, buffer[i]);
		// while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);
		USART_SendChar(USARTx, buffer[i]);
	}
}
