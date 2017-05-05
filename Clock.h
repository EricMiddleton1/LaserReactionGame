/*
 * Clock.h
 *
 *  Created on: May 5, 2017
 *      Author: eric
 */

#ifndef CLOCK_H_
#define CLOCK_H_

#include <stdint.h>

void Clock_init();

void Clock_start();
void Clock_stop();

uint32_t Clock_getMillis();
uint32_t Clock_getMicros();

void Clock_waitMicros(uint32_t micros);


#endif /* CLOCK_H_ */
