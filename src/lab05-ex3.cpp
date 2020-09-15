/*
 * lab05-ex3.cpp
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
#include "ITM_write.h"

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
void Task1(void *pvParameter) {
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
				Board_LED_Set(0, true);
				vTaskDelay(1000);
				Board_LED_Set(1, false);
			} else if (lmsw2.read()) {
				Board_LED_Set(1, true);
				vTaskDelay(1000);
				Board_LED_Set(1, false);
			} else {
				Board_LED_Set(0, false);
				Board_LED_Set(1, false);
			}
		}
		else {
			if (lmsw1.read() || lmsw2.read()) {
				Board_LED_Set(2, state);
				state = !state;
				//Board_LED_Toggle(2);
				vTaskDelay(100);
			} else {
				Board_LED_Set(2, false);
				run = true;
				xSemaphoreGive(binary);
			}
		}
	}
}
// Waits on the binary semaphore. When it gets the semaphore "go" signal, task continues.
// Run stepper motor in either direction, and run the motor at constant speed until one limit switch is closed.
// Then motor direction is toggled.
// Program COUNT the number of steps between limit switches.
// Multiple run is done here (5 times) -> average number.
// When average_step is known, Blue led ON for 2s.
// Then, run motor back and forth so that it touches limit-switches lightly but does not close them!
void Task2(void *pvParameter) {
	DigitalIoPin lmsw1(0, 9, DigitalIoPin::pullup, true);
	DigitalIoPin lmsw2(0, 29, DigitalIoPin::pullup, true);
	DigitalIoPin direct(1, 0, DigitalIoPin::output, true);
	DigitalIoPin step(0, 24, DigitalIoPin::output, true);
	bool direction = false;
	int runCount = 0;
	int stepCount = 0;
	int stepCountTotal = 0;
	int averageStep;

	xSemaphoreTake(binary, portMAX_DELAY);

	while (runCount <= 5) { // Test 5 times
		if ((lmsw1.read() && direction) || (lmsw2.read() && !direction)) {
			runCount++;
			//The 1st time is not counted because the motor starts from the middle of the screen.
			if (runCount > 1) {
				stepCountTotal += stepCount;
				ITM_print("Steps between 2 limits: %d steps\r\n", stepCount);
			}
			direction = !direction;
			stepCount = 0;
		}
		direct.write(direction);
		step.write(true);
		vTaskDelay(1);
		step.write(false);
		vTaskDelay(1);
		stepCount++;
	}

	averageStep = stepCountTotal / 5; // Average steps
	ITM_print("Average steps: %d steps\r\n", averageStep); // Print average steps
	Board_LED_Set(2, true);
	vTaskDelay(2000); // Blue led ON for 2s
	Board_LED_Set(2, false);
	int stepReduce = 1; // First, try to reduce step by 1 on each side
	direction = false;

	while (1) {
		direct.write(direction);
		step.write(false);
		vTaskDelay(1);
		step.write(true);
		vTaskDelay(1);
		stepCount++;

		if (stepCount == (averageStep - stepReduce)) {
			ITM_print("Count: %d steps\r\n", (stepCount-stepReduce));
			ITM_print("Step reduced: %d steps\r\n\n", (stepReduce*2)); // because it reduce step from 2 sides
			direction = !direction;
			stepCount = stepReduce;
		}

		if (lmsw1.read() || lmsw2.read()) {
			ITM_print("Limit hit!!\r\n1 more step reduced!\r\n\n");
			stepReduce++; // Increase Step-reduce
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

	xTaskCreate(Task1, "Read limit switches", configMINIMAL_STACK_SIZE + 64, NULL,
			(tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
	xTaskCreate(Task2, "Control stepper motor", configMINIMAL_STACK_SIZE + 64, NULL,
			(tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
	vTaskStartScheduler();

	return 0;
}
