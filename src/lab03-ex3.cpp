/*
 * lab03-ex3.cpp
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
#include "heap_lock_monitor.h"
#include "Syslog.h"
#include "queue.h"
#include "ITM_write.h"
#include <ctype.h>
#include <stdio.h>

struct debugEvent {
	const char *format;
	uint32_t data[3];
};

// Create a queue capable of containing 10 debugEvent objects.
QueueHandle_t queue = 	xQueueCreate(10, sizeof(debugEvent));
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

// Create a debugEvent object and send it to Queue
void sendToQueue(const char *format, uint32_t d1, uint32_t d2, uint32_t d3) {
	debugEvent sendToQueue = {
			format,
			{d1, d2, d3},
	};
	xQueueSendToBack( queue, &sendToQueue, portMAX_DELAY );
}

// Read SerialPort, Echoe back to Debug Serial Port.
// When whiteSpace, call sendToQueue() to send info to Queue.
static void readSerialPort(void *pvParameter) {
	int counter = 0;
	while (1) {
		//int ch = Board_UARTGetChar();
		int ch = mutex.read();

		if (ch != EOF) {
			if (!isspace(ch)) {
				counter++;
			} else if (isspace(ch)) {
				if (counter > 0) {
					sendToQueue("Received %d characters at %d tick\n", counter, xTaskGetTickCount(), 0);
					counter = 0;
				}
			}
			mutex.write(&ch);
			if (ch == '\n' || ch == '\r'){ // Break-line
				mutex.writeStr("\r\n");
			}
		} else {
			vTaskDelay(2);
		}
	}
}

// Monitor SW1, call sendtoQueue() when SW1 is pressed and released.
// Length of press is sent to Queue.
static void readSW1(void *pvParameter) {
	int pressTime, releaseTime, time;
	while (1) {
		if (sw1_button.read() == false) {
			pressTime = xTaskGetTickCount();
			while (!sw1_button.read()){};
			releaseTime = xTaskGetTickCount();
			time = releaseTime - pressTime;
			sendToQueue("SW1 was held for %d ticks\n", time, 0, 0);
		}
		vTaskDelay(2);
	}
}

// Wait on Queue. Receive items from queue and print them to ITM debug console.
void receiveQueueAndPrint(void *pvParameters) {
	char buffer[64];
	debugEvent e;
	// this is not complete! how do we know which queue to wait on?

	while (1) {
	// read queue
		xQueueReceive(queue, &e, portMAX_DELAY);
		snprintf(buffer, 64, e.format, e.data[0], e.data[1], e.data[2]);
		ITM_write(buffer);
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
	ITM_init();


	xTaskCreate(readSerialPort, "readSerialPort",
					configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
					(TaskHandle_t *) NULL);
	xTaskCreate(readSW1, "readSW1",
						configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
						(TaskHandle_t *) NULL);
	xTaskCreate(receiveQueueAndPrint, "receiveQueueAndPrint",
						configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
						(TaskHandle_t *) NULL);

	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}
