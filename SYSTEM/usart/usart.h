#ifndef __USART_H
#define __USART_H
#include "stdio.h"
// #include "sys.h"
#define USART_REC_LEN 200 // �����������ֽ��� 200
#define EN_USART1_RX 1	  // ʹ�ܣ�1��/��ֹ��0������1����
#define DeviceIDLength 10 // �豸ID����

extern u8 USART_RX_BUF[USART_REC_LEN]; // ���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з�
extern u8 USART_RX_STA;				   // ����״̬���
extern u8 DeviceID[DeviceIDLength];	   // �豸ID
extern u8 SendTime;					   // ���ͱ���ʱ����

// ����봮���жϽ��գ��벻Ҫע�����º궨��
void uart_init(u32 bound);
void uart2_init(u32 bound);

// ���ߺ���
void USART_SendString(USART_TypeDef *USARTx, const char *str);
void USART_SendChar(USART_TypeDef *USARTx, char c);
void USART_SendUint(USART_TypeDef *USARTx, uint8_t *str);
void USART_OUT(USART_TypeDef *USARTx, uint8_t *buf, uint8_t len);
void USART_SendNumber(USART_TypeDef *USARTx, uint16_t num);
#endif
