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
#include "StatusManager.h"
#include "UartHandler.h"
#include "RAMMirroredFlashConfigurationStorage.h"

#include <msp430hal/cpu/flash_controller.h>

volatile std::uint_fast8_t g_comparator_counter = 0;
volatile bool g_comparator_capture_cycle_finished = false;
volatile bool g_uart_message_received = false;
volatile RingBufferQueue<std::uint8_t, 16> g_rx_buffer;
volatile bool g_uart_transmit_ready = false;
volatile RingBufferQueue<std::uint8_t, 16> g_tx_buffer;


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


    RAMMirroredFlashConfigurationStorage<15> configuration_storage;
    configuration_storage.reloadFromFlash();

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


    //TODO: Replace with more readable gpio methods
    P2DIR |= BIT3 + BIT1;
    P2SEL |= BIT1;

    P2OUT &= ~BIT3;

    P1DIR |= BIT3;
    P1SEL |= BIT3 + BIT2 + BIT1;
    P1SEL2 |= BIT3 + BIT2 + BIT1;

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

    StatusManager status_manager;
    status_manager.bindLED(0, module_1_status::pins_value, module_1_status::out);
    status_manager.bindLED(1, module_2_status::pins_value, module_2_status::out);
    status_manager.bindLED(2, module_3_status::pins_value, module_3_status::out);
    status_manager.bindLED(3, module_4_status::pins_value, module_4_status::out);
    status_manager.bindLED(4, module_5_status::pins_value, module_5_status::out);
    status_manager.bindLED(5, module_6_status::pins_value, module_6_status::out);


    uart::enable();

    //uart::Usci::enableInterrupts();
    uart::Usci::enableRxInterrupt();
    //uart::Usci::enableTxInterrupt();
    //IE2 |= UCA0RXIE;
    __enable_interrupt();



    UartHandler<uart> uart_handler(&configuration_storage, &status_manager);

    int i = 0;
    for(;;)
    {
        if (g_comparator_capture_cycle_finished)
        {
            //Set status led if measured frequency is around 1kHz (+- 5%)
            std::uint16_t capture_value = timer_1mhz::getCaptureValue<1>();
            if (capture_value >= 900 && capture_value <= 1100)
                status_manager.setBit(0, true);
                //module_1_status::set();
            else
                status_manager.setBit(0, false);
                //module_1_status::clear();


            //reset counter
            g_comparator_counter = 0;
            g_comparator_capture_cycle_finished = false;
            timer_1mhz::selectCaptureCompareInput<1>(msp430hal::timer::CaptureCompareInputSelect::gnd);
            timer_1mhz::reset();
        }
        if (g_uart_message_received)
            uart_handler.update();

        if (uart::Usci::isTxInterruptPending() && !g_tx_buffer.empty())
            *uart::Usci::tx_buf = g_tx_buffer.dequeue();

        if (!configuration_storage.synchronized())
        {
            gp_led::set();
            configuration_storage.writeToFlash();
            gp_led::clear();
        }

        status_manager.updateLEDs();
    }
    return 0;
}
