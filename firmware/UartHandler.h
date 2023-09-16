#ifndef IR_TRAIN_DETECTOR_UARTHANDLER_H
#define IR_TRAIN_DETECTOR_UARTHANDLER_H

#include <cstdint>
#include <msp430hal/usci/usci.h>
#include <msp430hal/gpio/pin.h>

#include "StatusManager.h"

typedef msp430hal::gpio::GPIOPins<msp430hal::gpio::Port::port_3, msp430hal::gpio::Pin::p_5> gp_led;

template<typename Uart>
class UartHandler
{
    static const std::uint8_t multibyte_command_bit = 0x80;

    bool m_receive_multibyte_in_progress = false;
    bool m_send_multibyte_in_progress = false;
    std::uint8_t m_multibyte_payload = 0;
    std::uint8_t m_receive_config_register = 0;

    std::uint8_t* m_configuration_registers;
    StatusManager* m_status_manager;

    enum UartCommands : std::uint8_t
    {
        reset = 0x00,
        status_all = 0x20,
        status_addr = 0x30,
        getconfig = 0x40,
        configure = 0x80
    };

    enum UartResponses : std::uint8_t
    {
        error = 0x00,
        ack = 0x10,
        status_addr_response = 0x20,
        status_all_response = 0x40,
        getconfig_response = 0x80,
        version_response = 0x90
    };

public:
    UartHandler(std::uint8_t* configuration_registers, StatusManager* status_manager)
    : m_receive_multibyte_in_progress{false}, m_configuration_registers{configuration_registers}, m_status_manager{status_manager}
    {}

    void update()
    {
        std::uint8_t response = 0;
        std::uint8_t message = g_rx_buffer.dequeue();
        //g_tx_buffer.queue(message);
        if (m_receive_multibyte_in_progress)
        {
            m_configuration_registers[m_receive_config_register] = message;
            m_receive_multibyte_in_progress = false;
        }
        else
        {
            switch(message & 0xf0)
            {
                case reset:
                    break;
                case status_all:
                    response = status_all_response | (m_status_manager->getStatusByte() & 0x3f);
                    gp_led::toggle();
                    break;
                case status_addr:
                    response = status_addr_response | (m_status_manager->getBit(status_addr) << 3) | (message & 0x07);
                    break;
                case getconfig:
                    response = getconfig_response | (message & 0x0f);
                    m_multibyte_payload = m_configuration_registers[message & 0x0f];
                    m_send_multibyte_in_progress = true;
                    break;
                case configure:
                    m_receive_multibyte_in_progress = true;
                    m_receive_config_register = message & 0x0f;
                    break;
                default: //This case should not happen
                    response = error;
                    break;
            }
        }

        if (response != 0)
        {
            g_tx_buffer.queue(response);
            if (m_send_multibyte_in_progress)
                g_tx_buffer.queue(m_multibyte_payload);
        }

        if (g_rx_buffer.empty())
            g_uart_message_received = false;
    }
};

#endif //IR_TRAIN_DETECTOR_UARTHANDLER_H
