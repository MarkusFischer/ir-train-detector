//
// Created by markus on 2/6/23.
//
#include <msp430.h>
#include <msp430hal/cpu/clock_module.h>
#include <msp430hal/timer/hwtimer.h>
#include <msp430hal/timer/watchdog_timer.h>
#include <msp430hal/peripherals/comparator.h>
#include <msp430hal/gpio/pin.h>
//#include <usci/usci.h>

#include "interrupts.h"
#include "flags.h"

volatile std::uint_fast8_t g_comparator_counter = 0;
volatile bool g_comparator_capture_cycle_finished = false;

typedef msp430hal::timer::Timer_t<msp430hal::timer::TimerModule::timer_a, 1> ir_module_timer;
typedef msp430hal::timer::Timer_t<msp430hal::timer::TimerModule::timer_a, 0> timer_1mhz;
typedef msp430hal::gpio::GPIOPins<msp430hal::gpio::Port::port_2, msp430hal::gpio::Pin::p_0> module_1_status;
typedef msp430hal::gpio::GPIOPins<msp430hal::gpio::Port::port_3, msp430hal::gpio::Pin::p_4, msp430hal::gpio::Mode::input, msp430hal::gpio::PinResistors::external_pullup> button;


int main()
{

    msp430hal::timer::stopWatchdog();

    //TODO: validate calibration data
    msp430hal::cpu::setCalibratedFrequency<msp430hal::cpu::CalibratedDCOFrequency::calibrated_8MHz>();
    msp430hal::cpu::setInputDivider<msp430hal::cpu::Clock::aclk>(msp430hal::cpu::Divider::times_1);
    msp430hal::cpu::setLowFrequencySource(msp430hal::cpu::LowFrequencySource::watch_crystal);
    msp430hal::cpu::selectOscillatorCapacitor(msp430hal::cpu::OscillatorCapacitor::approx_6pF);
    msp430hal::cpu::selectClockSource<msp430hal::cpu::Clock::smclk>(msp430hal::cpu::ClockSource::dcoclk);
    msp430hal::cpu::setInputDivider<msp430hal::cpu::Clock::smclk>(msp430hal::cpu::Divider::times_8);


    module_1_status::init();
    button::init();



    //TODO: Replace with more readable gpio methods
    P2DIR |= BIT3 + BIT1;
    P2SEL |= BIT1;

    P2OUT &= ~BIT3;

    P1DIR |= BIT3;
    P1SEL |= BIT3;
    P1SEL2 |= BIT3;

    ir_module_timer::init(msp430hal::timer::TimerMode::up, msp430hal::timer::TimerClockSource::smclk, msp430hal::timer::TimerClockInputDivider::times_1);
    ir_module_timer::setCompareValue<0>(100);
    ir_module_timer::setCompareValue<1>(50);
    ir_module_timer::setCompareValue<2>(50);
    ir_module_timer::setOutputMode<1>(msp430hal::timer::TimerOutputMode::set_reset);
    ir_module_timer::setOutputMode<2>(msp430hal::timer::TimerOutputMode::reset_set);
    ir_module_timer::reset();

    timer_1mhz::init(msp430hal::timer::TimerMode::continuous, msp430hal::timer::TimerClockSource::smclk, msp430hal::timer::TimerClockInputDivider::times_1);
    timer_1mhz::captureMode<1>();
    timer_1mhz::setCaptureMode<1>(msp430hal::timer::TimerCaptureMode::rising_edge);
    timer_1mhz::selectCaptureCompareInput<1>(msp430hal::timer::CaptureCompareInputSelect::gnd);
    timer_1mhz::enableCaptureCompareInterrupt<1>();

    msp430hal::peripherals::Comparator comparator;

    comparator.setNonInvertingInput(msp430hal::peripherals::ComparatorInput::vcc_025);
    comparator.setInvertingInput(msp430hal::peripherals::ComparatorInput::ca_5);
    comparator.enableOutputFilter();
    comparator.enableInterrupt();
    comparator.enable();

    __enable_interrupt();

    for(;;)
    {
        if (g_comparator_capture_cycle_finished)
        {
            //Set status led if measured frequency is around 1kHz (+- 5%)
            std::uint16_t capture_value = timer_1mhz::getCaptureValue<1>();
            if (capture_value >= 950 && capture_value <= 1050)
                module_1_status::set();
            else
                module_1_status::clear();


            //reset counter
            g_comparator_counter = 0;
            g_comparator_capture_cycle_finished = false;
            timer_1mhz::selectCaptureCompareInput<1>(msp430hal::timer::CaptureCompareInputSelect::gnd);
            timer_1mhz::reset();
        }
    }
    return 0;
}
