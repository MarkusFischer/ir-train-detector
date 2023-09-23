#include "interrupts.h"

#include <msp430hal/gpio/pin.h>
#include <msp430hal/timer/hwtimer.h>
#include "flags.h"
#include "constants.h"

void handleTimer0A0Interrupt()
{
    typedef msp430hal::timer::Timer_t<msp430hal::timer::timer_a, 0> timer;

    g_capture_cycle_finished = true;

    //Stop the timer
    timer::setCompareValue<0>(0);
    __low_power_mode_off_on_exit();
}

void handleComparatorInterrupt()
{
    g_comparator_counter++;
}

void handleUSCIRXInterrupt()
{
    g_uart_message_received = true;
    g_rx_buffer.queue(UCA0RXBUF);
    __low_power_mode_off_on_exit();
}
