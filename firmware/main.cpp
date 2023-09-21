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
#include "constants.h"

#include <msp430hal/cpu/flash_controller.h>

volatile std::uint_fast8_t g_comparator_counter = 0;
volatile bool g_capture_cycle_finished = false;
volatile bool g_uart_message_received = false;
volatile RingBufferQueue<std::uint8_t, 16> g_rx_buffer;
volatile bool g_uart_transmit_ready = false;
volatile RingBufferQueue<std::uint8_t, 16> g_tx_buffer;
volatile std::uint16_t g_comparator_cycle_timer_count = 0;

typedef msp430hal::timer::Timer_t<msp430hal::timer::TimerModule::timer_a, 1> ir_module_timer;
typedef msp430hal::timer::Timer_t<msp430hal::timer::TimerModule::timer_a, 0> gate_timer;
typedef msp430hal::gpio::GPIOPins<msp430hal::gpio::Port::port_2, msp430hal::gpio::Pin::p_0> module_1_status;
typedef msp430hal::gpio::GPIOPins<msp430hal::gpio::Port::port_3, msp430hal::gpio::Pin::p_0> module_2_status;
typedef msp430hal::gpio::GPIOPins<msp430hal::gpio::Port::port_3, msp430hal::gpio::Pin::p_1> module_3_status;
typedef msp430hal::gpio::GPIOPins<msp430hal::gpio::Port::port_2, msp430hal::gpio::Pin::p_3> module_4_status;
typedef msp430hal::gpio::GPIOPins<msp430hal::gpio::Port::port_3, msp430hal::gpio::Pin::p_6> module_5_status;
typedef msp430hal::gpio::GPIOPins<msp430hal::gpio::Port::port_3, msp430hal::gpio::Pin::p_7> module_6_status;
typedef msp430hal::gpio::GPIOPins<msp430hal::gpio::Port::port_3, msp430hal::gpio::Pin::p_5> gp_led;
typedef msp430hal::gpio::GPIOPins<msp430hal::gpio::Port::port_3, msp430hal::gpio::Pin::p_4, msp430hal::gpio::Mode::input, msp430hal::gpio::PinResistors::external_pullup> button;
typedef msp430hal::gpio::GPIOPins<msp430hal::gpio::Port::port_2, msp430hal::gpio::Pin::p_1 +
                                                                 msp430hal::gpio::Pin::p_2 +
                                                                 msp430hal::gpio::Pin::p_4 +
                                                                 msp430hal::gpio::Pin::p_5, msp430hal::gpio::Mode::output> port2_timer_out;
typedef msp430hal::gpio::GPIOPins<msp430hal::gpio::Port::port_3, msp430hal::gpio::Pin::p_2 +
                                                                 msp430hal::gpio::Pin::p_3, msp430hal::gpio::Mode::output> port3_timer_out;
typedef msp430hal::gpio::GPIOPins<msp430hal::gpio::Port::port_1, msp430hal::gpio::Pin::p_1 + msp430hal::gpio::Pin::p_2> uart_pins;
typedef msp430hal::usci::UART_t<0, 9600, 32768, msp430hal::usci::UARTClockSource::aclk> uart;

int main()
{
    const std::uint16_t gate_cycle_value = 10000;

    msp430hal::timer::stopWatchdog();

    //TODO: validate calibration data
    msp430hal::cpu::setCalibratedFrequency<msp430hal::cpu::CalibratedDCOFrequency::calibrated_8MHz>();
    msp430hal::cpu::setInputDivider<msp430hal::cpu::Clock::aclk>(msp430hal::cpu::Divider::times_1);
    msp430hal::cpu::setLowFrequencySource(msp430hal::cpu::LowFrequencySource::watch_crystal);
    msp430hal::cpu::selectOscillatorCapacitor(msp430hal::cpu::OscillatorCapacitor::approx_6pF);
    msp430hal::cpu::selectClockSource<msp430hal::cpu::Clock::smclk>(msp430hal::cpu::ClockSource::dcoclk);
    msp430hal::cpu::setInputDivider<msp430hal::cpu::Clock::smclk>(msp430hal::cpu::Divider::times_8);


    // Load user configuration and activate write protection for some values
    RAMMirroredFlashConfigurationStorage<15> configuration_storage;
    configuration_storage.reloadFromFlash();
    configuration_storage.setWriteProtection(1);
    configuration_storage.setWriteProtection(2);

    //Initialize module status leds
    module_1_status::init();
    module_2_status::init();
    module_3_status::init();
    module_4_status::init();
    module_5_status::init();
    module_6_status::init();


    gp_led::init();
    gp_led::clear();
    button::init();




    port2_timer_out::init(msp430hal::gpio::PinFunction::primary_peripheral);
    port3_timer_out::init(msp430hal::gpio::PinFunction::primary_peripheral);


    uart_pins::init(msp430hal::gpio::PinFunction::secondary_peripheral);

    ir_module_timer::init(msp430hal::timer::TimerMode::up, msp430hal::timer::TimerClockSource::smclk, msp430hal::timer::TimerClockInputDivider::times_1);
    ir_module_timer::setCompareValue<0>(100);
    ir_module_timer::setCompareValue<1>(50);
    ir_module_timer::setCompareValue<2>(50);
    ir_module_timer::setOutputMode<1>(msp430hal::timer::TimerOutputMode::set_reset);
    ir_module_timer::setOutputMode<2>(msp430hal::timer::TimerOutputMode::reset_set);

    gate_timer::init(msp430hal::timer::TimerMode::up, msp430hal::timer::TimerClockSource::smclk, msp430hal::timer::TimerClockInputDivider::times_1);
    gate_timer::setCompareValue<0>(gate_cycle_value);
    gate_timer::compareMode<0>();
    gate_timer::enableCaptureCompareInterrupt<0>();


    //Configure Comparator
    msp430hal::peripherals::Comparator comparator;

    const msp430hal::peripherals::ComparatorInput comparator_inputs[6] = {msp430hal::peripherals::ComparatorInput::ca_5,
                                                                    msp430hal::peripherals::ComparatorInput::ca_3,
                                                                    msp430hal::peripherals::ComparatorInput::ca_4,
                                                                    msp430hal::peripherals::ComparatorInput::ca_0,
                                                                    msp430hal::peripherals::ComparatorInput::ca_7,
                                                                    msp430hal::peripherals::ComparatorInput::ca_6};

    std::size_t current_channel = 0;
    comparator.setNonInvertingInput(msp430hal::peripherals::ComparatorInput::vcc_025);
    comparator.setInvertingInput(comparator_inputs[current_channel]);
    comparator.enableOutputFilter();
    comparator.enableInterrupt();
    comparator.enable();

    //CACTL2 = P2CA0 | CAF;
    //CACTL1 = CAREF_1 | CAON | CAIE;
    StatusManager status_manager;
    status_manager.bindLED(0, module_1_status::pins_value, module_1_status::out);
    status_manager.bindLED(1, module_2_status::pins_value, module_2_status::out);
    status_manager.bindLED(2, module_3_status::pins_value, module_3_status::out);
    status_manager.bindLED(3, module_4_status::pins_value, module_4_status::out);
    status_manager.bindLED(4, module_5_status::pins_value, module_5_status::out);
    status_manager.bindLED(5, module_6_status::pins_value, module_6_status::out);


    // Initialize UART
    uart::init();
    uart::Usci::enableRxInterrupt();
    uart::enable();

    __enable_interrupt();

    UartHandler<uart> uart_handler(&configuration_storage, &status_manager);


    for(;;)
    {
        if (g_capture_cycle_finished)
        {
            if (!(configuration_storage.get(11) & (1 << current_channel)))
            {
                if (g_comparator_counter >= 95 && g_comparator_counter <= 105)
                    status_manager.setBit(current_channel, true);
                else
                    status_manager.setBit(current_channel, false);
            }
            comparator.disableInterrupt();

            //Switch channel
            current_channel = (current_channel + 1) % 6;

            if (current_channel == 3)
            {
                comparator.setInvertingInput(msp430hal::peripherals::ComparatorInput::vcc_025);
                comparator.setNonInvertingInput(comparator_inputs[current_channel]);
            }
            else
            {
                comparator.setInvertingInput(comparator_inputs[current_channel]);
                comparator.setNonInvertingInput(msp430hal::peripherals::ComparatorInput::vcc_025);
            }
            //comparator.setInvertingInput(comparator_inputs[current_channel]);

            //reset counter
            g_comparator_counter = 0;
            g_capture_cycle_finished = false;


            gp_led::toggle();
            comparator.enableInterrupt();
            gate_timer::setCompareValue<0>(gate_cycle_value);
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
