#include "stm32f10x.h"  // CMSIS va STDPeriph Library

void UART_Config(void);
void GPIO_Config(void);
void USART1_IRQHandler(void);  // ISR de xu ly ngat UART

int main(void) {
    UART_Config();  // Cau hinh UART
    GPIO_Config();  // Cau hinh GPIO

    while (1) {
        // Main loop khong can xu ly nhieu, moi logic trong ngat
    }
}

void UART_Config(void) {
    // 1. Bat Clock cho GPIO va USART
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

    // 2. Cau hinh GPIO cho UART TX (PA9) va RX (PA10)
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;            // TX (PA9)
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;    // Toc do GPIO
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;      // Chan chuc nang thay the (TX)
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;           // RX (PA10)
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING; // Chan dau vao (RX)
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    // 3. Cau hinh UART
    USART_InitTypeDef USART_InitStruct;
    USART_InitStruct.USART_BaudRate = 9600;                     // Baud rate
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;    // 8-bit data
    USART_InitStruct.USART_StopBits = USART_StopBits_1;         // 1 stop bit
    USART_InitStruct.USART_Parity = USART_Parity_No;            // Khong parity
    USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx; // TX va RX
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // Khong flow control
    USART_Init(USART1, &USART_InitStruct);

    // 4. Kich hoat ngat UART
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);  // Bat ngat nhan du lieu (RX)
    NVIC_EnableIRQ(USART1_IRQn);                   // Enable ngat trong NVIC

    // 5. Bat UART
    USART_Cmd(USART1, ENABLE);
}

void GPIO_Config(void) {
    // 1. Bat Clock cho GPIO
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);  // Gia su dung chan PC13

    // 2. Cau hinh GPIO lam dau ra
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;            // Chon chan PC13
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;     // Toc do GPIO
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;      // Che do dau ra Push-Pull
    GPIO_Init(GPIOC, &GPIO_InitStruct);
}

// ISR xu ly ngat UART
void USART1_IRQHandler(void) {
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {  // Neu co du lieu nhan
        char received = USART_ReceiveData(USART1);  // Doc du lieu tu UART

        // Dieu khien GPIO dua tren ky tu nhan duoc
        if (received == 'A') {
            GPIO_SetBits(GPIOC, GPIO_Pin_13);  // Keo chan PC13 len muc cao
        } else if (received == 'B') {
            GPIO_ResetBits(GPIOC, GPIO_Pin_13);  // Keo chan PC13 xuong muc thap
        }
    }
}
// Baud rate: 9600.
// Data bits: 8.
// Stop bits: 1.
// Parity: None.
