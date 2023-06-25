//
// Created by markus on 2/6/23.
//
#include <msp430.h>
#include <msp430hal/cpu/clock_module.h>
#include <msp430hal/timer/hwtimer.h>
#include <msp430hal/timer/watchdog_timer.h>
#include <msp430hal/peripherals/comparator.h>
#include <msp430hal/gpio/pin.h>
#include <msp430hal/usci/uart.h>

#include "interrupts.h"
#include "flags.h"
#include "StatusLEDManager.h"

volatile std::uint_fast8_t g_comparator_counter = 0;
volatile bool g_comparator_capture_cycle_finished = false;
volatile bool g_uart_message_to_handle = false;

typedef msp430hal::timer::Timer_t<msp430hal::timer::TimerModule::timer_a, 1> ir_module_timer;
typedef msp430hal::timer::Timer_t<msp430hal::timer::TimerModule::timer_a, 0> timer_1mhz;
typedef msp430hal::gpio::GPIOPins<msp430hal::gpio::Port::port_2, msp430hal::gpio::Pin::p_0> module_1_status;
typedef msp430hal::gpio::GPIOPins<msp430hal::gpio::Port::port_3, msp430hal::gpio::Pin::p_0> module_2_status;
typedef msp430hal::gpio::GPIOPins<msp430hal::gpio::Port::port_3, msp430hal::gpio::Pin::p_1> module_3_status;
typedef msp430hal::gpio::GPIOPins<msp430hal::gpio::Port::port_2, msp430hal::gpio::Pin::p_3> module_4_status;
typedef msp430hal::gpio::GPIOPins<msp430hal::gpio::Port::port_3, msp430hal::gpio::Pin::p_6> module_5_status;
typedef msp430hal::gpio::GPIOPins<msp430hal::gpio::Port::port_3, msp430hal::gpio::Pin::p_7> module_6_status;
typedef msp430hal::gpio::GPIOPins<msp430hal::gpio::Port::port_3, msp430hal::gpio::Pin::p_5> gp_led;
typedef msp430hal::gpio::GPIOPins<msp430hal::gpio::Port::port_3, msp430hal::gpio::Pin::p_4, msp430hal::gpio::Mode::input, msp430hal::gpio::PinResistors::external_pullup> button;
typedef msp430hal::usci::UART_t<0, 9600, 32768, msp430hal::usci::UARTClockSource::aclk> uart;

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
    module_2_status::init();
    module_3_status::init();
    module_4_status::init();
    module_5_status::init();
    module_6_status::init();

    gp_led::init();
    gp_led::clear();
    button::init();

    uart::init();
    //UCA0CTL1 |= UCSWRST;
    //UCA0CTL1 = UCSSEL_1 | UCSWRST;

    //UCA0BR0 = 3;
    //UCA0MCTL = 56;
    //IE2 |= UCA0RXIE;



    //TODO: Replace with more readable gpio methods
    P2DIR |= BIT3 + BIT1;
    P2SEL |= BIT1;

    P2OUT &= ~BIT3;

    P1DIR |= BIT3;
    P1SEL |= BIT3 + BIT2 + BIT1;
    P1SEL2 |= BIT3 + BIT2 + BIT1;

    //UCA0CTL1 &= ~UCSWRST;

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

    StatusLEDManager<6> status_leds;
    status_leds.bindLED(0, module_1_status::pins_value, module_1_status::out);
    status_leds.bindLED(1, module_2_status::pins_value, module_2_status::out);
    status_leds.bindLED(2, module_3_status::pins_value, module_3_status::out);
    status_leds.bindLED(3, module_4_status::pins_value, module_4_status::out);
    status_leds.bindLED(4, module_5_status::pins_value, module_5_status::out);
    status_leds.bindLED(5, module_6_status::pins_value, module_6_status::out);

    uart::enable();

    uart::Usci::enableRxInterrupt();
    __enable_interrupt();

    int i = 0;
    for(;;)
    {
        if (g_comparator_capture_cycle_finished)
        {
            //Set status led if measured frequency is around 1kHz (+- 5%)
            std::uint16_t capture_value = timer_1mhz::getCaptureValue<1>();
            if (capture_value >= 900 && capture_value <= 1100)
                status_leds.setBit(0, true);
                //module_1_status::set();
            else
                status_leds.setBit(0, false);
                //module_1_status::clear();


            //reset counter
            g_comparator_counter = 0;
            g_comparator_capture_cycle_finished = false;
            timer_1mhz::selectCaptureCompareInput<1>(msp430hal::timer::CaptureCompareInputSelect::gnd);
            timer_1mhz::reset();
        }

        //IFG2 |= UCA0RXIFG;

        ++i;
        if (g_uart_message_to_handle)
        {
            /*char message = *uart::Usci::rx_buf;
            if (message == 'a')
                gp_led::set();
            else
                gp_led::clear();
            */
            gp_led::toggle();
            g_uart_message_to_handle = false;
        }

        status_leds.updateLEDs();
    }
    return 0;
}
