#include <stdbool.h>
#include <stdint.h>

#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"
#include "driverlib/adc.h"
#include "driverlib/rom_map.h"

#include "Clock.h"
#include "LEDPanel.h"

#define LED_COUNT   (2)

#define LED_PIN     GPIO_PIN_3
#define LED_PORT    GPIO_PORTE_BASE

#define UART_BASE   GPIO_PORTA_BASE
#define UART_PINS   (GPIO_PIN_0 | GPIO_PIN_1)

#define UART_BAUD   (921600)//(115200)

char *itoa (int value, char *result, int base);
static void UART_puts(const char* str);

static uint16_t ADC_read();

int main()
{
    SysCtlClockSet(SYSCTL_SYSDIV_4|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN); //50Mhz
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //Enable UART
    GPIOPinTypeUART(UART_BASE, UART_PINS);
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), UART_BAUD,
        UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE);

    //Clock
    Clock_init();
    Clock_start();

    //LED Panel
    int led = 0;
    LEDPanel_init();
    LEDPanel_setChannel(led);

    for (;;) {
        int value;
        if((value = LEDPanel_detect()) > 0) {
            char buffer[64];
            UART_puts("Input detected: ");
            UART_puts(itoa(value, buffer, 10));
            UART_puts("\r\n");

            led = (led + 1) % LED_COUNT;
            LEDPanel_setChannel(led);

            //Clock_waitMicros(100000);

            //LEDPanel_detect();

/*
            //Flush the detector
            uint32_t endTime = Clock_getMicros() + 1000;
            while(Clock_getMicros() < endTime) {
                LEDPanel_detect();
            }
*/
        }
    }

    return 0;
}

void UART_puts(const char* str) {
    char c;

    while( (c = *(str++)) != '\0') {
        UARTCharPut(UART0_BASE, c);
    }
}

uint16_t ADC_read() {
    //Trigger single reading
    ADCProcessorTrigger(ADC0_BASE, 3);

    //Read the data
    uint32_t value;

//    while(!ADCIntStatus(ADC0_BASE, 3, false));
    int32_t count;
    while((count = ADCSequenceDataGet(ADC0_BASE, 3, &value)) < 1);

    return value;
}

char *
itoa (int value, char *result, int base)
{
    // check that the base if valid
    if (base < 2 || base > 36) { *result = '\0'; return result; }

    char* ptr = result, *ptr1 = result, tmp_char;
    int tmp_value;

    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
    } while ( value );

    // Apply negative sign
    if (tmp_value < 0) *ptr++ = '-';
    *ptr-- = '\0';
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }
    return result;
}
