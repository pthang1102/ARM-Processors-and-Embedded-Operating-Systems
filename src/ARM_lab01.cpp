/*
===============================================================================
 Name        : main.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
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

/* Sets up system hardware */
DigitalIoPin button(0, 17, DigitalIoPin::input, true);
static bool greenOn = false;

static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	/* Initial LED0 state is off */
	Board_LED_Set(0, false);
}

/* LED1 toggle thread */
static void vLEDTask1(void *pvParameters) { //EXERCISE 2
	while (1) {
		bool LedState = true;
		static int counter = 0;

		while (1) {
			if (!greenOn) {
				Board_LED_Set(0, LedState);
				LedState = (bool) !LedState;

				if (counter == 6 || counter == 8 || counter == 10)
					vTaskDelay(configTICK_RATE_HZ);
				else
					vTaskDelay(configTICK_RATE_HZ / 3);

				counter++;

				if (counter == 18) {
					counter = 0;
					LedState = true;
				}
			} else {
				Board_LED_Set(0, false);
			}
		}
	}
}

/* LED2 toggle thread */
static void vLEDTask2(void *pvParameters) { // EXERCISE 2
	while (1) {
		while (1) {
			Board_LED_Set(1, greenOn);
			vTaskDelay(configTICK_RATE_HZ / 3 * 24);
			greenOn = !greenOn;
		}
	}
}

/* UART (or output) thread */
static void vUARTTask(void *pvParameters) {
	int tickCnt = 0;
	int second;
	int minute;

	// EXERCISE 3
	while (1) {
		if (button.read() == true) {
			tickCnt += 10;
		} else {
			tickCnt++;
		}
		minute = tickCnt / 60;
		second = tickCnt - minute*60;
		// EXERCISE 1
		DEBUGOUT("Time: %02d:%02d \r\n", minute, second);

		/* About a 1s delay here */
		vTaskDelay(configTICK_RATE_HZ);
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

	/* LED1 toggle thread */
	xTaskCreate(vLEDTask1, "vTaskLed1",
				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
				(TaskHandle_t *) NULL);

	/* LED2 toggle thread */
	xTaskCreate(vLEDTask2, "vTaskLed2",
				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
				(TaskHandle_t *) NULL);

	/* UART output thread, simply counts seconds */
	xTaskCreate(vUARTTask, "vTaskUart",
				configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
				(TaskHandle_t *) NULL);

	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}

