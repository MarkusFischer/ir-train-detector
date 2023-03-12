//
// Created by markus on 2/6/23.
//
#include <msp430.h>
#include <cpu/clock_module.h>
#include <timer/hwtimer.h>
#include <timer/watchdog_timer.h>
//#include <gpio/pin.h>

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

    //using pin = msp430hal::gpio::GPIOPins<msp430hal::gpio::Port::port_2, 0x08>;
    //pin::init();

    P2DIR |= BIT3 + BIT1;
    /*P2SEL &= ~BIT3;
    P2SEL2 &= ~BIT3;*/
    P2SEL |= BIT1;

    P2OUT &= ~BIT3;

    using ir_module_timer = msp430hal::timer::Timer_t<msp430hal::timer::TimerModule::timer_a, 1>;

    ir_module_timer::init(msp430hal::timer::TimerMode::up, msp430hal::timer::TimerClockSource::smclk, msp430hal::timer::TimerClockInputDivider::times_1);
    ir_module_timer::setCompareValue<0>(100);
    ir_module_timer::enableCaptureCompareInterrupt<0>();
    ir_module_timer::setCompareValue<1>(50);
    ir_module_timer::enableCaptureCompareInterrupt<1>();
    ir_module_timer::reset();

    //configure timer
    //using timer = msp430hal::timer::Timer_t<msp430hal::timer::TimerModule::timer_a, 0>;
    //timer::init(msp430hal::timer::TimerMode::continuous, msp430hal::timer::TimerClockSource::smclk, msp430hal::timer::TimerClockInputDivider::times_1);
    //timer::selectCaptureCompareInput<0>(msp430hal::timer::CaptureCompareInputSelect::gnd);
    //timer::setCaptureMode<0>(msp430hal::timer::TimerCaptureMode::rising_edge);

    __enable_interrupt();

    uint32_t expected = 70001;
    for(;;)
    {
    }
    return 0;
}
