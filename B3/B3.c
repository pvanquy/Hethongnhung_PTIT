#include "stm32f10x.h"                  // CMSIS và STDPeriph Library

void UART_Config(void);
void UART_SendString(USART_TypeDef* USARTx, const char* str);

int main(void) {
    UART_Config();  // C?u hình UART

    while (1) {
        UART_SendString(USART1, "Hello\r\n");  // Gui chuoi "Hello"
        for (volatile int i = 0; i < 1000000; i++);  // Delay don gi?n
    }
}

void UART_Config(void) {
    // 1. Bat Clock cho GPIO và USART
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

    // 2. Cau hình GPIO cho USART1 TX (PA9) và RX (PA10)
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;            // TX (PA9)
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;    // Toc do GPIO
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;      // Chan chuc nang thay the (TX)
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;           // RX (PA10)
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING; // Chan dau vao
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    // 3. Cau hinh USART
    USART_InitTypeDef USART_InitStruct;
    USART_InitStruct.USART_BaudRate = 9600;                     // Baud rate
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;    // 8-bit data
    USART_InitStruct.USART_StopBits = USART_StopBits_1;         // 1 stop bit
    USART_InitStruct.USART_Parity = USART_Parity_No;            // Khong parity
    USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx; // TX va RX
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // Khong Flow Control
    USART_Init(USART1, &USART_InitStruct);

    // 4. Kich hoat USART
    USART_Cmd(USART1, ENABLE);
}

void UART_SendString(USART_TypeDef* USARTx, const char* str) {
    while (*str) {
        // Gui tang ky tu
        USART_SendData(USARTx, *str++);
        while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);  // Cho TXE
    }
}