/*
 * lab05-ex2.cpp
 *
 *  Created on: 11 Sep 2020
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
#include "semphr.h"

SemaphoreHandle_t binary;
static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	/* Initial LED0 state is off */
	Board_LED_Set(0, false);
}

// Blue led blinks until both limit switches are open.
// Then signal "go" using binary semaphore.
// Then, enter a loop where it reads Limit-switches: red-led on if lmsw1 is closed
// and green-led on if lmsw2 is closed
void Task1(void *pvParameter)
{
	DigitalIoPin lmsw1(0, 9, DigitalIoPin::pullup, true);
	DigitalIoPin lmsw2(0, 29, DigitalIoPin::pullup, true);
	bool run = false;
	bool state = true;

	while (1) {
		if (run == true) {
			if (lmsw1.read() && lmsw2.read()) {
				Board_LED_Set(0, false);
				Board_LED_Set(1, false);
				run = false;
			} else if (lmsw1.read()) {
				Board_LED_Set(0, true); // Red led on
			} else if (lmsw2.read()) {
				Board_LED_Set(1, true); // Green led on
			} else {
				Board_LED_Set(0, false);
				Board_LED_Set(1, false);
			}
		}
		else {
			if (lmsw1.read() || lmsw2.read()) {
				Board_LED_Set(2, state); // Blue led blink
				state = !state;
				vTaskDelay(100);
			} else { //Both limit switches are open
				Board_LED_Set(2, false);
				run = true;
				xSemaphoreGive(binary); //Signal "go" using binary semaphore
			}
		}
	}
}

// Waits on the binary semaphore. When it gets the semaphore "go" signal, task continues.
// Run stepper motor in either direction, and run the motor at constant speed until one limit switch is closed.
// Then motor direction is toggled.
// If both limit switches are closed, motor is stopped immediately and task waits for 5s.
// Then the limit switches are checked: if they are both open, the motor runs again.
void Task2(void *pvParameter) {
	DigitalIoPin lmsw1(0, 9, DigitalIoPin::pullup, true);
	DigitalIoPin lmsw2(0, 29, DigitalIoPin::pullup, true);
	DigitalIoPin direct(1, 0, DigitalIoPin::output, true);
	DigitalIoPin step(0, 24, DigitalIoPin::output, true);
	bool direction = false;

	xSemaphoreTake(binary, portMAX_DELAY); //Get the semaphore

	while (1) {
		direct.write(direction);
		step.write(false);
		vTaskDelay(1);
		step.write(true);
		vTaskDelay(1);

		if (lmsw1.read() && lmsw2.read()) { // both limit switches are closed
			step.write(false);
			vTaskDelay(5000); // wait for 5 seconds
			continue;
		} else if (lmsw1.read() && direction == true) {
			direction = !direction; // motor direction toggled
		} else if (lmsw2.read() && direction == false) {
			direction = !direction; // motor direction toggled
		}
	}
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/* the following is required if runtime statistics are to be collected */
extern "C"
{

	void vConfigureTimerForRunTimeStats(void)
	{
		Chip_SCT_Init(LPC_SCTSMALL1);
		LPC_SCTSMALL1->CONFIG = SCT_CONFIG_32BIT_COUNTER;
		LPC_SCTSMALL1->CTRL_U = SCT_CTRL_PRE_L(255) | SCT_CTRL_CLRCTR_L; // set prescaler to 256 (255 + 1), and start timer
	}

}
/* end runtime statictics collection */

int main(void)
{

	prvSetupHardware();
	binary = xSemaphoreCreateBinary();

	xTaskCreate(Task1, "Read limit switches", configMINIMAL_STACK_SIZE, NULL,
			(tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
	xTaskCreate(Task2, "Control stepper motor", configMINIMAL_STACK_SIZE, NULL,
			(tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
	vTaskStartScheduler();

	return 0;
}
