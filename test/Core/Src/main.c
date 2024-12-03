#include "cmsis_os2.h"  // Thu vi?n CMSIS-RTOS
#include "stm32f10x.h"  // Thu vi?n STM32F103

// Prototype h�m task
void task1(void *argument);
void task2(void *argument);


// Task 1: Nh�y LED m?i 500ms
void task1(void *argument) {
    while (1) {
        GPIOC->ODR ^= GPIO_ODR_ODR13;  // �?i tr?ng th�i ch�n GPIO (Toggle LED)
        osDelay(500);                  // Tr? 500ms
    }
}

// Task 2: Ch?y v�ng l?p don gi?n
void task2(void *argument) {
    while (1) {
        // Th?c hi?n m?t c�ng vi?c kh�c, c� th? in ra UART ho?c di?u khi?n thi?t b? ngo?i vi
        osDelay(1000);                 // Tr? 1 gi�y
    }
}

int main(void) {
    // Kh?i t?o h? th?ng v� c?u h�nh clock cho GPIO


    // B?t Clock cho Port C (s? d?ng ch�n PC13)
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;

    // C?u h�nh PC13 l� output (cho LED)
    GPIOC->CRH &= ~GPIO_CRH_MODE13;
    GPIOC->CRH |= GPIO_CRH_MODE13_1;  // PC13 output t?c d? 2 MHz
    GPIOC->CRH &= ~GPIO_CRH_CNF13;    // Output push-pull

    // Kh?i t?o CMSIS-RTOS
    osKernelInitialize();

    // T?o task 1
    osThreadNew(task1, NULL, NULL);

    // T?o task 2
    osThreadNew(task2, NULL, NULL);

    // Kh?i d?ng h? di?u h�nh (RTOS Scheduler)
    osKernelStart();

    // V�ng l?p ch�nh s? kh�ng bao gi? t?i d�y v� kernel d� qu?n l� c�c task
    while (1) {
    }
}