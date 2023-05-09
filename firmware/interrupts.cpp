#include "interrupts.h"

#include <msp430hal/gpio/pin.h>
#include <msp430hal/timer/hwtimer.h>
#include "flags.h"


void handleTimer0A1Interrupts()
{
    typedef msp430hal::timer::Timer_t<msp430hal::timer::timer_a, 0> timer;


    if (*timer::tiv == 0x02) //comparator counted 10 rising edges -> update flag and let main program handle the remaining tasks
        g_comparator_capture_cycle_finished = true;
}

void handleComparatorInterrupt()
{
    g_comparator_counter++;

    if (g_comparator_counter >= 10)
        msp430hal::timer::Timer_t<msp430hal::timer::timer_a, 0>::selectCaptureCompareInput<1>(msp430hal::timer::CaptureCompareInputSelect::vcc);
}