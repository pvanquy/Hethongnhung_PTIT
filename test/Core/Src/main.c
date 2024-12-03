#include "cmsis_os2.h"  // Thu vi?n CMSIS-RTOS
#include "stm32f10x.h"  // Thu vi?n STM32F103

// Prototype hàm task
void task1(void *argument);
void task2(void *argument);


// Task 1: Nháy LED m?i 500ms
void task1(void *argument) {
    while (1) {
        GPIOC->ODR ^= GPIO_ODR_ODR13;  // Ð?i tr?ng thái chân GPIO (Toggle LED)
        osDelay(500);                  // Tr? 500ms
    }
}

// Task 2: Ch?y vòng l?p don gi?n
void task2(void *argument) {
    while (1) {
        // Th?c hi?n m?t công vi?c khác, có th? in ra UART ho?c di?u khi?n thi?t b? ngo?i vi
        osDelay(1000);                 // Tr? 1 giây
    }
}

int main(void) {
    // Kh?i t?o h? th?ng và c?u hình clock cho GPIO


    // B?t Clock cho Port C (s? d?ng chân PC13)
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;

    // C?u hình PC13 là output (cho LED)
    GPIOC->CRH &= ~GPIO_CRH_MODE13;
    GPIOC->CRH |= GPIO_CRH_MODE13_1;  // PC13 output t?c d? 2 MHz
    GPIOC->CRH &= ~GPIO_CRH_CNF13;    // Output push-pull

    // Kh?i t?o CMSIS-RTOS
    osKernelInitialize();

    // T?o task 1
    osThreadNew(task1, NULL, NULL);

    // T?o task 2
    osThreadNew(task2, NULL, NULL);

    // Kh?i d?ng h? di?u hành (RTOS Scheduler)
    osKernelStart();

    // Vòng l?p chính s? không bao gi? t?i dây vì kernel dã qu?n lý các task
    while (1) {
    }
}