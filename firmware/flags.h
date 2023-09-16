#ifndef IR_TRAIN_DETECTOR_FLAGS_H
#define IR_TRAIN_DETECTOR_FLAGS_H

#include <cstdint>
#include "RingBufferQueue.h"

//Global variables (used for communication with isr)
extern volatile std::uint_fast8_t g_comparator_counter;
extern volatile bool g_comparator_capture_cycle_finished;
extern volatile bool g_uart_message_received;
extern volatile RingBufferQueue<std::uint8_t, 16> g_rx_buffer;
extern volatile bool g_uart_transmit_ready;
extern volatile RingBufferQueue<std::uint8_t, 16> g_tx_buffer;


#endif //IR_TRAIN_DETECTOR_FLAGS_H
