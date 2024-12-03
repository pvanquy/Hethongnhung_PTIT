#include "stm32f10x.h"
#include "stm32f10x_gpio.h"             
#include "stm32f10x_rcc.h"              
#include "stm32f10x_tim.h"              



void GPIO_Config(void);
void TIM2_Config(void);
void TIM2_IRQHandler(void);

// Ham xu ly ngat cua Timer 2
void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        // Xaa co ngat
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

        // dao trang thai chan PA5
        GPIOA->ODR ^= GPIO_Pin_5; // XOR de toggle trang thai
    }
}

int main(void)
{
    // Cau hinh GPIO va Timer
    GPIO_Config();
    TIM2_Config();

    while (1)
    {
       
    }
}

void GPIO_Config(void)
{
    // Bat clock cho GPIOA
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    // Cau hinh PA5 lam Output Push-Pull
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void TIM2_Config(void)
{
    // Bat clock cho TIM2
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    // Cau hinh Timer
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_TimeBaseInitStruct.TIM_Prescaler = 7200 - 1; // T?n s? Timer = 72 MHz / 7200 = 10 kHz
    TIM_TimeBaseInitStruct.TIM_Period = 10000 - 1;   // Chu k? ng?t = 10 kHz / 10000 = 1 Hz (1 giây)
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);

    // Cho phep ngat Timer
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    // Kich hoat Timer
    TIM_Cmd(TIM2, ENABLE);

    // Cau hinh NVIC cho ngat Timer 2
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}