#include "stm32f10x.h"                  // Cho STM32F10x

void PWM_Config(void) {
    GPIO_InitTypeDef GPIO_;
    TIM_TimeBaseInitTypeDef TIM_;
    TIM_OCInitTypeDef TIM_OCInitStruct;

    // Bat clock cho GPIOA va TIM1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_TIM1, ENABLE);

    // Cau hinh PA8 lam ngo ra PWM
    GPIO_.GPIO_Pin = GPIO_Pin_8;
    GPIO_.GPIO_Mode = GPIO_Mode_AF_PP; 
    GPIO_.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_);

    // Cau hinh thong so TIM1 de tao tan so PWM 1kHz
    TIM_.TIM_Prescaler = (SystemCoreClock / 1000000) - 1; // Dua tan so dem ve 1MHz
    TIM_.TIM_Period = 1000 - 1;                           // 1ms chu ki (1000 buoc)
    TIM_.TIM_ClockDivision = 0;
    TIM_.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM1, &TIM_);

    // Cau hinh che do PWM
    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStruct.TIM_Pulse = TIM_.TIM_Period * 0.3;   // Ðo rong xung = 30% chu ki
    TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC1Init(TIM1, &TIM_OCInitStruct);

    // Kich hoat TIM1 va dau ra PWM
    TIM_Cmd(TIM1, ENABLE);
    TIM_CtrlPWMOutputs(TIM1, ENABLE);
}

int main(void) {
    PWM_Config();

    while (1) {
        // Vong lap chinh
    }
}