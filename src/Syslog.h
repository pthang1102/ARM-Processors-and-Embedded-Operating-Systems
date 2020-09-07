/*
 * Syslog.h
 *
 *  Created on: 1 Sep 2020
 *      Author: Thang Tran
 */

#ifndef SYSLOG_H_
#define SYSLOG_H_

#include <string>
#include "FreeRTOS.h"
#include "semphr.h"

class Syslog {
public:
	Syslog();
	virtual ~Syslog();
	void write(int *description);
	void writeStr(const char* description);
	int read();
	void writeStr(const std::string description);
	void print(const char *format, ...);
	//void printInt(int description);
private:
	SemaphoreHandle_t mutex;
};

#endif /* SYSLOG_H_ */
