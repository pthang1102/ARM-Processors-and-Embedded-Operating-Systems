/*
 * lab05-ex1.cpp
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

static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	/* Initial LED0 state is off */
	Board_LED_Set(0, false);
}

// When Limit-switch-1 is closed/open, set Red led on/off.
// When Limit-switch-2 is closed/open, set Green led on/off.
void Task1(void *pvParameter){
	DigitalIoPin lmsw1(0, 9, DigitalIoPin::pullup, true);
	DigitalIoPin lmsw2(0, 29, DigitalIoPin::pullup, true);
	while(1) {
		if (lmsw1.read()) {
			Board_LED_Set(0, true);
		} else {
			Board_LED_Set(0, false);
		}

		if (lmsw2.read()) {
			Board_LED_Set(1, true);
		} else {
			Board_LED_Set(1, false);
		}
	}
}

// Read LPCXpresso Button1 and Button3.
// While Button1 is pressed, stepper motor runs SLOWLY clockwise.
// While Button3 is pressed, stepper motor runs SLOWLY anti-clockwise.
// When the button is released, the motor stops.
// If Button1 and Button3 is pressed at the same time, the motor does not run.
// If either of the Limit-switches is closed, the motor does not run.
void Task2(void *pvParameter){
	DigitalIoPin b1(0, 8, DigitalIoPin::pullup, true);
	DigitalIoPin b3(1, 8, DigitalIoPin::pullup, true);
	DigitalIoPin lmsw1(0, 9, DigitalIoPin::pullup, true);
	DigitalIoPin lmsw2(0, 29, DigitalIoPin::pullup, true);

	DigitalIoPin direct(1, 0, DigitalIoPin::output, true);
	DigitalIoPin step(0, 24, DigitalIoPin::output, true);
	while(1) {
		 if ((b1.read() && b3.read()) || lmsw1.read() || lmsw2.read()) {
			 continue;
		 } else if (b1.read()) {
			 direct.write(false);
			 step.write(true);
			 vTaskDelay(1);
			 step.write(false);
			 vTaskDelay(1);
		 } else if (b3.read()) {
			 direct.write(true);
			 step.write(true);
			 vTaskDelay(1);
			 step.write(false);
			 vTaskDelay(1);
		 }
	}
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/* the following is required if runtime statistics are to be collected */
extern "C" {

void vConfigureTimerForRunTimeStats( void ) {
	Chip_SCT_Init(LPC_SCTSMALL1);
	LPC_SCTSMALL1->CONFIG = SCT_CONFIG_32BIT_COUNTER;
	LPC_SCTSMALL1->CTRL_U = SCT_CTRL_PRE_L(255) | SCT_CTRL_CLRCTR_L; // set prescaler to 256 (255 + 1), and start timer
}

}
/* end runtime statictics collection */


int main(void) {

	prvSetupHardware();
	xTaskCreate(Task1, "Led task", configMINIMAL_STACK_SIZE, NULL,
			(tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
	xTaskCreate(Task2, "Stepper Task", configMINIMAL_STACK_SIZE, NULL,
			(tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
	vTaskStartScheduler();

    return 0 ;
}
