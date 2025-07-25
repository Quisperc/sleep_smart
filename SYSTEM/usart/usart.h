#ifndef __USART_H
#define __USART_H
#include "stdio.h"
// #include "sys.h"
#define USART_REC_LEN 200 // 定义最大接收字节数 200
#define EN_USART1_RX 1	  // 使能（1）/禁止（0）串口1接收
#define DeviceIDLength 10 // 设备ID长度

extern u8 USART_RX_BUF[USART_REC_LEN]; // 接收缓冲,最大USART_REC_LEN个字节.末字节为换行符
extern u8 USART_RX_STA;				   // 接收状态标记
extern u8 DeviceID[DeviceIDLength];	   // 设备ID
extern u8 SendTime;					   // 发送报告时间间隔

// 如果想串口中断接收，请不要注释以下宏定义
void uart_init(u32 bound);
void uart2_init(u32 bound);

// 工具函数
void USART_SendString(USART_TypeDef *USARTx, const char *str);
void USART_SendChar(USART_TypeDef *USARTx, char c);
void USART_SendUint(USART_TypeDef *USARTx, uint8_t *str);
void USART_OUT(USART_TypeDef *USARTx, uint8_t *buf, uint8_t len);
void USART_SendNumber(USART_TypeDef *USARTx, uint16_t num);
#endif
