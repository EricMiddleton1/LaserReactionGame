/*
 * LEDPanel.h
 *
 *  Created on: May 5, 2017
 *      Author: eric
 */

#ifndef LEDPANEL_H_
#define LEDPANEL_H_

#include <stdint.h>
#include <stdbool.h>

void LEDPanel_init();

void LEDPanel_setChannel(uint8_t channel);

int LEDPanel_detect();


#endif /* LEDPANEL_H_ */
