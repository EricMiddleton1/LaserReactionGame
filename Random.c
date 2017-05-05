/*
 * Random.c
 *
 *  Created on: May 5, 2017
 *      Author: ericm
 */

#include "Random.h"

#include <stdint.h>
#include <stdbool.h>

#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h"
#include "driverlib/timer.h"

#define DEFAULT_SKIP    (4)

static int __prev;
static int __max;
static int __skip;

void Random_init(int max, int initialState) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);

    TimerConfigure(TIMER2_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PERIODIC);
    TimerPrescaleSet(TIMER2_BASE, TIMER_A, 1);
    TimerLoadSet(TIMER2_BASE, TIMER_A, max - 1);
    TimerEnable(TIMER2_BASE, TIMER_A);

    __prev = initialState;
    __max = max;
    __skip = DEFAULT_SKIP % max;
    if(__skip == 0) {
        __skip = 1;
    }
}

int Random_getNext() {
    int next = TimerValueGet(TIMER2_BASE, TIMER_A);

    if(next == __prev) {
        next = (next + __skip) % __max;
    }

    __prev = next;

    return next;
}
