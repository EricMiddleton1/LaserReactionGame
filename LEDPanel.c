/*
 * LEDPanel.c
 *
 *  Created on: May 5, 2017
 *      Author: eric
 */

#include "LEDPanel.h"

#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h"
#include "driverlib/timer.h"
#include "driverlib/adc.h"
#include "driverlib/gpio.h"

#define BIT0    0x01
#define BIT1    0x02
#define BIT2    0x04
#define BIT3    0x08
#define BIT4    0x10
#define BIT5    0x20
#define BIT6    0x40
#define BIT7    0x80

#define DETECT_THRESHOLD     (1000)

const static uint32_t LED_BASE[] =
    {GPIO_PORTE_BASE, GPIO_PORTE_BASE, GPIO_PORTE_BASE, GPIO_PORTE_BASE, GPIO_PORTD_BASE,
     GPIO_PORTD_BASE, GPIO_PORTD_BASE, GPIO_PORTD_BASE, GPIO_PORTE_BASE, GPIO_PORTE_BASE};
const static uint8_t LED_PIN[] =
    {BIT3, BIT2, BIT1, BIT0, BIT3,
     BIT2, BIT1, BIT0, BIT5, BIT4};
const static uint32_t ADC_CH[] =
    {ADC_CTL_CH0, ADC_CTL_CH1, ADC_CTL_CH2, ADC_CTL_CH3, ADC_CTL_CH4,
     ADC_CTL_CH5, ADC_CTL_CH6, ADC_CTL_CH7, ADC_CTL_CH8, ADC_CTL_CH9};

static uint8_t __channel;
int __detect;
static enum {
    OFF = 0,
    ON
} __ledState = ON;

static void __intHandler();

void LEDPanel_init() {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);

    //Configure ADC
    ADCHardwareOversampleConfigure(ADC0_BASE, 4);
    ADCSequenceDisable(ADC0_BASE, 3);
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_ALWAYS, 0);
    ADCSequenceEnable(ADC0_BASE, 3);

    //Configure timer
    TimerConfigure(TIMER1_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PERIODIC);
    TimerPrescaleSet(TIMER1_BASE, TIMER_A, 50);
    TimerLoadSet(TIMER1_BASE, TIMER_A, 999);
    TimerIntRegister(TIMER1_BASE, TIMER_A, __intHandler);
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    LEDPanel_setChannel(0);

    TimerEnable(TIMER1_BASE, TIMER_A);
}

void LEDPanel_setChannel(uint8_t channel) {
    TimerIntDisable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    //Turn off previous channel
    GPIOPinTypeGPIOOutput(LED_BASE[__channel], LED_PIN[__channel]);
    GPIOPinWrite(LED_BASE[__channel], LED_PIN[__channel], 0);

    //Turn on new channel
    GPIOPinTypeGPIOOutput(LED_BASE[channel], LED_PIN[channel]);
    GPIOPinWrite(LED_BASE[channel], LED_PIN[channel], LED_PIN[channel]);

    //Switch ADC to use this channel
    ADCSequenceDisable(ADC0_BASE, 3);
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CH[channel]);
    ADCSequenceEnable(ADC0_BASE, 3);

    __channel = channel;
    __ledState = ON;
    __detect = 0;

    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    Clock_waitMicros(1000);
    __detect = 0;
}

int LEDPanel_detect() {
    int detectCopy = __detect;
    __detect = 0;

    return detectCopy;
}

void __intHandler() {
    uint32_t intStatus = TimerIntStatus(TIMER1_BASE, true);
    TimerIntClear(TIMER1_BASE, intStatus);

    if(__ledState == ON) {
        //Turn off LED
        GPIOPinWrite(LED_BASE[__channel], LED_PIN[__channel], 0);
        //Reconfigure pin as analog input
        GPIOPinTypeADC(LED_BASE[__channel], LED_PIN[__channel]);

        //Trigger ADC
        //ADCProcessorTrigger(ADC0_BASE, 3);
    }
    else {
        //Read ADC value
        uint32_t value;
        int32_t count = ADCSequenceDataGet(ADC0_BASE, 3, &value);
        if(count == 1 && value > DETECT_THRESHOLD) {
            __detect = value;
        }

        //Turn LED back on
        GPIOPinTypeGPIOOutput(LED_BASE[__channel], LED_PIN[__channel]);
        GPIOPinWrite(LED_BASE[__channel], LED_PIN[__channel], LED_PIN[__channel]);
    }

    __ledState = !__ledState;
}
