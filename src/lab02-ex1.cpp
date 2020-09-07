/*
 * lab02-ex1.cpp
 *
 *  Created on: 31 Aug 2020
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
DigitalIoPin sw1_button(0, 17, DigitalIoPin::pullup);
DigitalIoPin sw2_button(1, 11, DigitalIoPin::pullup);
DigitalIoPin sw3_button(1, 9, DigitalIoPin::pullup);

Syslog mutex = Syslog();

static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	/* Initial LED0 state is off */
	Board_LED_Set(0, false);
}

static void sw1Task(void *pvParameter) {
	while (1) {
		if (sw1_button.read() == false) {
		mutex.writeStr("SW1 pressed \r\n");
		while(!sw1_button.read()){};
		}
	}
}

static void sw2Task(void *pvParameter) {
	while (1) {
		if (sw2_button.read() == false) {
		mutex.writeStr("SW2 pressed \r\n");
		while(!sw2_button.read()){};
		}
	}
}

static void sw3Task(void *pvParameter) {
	while (1) {
		if (sw3_button.read() == false) {
		mutex.writeStr("SW3 pressed \r\n");
		while(!sw3_button.read()){};
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

	xTaskCreate(sw1Task, "sw1Task",
					configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
					(TaskHandle_t *) NULL);
	xTaskCreate(sw2Task, "sw2Task",
					configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
					(TaskHandle_t *) NULL);
	xTaskCreate(sw3Task, "sw3Task",
					configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
					(TaskHandle_t *) NULL);
	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}





