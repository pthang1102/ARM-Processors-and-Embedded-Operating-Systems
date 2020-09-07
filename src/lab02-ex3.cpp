/*
 * lab02-ex3.cpp
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
#include <string.h>

#define SECOND configTICK_RATE_HZ
Syslog mutex = Syslog();
SemaphoreHandle_t counting;

static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	/* Initial LED0 is off */
	Board_LED_Set(0, false);
}

void Task1(void *pvParameter) {
	int index = 0;
	int maxLength = 60;
	char buffer[maxLength] = "\0";
	bool questionMark = false;

	while (1) {
		int ch = mutex.read();
		if ((ch != EOF) && (index < maxLength) && (ch != '\n') && (ch != '\r')) {
			if (index == 0) {
				mutex.writeStr("         "); // Leave some space for [You]
			}
			if (ch == '?') {
				questionMark = true;
			}
			mutex.write(&ch);
			buffer[index] = ch;
			buffer[index+1] = '\0';
			index++;
		}
		else if ((index >= maxLength) || (ch == '\n') || (ch == '\r')) {
			mutex.writeStr("\r[You]    ");
			mutex.writeStr(buffer);
			mutex.writeStr("\r\n");
			index = 0;
			if (questionMark) {
				xSemaphoreGive(counting);
			}
			questionMark = false; // Reset questionMark state for new conversation

			for (int i=maxLength; i>=0; i--) { // Erase the buffer
				buffer[i] = '\0';
			}
		}
	}
}

void Task2(void *pvParameter)
{
	char *responses[10] {
		"I finally realized that people are prisoners of their phones... that's why it's called a cell phone.\r\n",
		"People say you can't live without love, but I think oxygen is more important.\r\n",
		"Never take life seriously. Nobody gets out alive anyway.\r\n",
		"If I won the award for laziness, I would send somebody to pick it up for me.\r\n",
		"Doing nothing is hard, you never know when you're done.\r\n",
		"Who says nothing is impossible? I've been doing nothing for years.\r\n",
		"If we shouldn't eat at night, why is there a light in the fridge?\r\n",
		"If you think nothing is impossible, try slamming a revolving door.\r\n",
		"Yesterday I did nothing and today I'm finishing what I did yesterday.\r\n",
		"The difference between stupidity and genius is that genius has its limits."};
	while (1) {
		if (xSemaphoreTake(counting, portMAX_DELAY) == pdTRUE) {
			mutex.writeStr("\r\n[Oracle] Hmmm...\r\n");
			vTaskDelay(SECOND * 3);
			mutex.writeStr("\r\n[Oracle] ");
			mutex.writeStr(responses[(xTaskGetTickCount() % 10)]);
			vTaskDelay(SECOND * 2);
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
	counting = xSemaphoreCreateCounting(10, 0);

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





