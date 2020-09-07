/*
 * lab02-ex2.cpp
 *
 *  Created on: 1 Sep 2020
 *      Author: Thang Tran
 */

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include <cr_section_macros.h>
#include "FreeRTOS.h"
#include "task.h"
#include "DigitalIoPin.h"
#include "Syslog.h"

/* Sets up system hardware */
Syslog mutex = Syslog();
SemaphoreHandle_t binary;

static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	/* Initially, LED0, LED1, LED2 state is off */
	Board_LED_Set(0, false);
	Board_LED_Set(1, false);
	Board_LED_Set(2, false);
}

static void Task1(void *pvParameter) {
	while (1) {
		int ch = Board_UARTGetChar();
		if (ch != EOF) {
			mutex.write(&ch);
			xSemaphoreGive(binary);
		}
	}
}

static void Task2(void *pvParameter) {
	while (1) {
		if (xSemaphoreTake(binary, portMAX_DELAY) == pdPASS) {
			//FIRST, TURN ON THE LEDS
			//Board_LED_Set(0, true); // Red
			//Board_LED_Set(1, true); // Green
			Board_LED_Set(2, true); // Blue
			vTaskDelay(configTICK_RATE_HZ/10); //100ms

			//TURN OFF THE LEDS
			//Board_LED_Set(0, false);
			//Board_LED_Set(1, false);
			Board_LED_Set(2, false);
			vTaskDelay(configTICK_RATE_HZ/10); //100ms
		}
	}
}

extern "C" {

void vConfigureTimerForRunTimeStats( void ) {
	Chip_SCT_Init(LPC_SCTSMALL1);
	LPC_SCTSMALL1->CONFIG = SCT_CONFIG_32BIT_COUNTER;
	LPC_SCTSMALL1->CTRL_U = SCT_CTRL_PRE_L(255) | SCT_CTRL_CLRCTR_L; // set prescaler to 256 (255 + 1), and start timer
}

}

int main(void)
{
	prvSetupHardware();
	binary = xSemaphoreCreateBinary();

	xTaskCreate(Task1, "Task1",
					configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
					(TaskHandle_t *) NULL);
	xTaskCreate(Task2, "Task2",
						configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
						(TaskHandle_t *) NULL);

	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}





