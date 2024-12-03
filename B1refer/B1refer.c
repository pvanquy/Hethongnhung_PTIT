#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"
#include "misc.h"

void GPIO_Config(void);
void Timer_Init(void);

volatile uint8_t led_state = 0;  // trang thai 0

void GPIO_Config(void) {
    GPIO_InitTypeDef GPIO_;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_.GPIO_Pin = GPIO_Pin_0;
    GPIO_.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_);
}

void Timer_Init(void) {
    TIM_TimeBaseInitTypeDef timer_init;
    NVIC_InitTypeDef NVICc;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    timer_init.TIM_CounterMode = TIM_CounterMode_Up;
    timer_init.TIM_Period = (SystemCoreClock / 2000)  - 1; 
    timer_init.TIM_Prescaler = 2000-1;
    TIM_TimeBaseInit(TIM2, &timer_init);

    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    //  NVIC cho Timer2
    NVICc.NVIC_IRQChannel = TIM2_IRQn;
    NVICc.NVIC_IRQChannelPreemptionPriority = 0;
    NVICc.NVIC_IRQChannelSubPriority = 1;
    NVICc.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVICc);

    // Timer2
    TIM_Cmd(TIM2, ENABLE);
}

void TIM2_IRQHandler(void) {
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        if (led_state) {
            GPIO_ResetBits(GPIOA, GPIO_Pin_0);
        } else {
            GPIO_SetBits(GPIOA, GPIO_Pin_0);
        }
        led_state = !led_state;  
    }
}

int main(void) {
    GPIO_Config();
    Timer_Init();

    while (1) {
  
    }
}