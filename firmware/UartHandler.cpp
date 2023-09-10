#include "UartHandler.h"
#include "flags.h"

template<typename Uart>
UartHandler<Uart>::UartHandler(std::uint8_t* configuration_registers, StatusManager* status_manager)
: m_receive_multibyte_in_progress{false}, m_configuration_registers{configuration_registers}, m_status_manager{status_manager}
{}

template<typename Uart>
void UartHandler<Uart>::update()
{
    std::uint8_t response = 0;
    if (g_uart_message_to_handle)
    {
        std::uint8_t message = *Uart::Usci::rx_buf;
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
                    break;
                case status_addr:
                    std::uint8_t status_addr = message & 0x07;
                    bool status = m_status_manager->getBit(status_addr);
                    response = status_addr_response | (status << 3) | status_addr;
                    break;
                case getconfig:
                    std::uint8_t config_register = message & 0x0f;
                    response = getconfig_response | config_register;
                    m_multibyte_payload = m_configuration_registers[config_register];
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
            // TODO: Replace with non blocking transmit
            while (!Uart::Usci::isTxInterruptPending());
            Uart::Usci::tx_buf = response;
            if (m_send_multibyte_in_progress)
            {
                while (!Uart::Usci::isTxInterruptPending());
                Uart::Usci::tx_buf = m_multibyte_payload;
            }
        }

        g_uart_message_to_handle = false;
    }
}