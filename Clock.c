/*
 * Clock.c
 *
 *  Created on: May 5, 2017
 *      Author: eric
 */

#include "Clock.h"

#include <stdbool.h>
#include "driverlib/timer.h"
#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h"

static uint32_t __millis;

static void __intHandler();

void Clock_init() {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

    TimerConfigure(TIMER0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PERIODIC);
    TimerPrescaleSet(TIMER0_BASE, TIMER_A, 50);
    TimerLoadSet(TIMER0_BASE, TIMER_A, 999);

    TimerIntRegister(TIMER0_BASE, TIMER_A, __intHandler);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
}

void Clock_start() {
    __millis = 0;

    TimerEnable(TIMER0_BASE, TIMER_A);
}
void Clock_stop() {
    TimerDisable(TIMER0_BASE, TIMER_A);
}

void Clock_restart() {
    Clock_stop();
    Clock_start();
}

uint32_t Clock_getMillis() {
    TimerIntDisable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    uint32_t copyTime = __millis;
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    return copyTime;
}

uint32_t Clock_getMicros() {
    TimerIntDisable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    uint32_t copyTime = 1000*__millis + (999 - TimerValueGet(TIMER0_BASE, TIMER_A));
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    return copyTime;
}

void Clock_waitMicros(uint32_t micros) {
    uint32_t endTime = Clock_getMicros() + micros;

    while(Clock_getMicros() < endTime);
}

void __intHandler() {
    uint32_t intStatus = TimerIntStatus(TIMER0_BASE, true);
    TimerIntClear(TIMER0_BASE, intStatus);

    __millis++;
}


