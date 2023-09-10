#include "UartHandler.h"
#include "flags.h"

template<int uart_instance, std::size_t size, typename size_t>
UartHandler<uart_instance, size, size_t>::UartHandler(std::uint8_t* configuration_registers,
                                                      StatusManager<size, size_t>* status_manager)
: m_receive_multibyte_in_progress{false}, m_configuration_registers{configuration_registers}, m_status_manager{status_manager}
{}

template<int uart_instance, std::size_t size, typename size_t>
void UartHandler<uart_instance, size, size_t>::update()
{
    if (g_uart_message_to_handle)
    {
        std::uint8_t message = *Usci::rx_buf;
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
                    break;
                case status_addr:
                    std::uint8_t status_addr = message & 0x07;
                    bool status = m_status_manager->getBit(status_addr);
                    break;
                case getconfig:
                    std::uint8_t config_register = message & 0x0f;
                    std::uint8_t config_value = m_configuration_registers[config_register];
                    break;
                case configure:
                    m_receive_multibyte_in_progress = true;
                    m_receive_config_register = message & 0x0f;
                    break;
                default: //Nothing should happen
                    break;
            }
        }
        g_uart_message_to_handle = true;
    }
}