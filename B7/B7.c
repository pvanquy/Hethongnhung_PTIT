#include "stm32f10x.h"

void PWM_Config(void);

int main(void) {
    PWM_Config();  // Cau hinh PWM

    while (1) {
        // Chuong trinh chinh khong lam gi, PWM duoc dieu khien boi TIM2
    }
}

void PWM_Config(void) {
    // 1. Bat clock cho GPIOA va TIM2
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); // GPIO
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);  // TIM2

    // 2. Cau hinh GPIOA cho chan TIM2_CH1 (PA0)
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;            // PA0 - TIM2_CH1
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;    // Toc do GPIO
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;      // Chuc nang thay the (PWM)
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    // 3. Cau hinh TIM2
    TIM_TimeBaseInitTypeDef TIM_BaseStruct;
    TIM_BaseStruct.TIM_Prescaler = 71;               // Chia tan so he thong (72MHz / (71 + 1) = 1MHz)
    TIM_BaseStruct.TIM_CounterMode = TIM_CounterMode_Up; // Dem len
    TIM_BaseStruct.TIM_Period = 999;                 // Tan so PWM = 1MHz / (999 + 1) = 1kHz
    TIM_BaseStruct.TIM_ClockDivision = TIM_CKD_DIV1; // Khong chia clock
    TIM_BaseStruct.TIM_RepetitionCounter = 0;        // Khong su dung lap lai
    TIM_TimeBaseInit(TIM2, &TIM_BaseStruct);

    // 4. Cau hinh Output Compare (OC) cho TIM2_CH1
    TIM_OCInitTypeDef TIM_OCStruct;
    TIM_OCStruct.TIM_OCMode = TIM_OCMode_PWM1;       // Che do PWM1
    TIM_OCStruct.TIM_OutputState = TIM_OutputState_Enable; // Bat dau PWM
    TIM_OCStruct.TIM_Pulse = 300;                   // Do rong xung = (300 / (999 + 1)) * 100% = 30%
    TIM_OCStruct.TIM_OCPolarity = TIM_OCPolarity_High; // C cuc cao khi dem < Pulse
    TIM_OC1Init(TIM2, &TIM_OCStruct);

    // 5. Bat TIM2
    TIM_Cmd(TIM2, ENABLE);

    // 6. Bat xuat PWM tren TIM2_CH1
    TIM_CtrlPWMOutputs(TIM2, ENABLE);
}
