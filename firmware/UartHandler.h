#ifndef IR_TRAIN_DETECTOR_UARTHANDLER_H
#define IR_TRAIN_DETECTOR_UARTHANDLER_H

#include <cstdint>
#include <msp430hal/usci/usci.h>

#include "StatusManager.h"


template<int uart_instance>
class UartHandler
{
    typedef msp430hal::usci::Usci_t<msp430hal::usci::UsciModule::usci_a, uart_instance> Usci;
    static constexpr std::uint8_t multibyte_command_bit = 0x80;

    bool m_receive_multibyte_in_progress = false;
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
    UartHandler(std::uint8_t* configuration_registers, StatusManager* status_manager);

    void update();
};

#endif //IR_TRAIN_DETECTOR_UARTHANDLER_H
