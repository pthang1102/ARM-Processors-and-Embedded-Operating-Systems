/*
 * lab03-ex2.cpp
 *
 *  Created on: 4 Sep 2020
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
#include <stdlib.h> //rand
#include <time.h>

// Create a queue capable of containing 20 int values.
QueueHandle_t queue2 = 	xQueueCreate(20, sizeof(int));
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
	int number;
	int delay;
	while (1) {
		number = rand() % 200; // 0-199
		delay = rand() % 9 + 2; // 2-10
		xQueueSendToBack( queue2, &number, portMAX_DELAY );
		vTaskDelay(configTICK_RATE_HZ / delay); // 0.1s - 0.5s
	}
}

static void Task2(void *pvParameter) {
	int emergencyNumber = 112;
	while (1) {
		if (sw1_button.read() == false) {
			xQueueSendToFront( queue2, &emergencyNumber, portMAX_DELAY );
			while (!sw1_button.read()){};
		}
	}
}

static void Task3(void *pvParameter) {
	int buffer;
	while (1) {
		xQueueReceive( queue2, &buffer, portMAX_DELAY );
		mutex.print("\r\n%d", buffer);
		if (buffer != 112) {
			//mutex.printInt(buffer);
		} else {
			mutex.print(" Help me!!!");
			vTaskDelay(configTICK_RATE_HZ / 3.3); //300ms
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
	srand (time(NULL));

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









