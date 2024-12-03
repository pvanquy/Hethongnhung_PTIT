#include "main.h"

//task
void vTaskReadBH1750(void *pvParameters);
void vTaskState(void *pvParameters);
void vTaskPWMControl(void *pvParameters);
void vTaskGPIOControl (void *pvParameters);
void vTaskSendUART(void *pvParameters);

int main( void){
systemclock_Config();
UART1_Config();
I2C2_Config();
GPIO_Config();
PWM_Config();
BH1750_Start();
BH1750_Read();
	
	luxQueue = xQueueCreate(10, sizeof(uint16_t));
	luxUART = xQueueCreate(10, sizeof(uint16_t));
	pwmDutyQueue = xQueueCreate(10, sizeof(uint16_t));
	pwmUART = xQueueCreate(10, sizeof(uint16_t));
	gpioQueue = xQueueCreate(10, sizeof(uint16_t));
	
	xTaskCreate(vTaskReadBH1750, "ReadBH1750", 128, NULL, 1, NULL);//task1: doc gia tri cam bien luu vao ham doi
	xTaskCreate(vTaskState, "State", 128, NULL, 1, NULL);//task2: kiem tra trang thai lux >> gpio >>pwmDuty
	xTaskCreate(vTaskPWMControl, "PWMControl", 128, NULL, 1, NULL);//task 3: dieu che pwm
	xTaskCreate(vTaskGPIOControl, "GPIOControl", 128, NULL, 1, NULL);//task 4: dieu khien GPIO
	xTaskCreate(vTaskSendUART, "SendUART", 128, NULL, 1, NULL);//task 5: gui gia tri len UART

	vTaskStartScheduler();
	while(1){
		}
}
//task1: doc gia tri cam bien luu vao ham doi
void vTaskReadBH1750(void *pvParameters){
	uint16_t lux = 0;
    while (1) {
        lux = BH1750_Read();
			if(lux > 0 ){
        xQueueSend(luxQueue, &lux,  portMAX_DELAY);
			  xQueueSend(luxUART, &lux,  portMAX_DELAY);
				}
        	vTaskDelay(pdMS_TO_TICKS(100)); 
    }
}
//task2: kiem tra trang thai lux >> gpio >>pwmDuty
void vTaskState(void *pvParameters){
	  uint16_t lux ;
    uint8_t gpioState = 0;
    uint16_t pwmDuty = 0;
    while (1) {
        if (xQueueReceive(luxQueue, &lux, portMAX_DELAY)) {
            if (lux >= 1000 && lux <= 20000) {
                pwmDuty = (uint16_t)(99 - (89 * (lux - 1000) / 19000.0));
                gpioState = 0;  // GPIO Low
            } else if (lux > 20000) {
                pwmDuty = 0;
                gpioState = 1;  // GPIO High
            }
						else if (lux < 1000) {
                pwmDuty = 100;
                gpioState = 0;  // GPIO LOW1
            }
            xQueueSend( gpioQueue, &gpioState,  portMAX_DELAY);
            xQueueSend( pwmDutyQueue, &pwmDuty,  portMAX_DELAY);
						xQueueSend( pwmUART, &pwmDuty,  portMAX_DELAY);
						vTaskDelay(pdMS_TO_TICKS(100)); 
        }
    }
}
//task 3: dieu che pwm
void vTaskPWMControl(void *pvParameters) {
	 uint16_t pwmDuty;
    while (1) {
			if (xQueueReceive(pwmDutyQueue, &pwmDuty, portMAX_DELAY)) {
			TIM_SetCompare2(TIM4, pwmDuty * 10);  // Update PWM duty cycle
			}	
      vTaskDelay(pdMS_TO_TICKS(100)); 
    }
}
//task 4: dieu khien GPIO
void vTaskGPIOControl (void *pvParameters) {
    uint8_t gpioState;
    while (1) {
        if (xQueueReceive(gpioQueue, &gpioState, portMAX_DELAY)) {
            if (gpioState) {
                GPIO_SetBits(GPIOC, GPIO_PIN_13);  // GPIO High
            } else {
                GPIO_ResetBits(GPIOC, GPIO_PIN_13);  // GPIO Low
            }
        }
				 vTaskDelay(pdMS_TO_TICKS(100)); 
    }
}
//task 5: gui gia tri len UART
void vTaskSendUART(void *pvParameters) {
    uint16_t lux;
	  uint16_t pwmDuty;
	  char buffer[50];
    while (1) {
        if (xQueueReceive(luxUART, &lux, portMAX_DELAY) && xQueueReceive(pwmUART, &pwmDuty, portMAX_DELAY)) {
          
          snprintf(buffer,sizeof(buffer), "Lux: %u lx ==>", lux);
          UART1_SendString(buffer);// gui gia tri cam bien
					snprintf(buffer,sizeof(buffer), "Do rong: %u\n\r", pwmDuty*10);
					UART1_SendString(buffer);// gui do rong xung
        }
        vTaskDelay(pdMS_TO_TICKS(100)); 
    }
}