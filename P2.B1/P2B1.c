#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <stdio.h>
#include <string.h>
#include "stm32f10x_flash.h"
#include "stm32f10x_rcc.h"

/* Define queue handles */
xQueueHandle adcQueuePA0;
xQueueHandle adcQueuePA1;
xQueueHandle pwmFrequencyy;


xQueueHandle pwmDutyCyclee;

/* Function prototypes */
void systemclock_config(void);
void UART1_Init(void);
void ADC1_Init(void);
void PWM_Init(void);
uint16_t ADC1_Read(uint8_t channel);
void UART1_SendString(char* str);

void vTaskReadADC_PA0(void *pvParameters);
void vTaskReadADC_PA1(void *pvParameters);
void vTaskSendUART(void *pvParameters);
void vTaskPWMControl(void *pvParameters);

int main(void) {
    systemclock_config();
    UART1_Init();
    ADC1_Init();
    PWM_Init();

    adcQueuePA0 = xQueueCreate(10, sizeof(uint16_t));
    adcQueuePA1 = xQueueCreate(10, sizeof(uint16_t));
    pwmFrequencyy = xQueueCreate(10, sizeof(uint16_t));
    pwmDutyCyclee = xQueueCreate(10, sizeof(uint16_t));
 
    xTaskCreate(vTaskReadADC_PA0, "ADC PA0 Read", 128, NULL, 1, NULL);
    xTaskCreate(vTaskReadADC_PA1, "ADC PA1 Read", 128, NULL, 1, NULL);
    xTaskCreate(vTaskSendUART, "UART Send", 128, NULL, 1, NULL);
    xTaskCreate(vTaskPWMControl, "PWM Control", 128, NULL, 1, NULL);

    vTaskStartScheduler();
    while (1);
}

void vTaskReadADC_PA0(void *pvParameters) {
    uint16_t adcValue;
    while (1) {
        adcValue = ADC1_Read(ADC_Channel_1);  // PA0 is connected to ADC Channel 0
        xQueueSend(adcQueuePA0, &adcValue, portMAX_DELAY);  
        xQueueSend(pwmDutyCyclee, &adcValue, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(500));  
    }
}

void vTaskReadADC_PA1(void *pvParameters) {
    uint16_t adcValue;
    while (1) {
        adcValue = ADC1_Read(ADC_Channel_0);  // PA1 is connected to ADC Channel 1
        xQueueSend(adcQueuePA1, &adcValue, portMAX_DELAY);
        xQueueSend(pwmFrequencyy, &adcValue, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(500)); 
    }
}

void vTaskPWMControl(void *pvParameters) {
    uint16_t adcValuePA0 = 0;
    uint16_t adcValuePA1 = 0;
    uint32_t pwmFrequency = 0;
    uint16_t pwmDutyCycle = 0;
    while (1) {
        if (xQueueReceive(pwmDutyCyclee, &adcValuePA0, 0) == pdTRUE) {
            pwmDutyCycle = (adcValuePA0 * 80) / 4095 + 10; 
            TIM4->CCR2 = (TIM4->ARR + 1) * pwmDutyCycle / 100;
        }

        if (xQueueReceive(pwmFrequencyy, &adcValuePA1, 0) == pdTRUE) {
            pwmFrequency = 1000 + ((adcValuePA1 * 9000) / 4095); 
            TIM4->ARR = (72000000 / (72 * pwmFrequency)) - 1;
        }
        vTaskDelay(pdMS_TO_TICKS(100)); 
    }
}

void vTaskSendUART(void *pvParameters) {
    uint16_t adcValuePA0;
    uint16_t adcValuePA1;
    char buffer[40];

    while (1) {
        if (xQueueReceive(adcQueuePA0, &adcValuePA0, portMAX_DELAY) == pdTRUE) {
            snprintf(buffer, sizeof(buffer), "PA0: %u ==> ", adcValuePA0);
            UART1_SendString(buffer);
            snprintf(buffer, sizeof(buffer), "Width: %u\r\n", ((adcValuePA0 * 80) / 4095 + 10));
            UART1_SendString(buffer);
        }
        if (xQueueReceive(adcQueuePA1, &adcValuePA1, portMAX_DELAY) == pdTRUE) {
            snprintf(buffer, sizeof(buffer), "PA1: %u ==> ", adcValuePA1);
            UART1_SendString(buffer);
            snprintf(buffer, sizeof(buffer), "Freq: %u\r\n", (1000 + ((adcValuePA1 * 9000) / 4095)));
            UART1_SendString(buffer);
            UART1_SendString("  \r\n");
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void systemclock_config(void) {
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY));
    
    RCC->CFGR &= ~(RCC_CFGR_SW);
    RCC->CFGR |= RCC_CFGR_SW_HSE;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSE);
    
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY));

    RCC->CFGR &= ~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLMULL);
    RCC->CFGR |= (RCC_CFGR_PLLSRC_HSE | RCC_CFGR_PLLMULL9);

    RCC->CFGR &= ~(RCC_CFGR_SW);
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);

    RCC->CFGR &= ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2);
    RCC->CFGR |= (RCC_CFGR_HPRE_DIV1 | RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_PPRE2_DIV1);
}

void UART1_Init(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO | RCC_APB2Periph_USART1, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);
}

void ADC1_Init(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    ADC_InitTypeDef ADC_InitStructure;
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 2;
    ADC_Init(ADC1, &ADC_InitStructure);
    
    ADC_Cmd(ADC1, ENABLE);

    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1));
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

void PWM_Init(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    TIM_TimeBaseStructure.TIM_Prescaler = 72 - 1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period = 1000 - 1;  
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 500;  
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC2Init(TIM4, &TIM_OCInitStructure);

    TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM4, ENABLE);

    TIM_Cmd(TIM4, ENABLE);
}

uint16_t ADC1_Read(uint8_t channel) {
    ADC_RegularChannelConfig(ADC1, channel, 1, ADC_SampleTime_55Cycles5);
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
    return ADC_GetConversionValue(ADC1);
}

void UART1_SendString(char* str) {
    while (*str) {
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART_SendData(USART1, *str++);
    }
}