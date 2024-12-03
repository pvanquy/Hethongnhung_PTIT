#include "stm32f10x.h"
#include <stdio.h>

// Prototype cac ham
void UART_Config(void);
void I2C_Config(void);
void I2C_Write(uint8_t deviceAddr, uint8_t data);
uint16_t I2C_ReadLight(uint8_t deviceAddr);
void sendValueUART(uint16_t value);

int main(void) {
    // Cau hinh UART va I2C
    UART_Config();
    I2C_Config();

    // Cau hinh cam bien BH1750 (che do do lien tuc, do phan giai cao)
    I2C_Write(0x23, 0x10); // 0x10: Che do lien tuc voi do phan giai cao (1 lux)

    while (1) {
        // Doc gia tri anh sang tu BH1750
        uint16_t lightValue = I2C_ReadLight(0x23);

        // Gui gia tri anh sang qua UART
        sendValueUART(lightValue);

        // Do tre
        for (int i = 0; i < 500000; i++);
    }
}

void UART_Config(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;            // TX (PA9)
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;      // Chan chuc nang thay the (TX)
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;           // RX (PA10)
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    USART_InitTypeDef USART_InitStruct;
    USART_InitStruct.USART_BaudRate = 9600;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART1, &USART_InitStruct);

    USART_Cmd(USART1, ENABLE);
}

void I2C_Config(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; // SCL (PB6) va SDA (PB7)
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_OD;       // Open-drain
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    I2C_InitTypeDef I2C_InitStruct;
    I2C_InitStruct.I2C_ClockSpeed = 100000;           // 100 kHz
    I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStruct.I2C_OwnAddress1 = 0x00;
    I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(I2C1, &I2C_InitStruct);

    I2C_Cmd(I2C1, ENABLE);
}

void I2C_Write(uint8_t deviceAddr, uint8_t data) {
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));

    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

    I2C_Send7bitAddress(I2C1, deviceAddr << 1, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

    I2C_SendData(I2C1, data);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    I2C_GenerateSTOP(I2C1, ENABLE);
}

uint16_t I2C_ReadLight(uint8_t deviceAddr) {
    uint16_t lightValue = 0;

    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));

    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

    I2C_Send7bitAddress(I2C1, deviceAddr << 1, I2C_Direction_Receiver);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));

    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
    lightValue = I2C_ReceiveData(I2C1) << 8; // Byte cao

    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
    lightValue |= I2C_ReceiveData(I2C1); // Byte thap

    I2C_GenerateSTOP(I2C1, ENABLE);

    return lightValue;
}

void sendValueUART(uint16_t value) {
    char buffer[20];
    sprintf(buffer, "Lux: %u\n", value);

    for (int i = 0; buffer[i] != '\0'; i++) {
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART_SendData(USART1, buffer[i]);
    }
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
}
