#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stdio.h"

#define BH1750_POWER_DOWN 0x00     
#define BH1750_Addr_L 0x46
#define BH1750_ON      0x01
#define BH1750_RESET   0x07
#define BH1750_CONT_H_RES_MODE 0x10 
#define GPIO_PIN_13 ((uint16_t)0x2000) // Pin 13
//ham
void systemclock_Config(void);
void UART1_Config(void);
void UART_SendChar(char c);
void UART_SendString(char *str);
void I2C2_Config(void);
void BH1750_Cmd_Write(uint8_t cmd);
void BH1750_Start(void);
uint16_t BH1750_Read(void);
void GPIO_Config(void);
void PWM_Config(void);


//ham doi
xQueueHandle luxQueue;
xQueueHandle luxUART;
xQueueHandle pwmDutyQueue;
xQueueHandle gpioQueue;
xQueueHandle pwmUART;


void systemclock_Config(){
  RCC->CR |= RCC_CR_HSEON;
  while (!(RCC->CR & RCC_CR_HSERDY));
	
  RCC->CFGR &= ~(RCC_CFGR_SW);
  RCC->CFGR |= RCC_CFGR_SW_HSE;//cai nguon dong bo he thong la hse
  while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSE);//cho on dinh
  
  RCC->CR |= RCC_CR_PLLON;
  while (!(RCC->CR & RCC_CR_PLLRDY));//cho on dinh

  RCC->CFGR &= ~(RCC_CFGR_PLLSRC/*chon nguon clock*/ | RCC_CFGR_PLLXTPRE/*cau hinh gtri chia*/ | RCC_CFGR_PLLMULL);
  RCC->CFGR |= (RCC_CFGR_PLLSRC_HSE/*chon hse lam nguon cho pll*/ | RCC_CFGR_PLLMULL9/*bo nhan pll la 9*/);

  RCC->CFGR &= ~(RCC_CFGR_SW);
  RCC->CFGR |= RCC_CFGR_SW_PLL;
  while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);

  RCC->CFGR &= ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2);
  RCC->CFGR |= (RCC_CFGR_HPRE_DIV1/*APB chia 1*/ | RCC_CFGR_PPRE1_DIV2/*APB1 chia 2*/ | RCC_CFGR_PPRE2_DIV1/*APB2 CHIA 1*/);
}
// cau hinh UART
void UART1_Config(void){
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |RCC_APB2Periph_AFIO| RCC_APB2Periph_USART1, ENABLE);

	// PA9 - TX
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// PA10 -RX
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
	USART_Cmd(USART1, ENABLE);
}

void UART1_SendChar(char c){
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	USART_SendData(USART1, c);
}

void UART1_SendString(char* str){
	while (*str){
		UART1_SendChar(*str++);
	}
}
// cau hinh I2C2
void I2C2_Config(void) {
    I2C_InitTypeDef I2C_InitStruct;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);  
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO, ENABLE);
  
		GPIO_InitTypeDef GPIO_InitStructure;
	  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10| GPIO_Pin_11;//I2C2 PB10-SCL2; PB11-SDA2
	  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	  GPIO_Init(GPIOB, &GPIO_InitStructure);
   
    I2C_InitStruct.I2C_ClockSpeed = 100000;  // 100kHz
    I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStruct.I2C_OwnAddress1 = 0x00;
    I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(I2C2, &I2C_InitStruct);

    I2C_Cmd(I2C2, ENABLE);
}
// BH1750
typedef struct {
    uint8_t mode;
    uint16_t result_lx;
    uint32_t time; 
} BH1750_t;

BH1750_t BH1750 = { BH1750_CONT_H_RES_MODE, 0, 200 };

void BH1750_Cmd_Write(uint8_t cmd) {
    I2C_GenerateSTART(I2C2, ENABLE);
    while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT));

    I2C_Send7bitAddress(I2C2, BH1750_Addr_L, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

    I2C_SendData(I2C2, cmd);
    while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	
    I2C_GenerateSTOP(I2C2, ENABLE);

    for (uint32_t i = 0; i < 100000; i++);  
}

void BH1750_Start(void) {
    BH1750_Cmd_Write(BH1750_ON);    // Bat nguon
    BH1750_Cmd_Write(BH1750_RESET); // Reset cam bien
    BH1750_Cmd_Write(BH1750.mode);  // Mode
}


uint16_t BH1750_Read(void) {
    uint8_t data_re[2] = {0, 0};
    uint16_t lux = 0;

    // Start I2C
    I2C_GenerateSTART(I2C2, ENABLE);
    while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT));

    // Goi dia chi , chon che do nhan
    I2C_Send7bitAddress(I2C2, BH1750_Addr_L, I2C_Direction_Receiver);
    while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));

    // Nhan 2 byte du lieu
    for (int i = 0; i < 2; i++) {
        while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_RECEIVED));
        data_re[i] = I2C_ReceiveData(I2C2);
    }

    // Stop I2C
    I2C_GenerateSTOP(I2C2, ENABLE);

    // Chuyen du lien di
    lux = (data_re[0] << 8) | data_re[1];  // Gep 2 byte thanh 16 bit
    lux = lux / 1.2;

    for (int i = 0; i < 100000; i++);

    return lux;
}
// cau hinh PWM
void PWM_Config(void){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// 1KHz
	TIM_TimeBaseStructure.TIM_Prescaler = 72 - 1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = 1000 - 1;  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

	// Kenh 2
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 990;  
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC2Init(TIM4, &TIM_OCInitStructure);

	TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);
	TIM_ARRPreloadConfig(TIM4, ENABLE);

	TIM_Cmd(TIM4, ENABLE);
}
// cau hinh GPIO
void GPIO_Config(void){
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;// GPIO dieu khien
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}