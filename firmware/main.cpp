//
// Created by markus on 2/6/23.
//
#include <msp430.h>
#include <gpio/pin.h>

int main()
{
    WDTCTL = WDTPW | WDTHOLD;
    
    typedef msp430hal::gpio::GPIOPin<msp430hal::gpio::Port::port_2, msp430hal::gpio::Pin::p_1, msp430hal::gpio::Mode::output> module1_status;
    typedef msp430hal::gpio::GPIOPin<msp430hal::gpio::Port::port_2, msp430hal::gpio::Pin::p_0, msp430hal::gpio::Mode::output> module2_status;
    typedef msp430hal::gpio::GPIOPin<msp430hal::gpio::Port::port_3, msp430hal::gpio::Pin::p_0, msp430hal::gpio::Mode::output> module3_status;
    typedef msp430hal::gpio::GPIOPin<msp430hal::gpio::Port::port_3, msp430hal::gpio::Pin::p_1, msp430hal::gpio::Mode::output> module4_status;
    typedef msp430hal::gpio::GPIOPin<msp430hal::gpio::Port::port_3, msp430hal::gpio::Pin::p_6, msp430hal::gpio::Mode::output> module5_status;
    typedef msp430hal::gpio::GPIOPin<msp430hal::gpio::Port::port_3, msp430hal::gpio::Pin::p_7, msp430hal::gpio::Mode::output> module6_status;

    typedef msp430hal::gpio::GPIOPin<msp430hal::gpio::Port::port_3, msp430hal::gpio::Pin::p_5, msp430hal::gpio::Mode::output> gp_led;

    module1_status::init();
    module2_status::init();
    module3_status::init();
    module4_status::init();
    module5_status::init();
    module6_status::init();

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