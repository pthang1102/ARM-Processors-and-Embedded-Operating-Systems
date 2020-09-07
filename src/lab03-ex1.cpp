/*
 * lab03-ex1.cpp
 *
 *  Created on: 3 Sep 2020
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
#include "queue.h"

// Create a queue capable of containing 5 int values.
QueueHandle_t queue1 = 	xQueueCreate(5, sizeof(int));
DigitalIoPin sw1_button(0, 17, DigitalIoPin::pullup);
Syslog mutex = Syslog();

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
	int counter = 0;
	while (1) {
		int ch = Board_UARTGetChar();
		if (ch != EOF) {
			if ((ch != '\n') && (ch != '\r')) {
				mutex.write(&ch);
				counter++;
			} else {
				xQueueSendToBack( queue1, &counter, portMAX_DELAY );
				counter = 0;
			}
		}
	}
}

static void Task2(void *pvParameter) {
	int minusOne = -1;
	while (1) {
		if (sw1_button.read() == false) {
			xQueueSendToBack( queue1, &minusOne, portMAX_DELAY );
			while (!sw1_button.read()){};
		}
	}
}

static void Task3(void *pvParameter) {
	int buffer;
	int sum = 0;
	while (1) {
		xQueueReceive( queue1, &buffer, portMAX_DELAY );
		if (buffer != -1) {
			sum = sum + buffer;
		} else {
			mutex.print("\r\nYou have typed %d characters\r\n", sum);
			sum = 0;
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
	//binary = xSemaphoreCreateBinary();

	xTaskCreate(Task1, "Task1",
					configMINIMAL_STACK_SIZE + 64, NULL, (tskIDLE_PRIORITY + 1UL),
					(TaskHandle_t *) NULL);
	xTaskCreate(Task2, "Task2",
						configMINIMAL_STACK_SIZE + 64, NULL, (tskIDLE_PRIORITY + 1UL),
						(TaskHandle_t *) NULL);
	xTaskCreate(Task3, "Task3",
						configMINIMAL_STACK_SIZE + 64, NULL, (tskIDLE_PRIORITY + 1UL),
						(TaskHandle_t *) NULL);

	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}









