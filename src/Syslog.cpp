/*
 * Syslog.cpp
 *
 *  Created on: 1 Sep 2020
 *      Author: Thang Tran
 */

#include "Syslog.h"
#include <stdarg.h>

Syslog::Syslog() {
	// TODO Auto-generated constructor stub
	this->mutex = xSemaphoreCreateMutex();
}

Syslog::~Syslog() {
	// TODO Auto-generated destructor stub
	vSemaphoreDelete(mutex);
}

void Syslog::write(int *description) {
	if (mutex != NULL) {
		if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
			Board_UARTPutChar(*description);
			xSemaphoreGive(mutex);
		}
	}
}

void Syslog::writeStr(const char *description) {
	if (mutex != NULL) {
		if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
			Board_UARTPutSTR(description);
			xSemaphoreGive(mutex);
		}
	}
}

int Syslog::read(){
	if(mutex != NULL){
		if(xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE){
			int ch = Board_UARTGetChar();
			xSemaphoreGive(mutex);
			return ch;
		}
	}
}

void Syslog::writeStr(const std::string description){
	writeStr(description.c_str());
}

void Syslog::print(const char *format, ...) {
	if (mutex != NULL) {
		if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
			char buffer [256];
			va_list args;
			va_start(args, format);
			vsnprintf(buffer, 256, format, args);
			va_end(args);
			Board_UARTPutSTR(buffer);
			xSemaphoreGive(mutex);
		}
	}
}

/*
void Syslog::printInt(int description) {
	char str[6];
	sprintf(str, "\r\n%d", description);
	if (mutex != NULL) {
		if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
			Board_UARTPutSTR(str);
			xSemaphoreGive(mutex);
		}
	}
}
*/


