#ifndef IR_TRAIN_DETECTOR_RINGBUFFERQUEUE_H
#define IR_TRAIN_DETECTOR_RINGBUFFERQUEUE_H

#include <cstdint>
#include <msp430.h>
#include <msp430hal/multitasking/interrupt_guard.h>

typedef msp430hal::gpio::GPIOPins<msp430hal::gpio::Port::port_3, msp430hal::gpio::Pin::p_5> gp_led;

template<typename T, std::size_t size>
class RingBufferQueue
{
private:
    volatile T m_data[size];

    volatile std::size_t m_write_index = 0;
    volatile std::size_t m_read_index = 0;

    volatile std::size_t m_elements = 0;
public:

    void queue(T element) volatile
    {
        msp430hal::multitasking::InterruptGuard guard;
        if (m_elements < size)
        {
            m_data[m_write_index] = element;
            m_elements++;
            m_write_index = (m_write_index + 1) % size;
        }

    }

    T dequeue() volatile
    {
        msp430hal::multitasking::InterruptGuard guard;
        if (m_elements > 0)
        {
            m_elements--;
            std::uint8_t data = m_data[m_read_index];
            m_read_index = (m_read_index + 1) % size;
            return data;
        }

    }

    bool empty() const volatile
    {
        return m_elements == 0;
    }

    bool hasSpace() const volatile
    {
        return m_elements < size;
    }
};

#endif //IR_TRAIN_DETECTOR_RINGBUFFERQUEUE_H
