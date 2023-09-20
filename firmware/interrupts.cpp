#include "interrupts.h"

#include <msp430hal/gpio/pin.h>
#include <msp430hal/timer/hwtimer.h>
#include "flags.h"
#include "constants.h"


void handleTimer0A1Interrupts()
{
    typedef msp430hal::timer::Timer_t<msp430hal::timer::timer_a, 0> timer;

    //comparator counted 100 rising edges -> update flag and let main program handle the remaining tasks
    if (*timer::tiv == 0x02)
    {
        g_capture_cycle_finished = true;
        g_comparator_cycle_timer_count = timer::getCaptureValue<1>();
    }
}

void handleTimer0A0Interrupt()
{
    typedef msp430hal::timer::Timer_t<msp430hal::timer::timer_a, 0> timer;

    g_capture_cycle_finished = true;

    //Stop the timer
    timer::setCompareValue<0>(0);
}

void handleComparatorInterrupt()
{
    g_comparator_counter++;

}

void handleUSCIRXInterrupt()
{
    g_uart_message_received = true;
    g_rx_buffer.queue(UCA0RXBUF);
}

void handleUSCITXInterrupt()
{
    g_uart_transmit_ready = true;
}