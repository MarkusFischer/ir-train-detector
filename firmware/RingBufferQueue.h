#ifndef IR_TRAIN_DETECTOR_RINGBUFFERQUEUE_H
#define IR_TRAIN_DETECTOR_RINGBUFFERQUEUE_H

#include <cstdint>
#include <msp430.h>
#include <msp430hal/multitasking/interrupt_guard.h>

template<typename T, std::size_t size>
class RingBufferQueue
{
private:
    T m_data[size];

    std::size_t m_write_index = 0;
    std::size_t m_read_index = 0;

    std::size_t m_elements = 0;
public:

    void queue(T element)
    {
        msp430hal::multitasking::InterruptGuard guard;
        if (m_elements < size)
        {
            m_data[m_write_index] = element;
            m_elements++;
            m_write_index = (m_write_index + 1) % size;
        }

    }

    T dequeue()
    {
        msp430hal::multitasking::InterruptGuard guard;
        if (m_elements > 0)
        {
            m_elements--;
            m_read_index = (m_read_index + 1) % size;
            return m_data[m_read_index];
        }

    }

    bool empty() const
    {
        return m_elements == 0;
    }

    bool hasSpace() const
    {
        return m_elements < size;
    }
};

#endif //IR_TRAIN_DETECTOR_RINGBUFFERQUEUE_H
