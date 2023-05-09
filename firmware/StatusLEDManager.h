#ifndef IR_TRAIN_DETECTOR_STATUSLEDMANAGER_H
#define IR_TRAIN_DETECTOR_STATUSLEDMANAGER_H

#include <cstdint>

#include <msp430hal/gpio/pin.h>

template<std::size_t size, typename size_t = std::size_t>
class StatusLEDManager
{
    std::uint8_t m_status[(size / 8) + 1];

    struct LEDBind
    {
        size_t m_bit;
        volatile std::uint8_t* m_out_register;
    };

    LEDBind m_led_binds[size];

public:

    std::uint8_t& getStatusByte(size_t byte)
    {
        if (byte < (size / 8) + 1)
            return m_status[byte];
    }

    const std::uint8_t& getStatusByte(size_t byte) const
    {
        if (byte < (size / 8) + 1)
            return m_status[byte];
    }

    bool getBit(size_t i) const
    {
        if (i < size)
            return m_status[(i / 8)] & (1 << (i % 8));
    }

    void setBit(size_t i, bool bit)
    {
        if (i < size)
        {
            if (bit)
                m_status[(i / 8)] |= (1 << (i % 8));
            else
                m_status[(i / 8)] &= ~(1 << (i % 8));
        }
    }

    template<msp430hal::gpio::Port port>
    void bindLED(size_t i, msp430hal::gpio::Pin pin)
    {
        if (i < size)
        {
            m_led_binds[i].m_bit = pin;
            m_led_binds[i].m_out_register = msp430hal::gpio::GPIOPins<port, 1>::out;
        }
    }

    void bindLED(size_t i, size_t pin, volatile std::uint8_t* out_register)
    {
        if (i < size)
        {
            m_led_binds[i].m_bit = pin;
            m_led_binds[i].m_out_register = out_register;
        }
    }

    void updateLEDs()
    {
        for (size_t i = 0; i < size; ++i)
        {
            if (m_led_binds[i].m_out_register == nullptr)
                continue;
            if (getBit(i))
                *m_led_binds[i].m_out_register |= m_led_binds[i].m_bit;
            else
                *m_led_binds[i].m_out_register &= ~m_led_binds[i].m_bit;
        }
    }
};

#endif //IR_TRAIN_DETECTOR_STATUSLEDMANAGER_H
