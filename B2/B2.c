#include "stm32f10x.h"

// Khai bao LED va Button GPIO
#define LED_PIN      GPIO_Pin_13
#define LED_PORT     GPIOC

#define BUTTON_PIN   GPIO_Pin_0
#define BUTTON_PORT  GPIOA

void GPIO_Config(void);
void NVIC_Config(void);
void EXTI_Config(void);

volatile uint8_t ledState = 0; // Trang thai LED

int main(void) {
    // Cau hinh GPIO va NVIC
    GPIO_Config();
    NVIC_Config();
    EXTI_Config();

    while (1) {
        // Main loop: CPU se nhan roi, doi su kien ngat
    }
}

// Cau hinh GPIO cho LED va nut bam
void GPIO_Config(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOA, ENABLE);

    // Cau hinh LED (Output)
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = LED_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LED_PORT, &GPIO_InitStruct);

    // Cau hinh nut bam (Input voi Pull-Up)
    GPIO_InitStruct.GPIO_Pin = BUTTON_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU; // Input Pull-Up
    GPIO_Init(BUTTON_PORT, &GPIO_InitStruct);
}

// Cau hinh NVIC cho ngat EXTI
void NVIC_Config(void) {
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn; // Ngat EXTI0
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0; // Uu tien cao
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

// Cau hinh EXTI cho nut bam
void EXTI_Config(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    // Ket noi GPIOA Pin0 voi EXTI Line0
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);

    EXTI_InitTypeDef EXTI_InitStruct;
    EXTI_InitStruct.EXTI_Line = EXTI_Line0;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling; // Kich hoat khi nhan nut (muc thap)
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);
}

// Trinh xu ly ngat EXTI0 (nut bam)
void EXTI0_IRQHandler(void) {
    if (EXTI_GetITStatus(EXTI_Line0) != RESET) {
        // Dao trang thai LED
        ledState = !ledState;
        if (ledState) {
            GPIO_SetBits(LED_PORT, LED_PIN); // Bat LED
        } else {
            GPIO_ResetBits(LED_PORT, LED_PIN); // Tat LED
        }

        // Xoa co ngat
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}
