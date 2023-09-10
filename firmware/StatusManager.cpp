#include "StatusManager.h"

std::uint8_t& StatusManager::getStatusByte()
{
    return m_status;
}

const std::uint8_t& StatusManager::getStatusByte() const
{
    return m_status;
}

bool StatusManager::getBit(std::size_t i) const
{
    if (i < size)
        return m_status & (1 << i);
    return false;
}

void StatusManager::setBit(std::size_t i, bool bit)
{
    if (i < size)
    {
        if (bit)
            m_status |= (1 << i);
        else
            m_status &= ~(1 << i);
        m_updated = true;
    }
}

template<msp430hal::gpio::Port port>
void StatusManager::bindLED(std::size_t i, msp430hal::gpio::Pin pin)
{
    if (i < size)
    {
        m_led_binds[i].m_bit = pin;
        m_led_binds[i].m_out_register = msp430hal::gpio::GPIOPins<port, 1>::out;
    }
}

void StatusManager::bindLED(std::size_t i, std::size_t pin, volatile std::uint8_t* out_register)
{
    if (i < size)
    {
        m_led_binds[i].m_bit = pin;
        m_led_binds[i].m_out_register = out_register;
    }
}

void StatusManager::updateLEDs()
{
    if (m_updated)
    {
        for (std::size_t i = 0; i < size; ++i)
        {
            if (m_led_binds[i].m_out_register == nullptr)
                continue;
            if (getBit(i))
                *m_led_binds[i].m_out_register |= m_led_binds[i].m_bit;
            else
                *m_led_binds[i].m_out_register &= ~m_led_binds[i].m_bit;
        }
    }
    m_updated = false;
}