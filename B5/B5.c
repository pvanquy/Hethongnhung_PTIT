#include "stdio.h"
#include "stm32f10x.h"  // CMSIS va thu vien Standard Peripheral

void UART_Config(void);
void ADC_Config(void);
void sendValueUART(uint16_t value);

int main(void) {
    UART_Config();  // Cau hinh UART
    ADC_Config();   // Cau hinh ADC

    while (1) {
        // 1. Bat dau chuyen doi ADC
        ADC_SoftwareStartConvCmd(ADC1, ENABLE);

        // 2. Cho chuyen doi hoan tat
        while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);

        // 3. Doc gia tri ADC
        uint16_t adcValue = ADC_GetConversionValue(ADC1);

        // 4. Truyen gia tri ADC qua UART
        sendValueUART(adcValue);

        // 5. Them thoi gian tre neu can
        for (int i = 0; i < 500000; i++);
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

    // 4. Bat UART
    USART_Cmd(USART1, ENABLE);
}

void ADC_Config(void) {
    // 1. Bat Clock cho GPIO va ADC
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE);

    // 2. Cau hinh GPIO cho ADC (PA0)
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;            // ADC1_IN0 (PA0)
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;        // Chan dau vao analog
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    // 3. Cau hinh ADC
    ADC_InitTypeDef ADC_InitStruct;
    ADC_InitStruct.ADC_Mode = ADC_Mode_Independent;                // ADC doc lap
    ADC_InitStruct.ADC_ScanConvMode = DISABLE;                     // Che do don kenh
    ADC_InitStruct.ADC_ContinuousConvMode = DISABLE;               // Khong lien tuc
    ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // Khong kich ngoai
    ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;            // Can phai
    ADC_InitStruct.ADC_NbrOfChannel = 1;                           // 1 kenh ADC
    ADC_Init(ADC1, &ADC_InitStruct);

    // 4. Cau hinh kenh va thu tu chuyen doi
    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5); // Kenh 0

    // 5. Bat ADC
    ADC_Cmd(ADC1, ENABLE);

    // 6. Hieu chinh ADC
    ADC_ResetCalibration(ADC1);  // Reset hieu chinh
    while (ADC_GetResetCalibrationStatus(ADC1));  // Cho hieu chinh hoan tat

    ADC_StartCalibration(ADC1);  // Bat dau hieu chinh
    while (ADC_GetCalibrationStatus(ADC1));  // Cho hieu chinh hoan tat
}

void sendValueUART(uint16_t value) {
    char buffer[10];
    sprintf(buffer, "%u\n", value);  // Chuyen gia tri ADC sang chuoi

    for (int i = 0; buffer[i] != '\0'; i++) {
        // Gui tung ky tu qua UART
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET); // Cho san sang truyen
        USART_SendData(USART1, buffer[i]);  // Gui ky tu
    }
}
// Baud rate: 9600.
// Data bits: 8.
// Stop bits: 1.
// Parity: None.
