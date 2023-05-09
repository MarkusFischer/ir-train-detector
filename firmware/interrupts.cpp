#include "interrupts.h"

#include <gpio/pin.h>
#include <timer/hwtimer.h>
#include "flags.h"


void handleTimer0A1Interrupts()
{
    typedef msp430hal::timer::Timer_t<msp430hal::timer::timer_a, 0> timer;

    //
    //msp430hal::gpio::GPIOPins<msp430hal::gpio::Port::port_2, msp430hal::gpio::Pin::p_0>::toggle();


    if (*timer::tiv == 0x02)
        g_comparator_capture_cycle_finished = true;
    if (*timer::tiv == 0x0a)
    {
        //comparator counted 50 rising edges -> update flag and let main program handle the remaining tasks

    }
}

void handleComparatorInterrupt()
{
    //msp430hal::gpio::GPIOPins<msp430hal::gpio::Port::port_2, msp430hal::gpio::Pin::p_0>::toggle();
    g_comparator_counter++;

    if (g_comparator_counter >= 10)
    {
        msp430hal::timer::Timer_t<msp430hal::timer::timer_a, 0>::selectCaptureCompareInput<1>(msp430hal::timer::CaptureCompareInputSelect::vcc);
    }
}