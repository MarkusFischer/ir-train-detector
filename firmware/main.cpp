//
// Created by markus on 2/6/23.
//
#include <msp430.h>
#include <gpio/pin.h>

int main()
{
    WDTCTL = WDTPW | WDTHOLD;

    using msp430hal::gpio::Pin;

    typedef msp430hal::gpio::GPIOPins<msp430hal::gpio::Port::port_2, msp430hal::gpio::Pin::p_1, msp430hal::gpio::Mode::output> module1_status;
    typedef msp430hal::gpio::GPIOPins<msp430hal::gpio::Port::port_2, msp430hal::gpio::Pin::p_0, msp430hal::gpio::Mode::output> module2_status;
    typedef msp430hal::gpio::GPIOPins<msp430hal::gpio::Port::port_3, msp430hal::gpio::Pin::p_0, msp430hal::gpio::Mode::output> module3_status;
    typedef msp430hal::gpio::GPIOPins<msp430hal::gpio::Port::port_3, msp430hal::gpio::Pin::p_1, msp430hal::gpio::Mode::output> module4_status;
    typedef msp430hal::gpio::GPIOPins<msp430hal::gpio::Port::port_3, msp430hal::gpio::Pin::p_6, msp430hal::gpio::Mode::output> module5_status;
    typedef msp430hal::gpio::GPIOPins<msp430hal::gpio::Port::port_3, msp430hal::gpio::Pin::p_7, msp430hal::gpio::Mode::output> module6_status;

    typedef msp430hal::gpio::GPIOPins<msp430hal::gpio::Port::port_3, msp430hal::gpio::Pin::p_5, msp430hal::gpio::Mode::output> gp_led;

    msp430hal::gpio::GPIOPins<msp430hal::gpio::Port::port_2, msp430hal::gpio::Pin::p_0 + msp430hal::gpio::Pin::p_1>::init();
    msp430hal::gpio::GPIOPins<msp430hal::gpio::Port::port_3, msp430hal::gpio::Pin::p_0 + msp430hal::gpio::Pin::p_1 + msp430hal::gpio::Pin::p_6 + msp430hal::gpio::Pin::p_7>::init();

    typedef msp430hal::gpio::GPIOPins<msp430hal::gpio::Port::port_2, Pin::p_2 + Pin::p_3 + Pin::p_4 + Pin::p_5> output_p2;
    output_p2::init();
    output_p2::set();

    typedef msp430hal::gpio::GPIOPins<msp430hal::gpio::Port::port_3, Pin::p_2 + Pin::p_3> output_p3;
    output_p3::init();
    output_p3::set();

    gp_led::init();

    module1_status::set();
    module2_status::set();
    module3_status::set();
    module4_status::set();
    module5_status::set();
    module6_status::set();

    gp_led::set();
    for(;;);
    return 0;
}