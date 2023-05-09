#ifndef IR_TRAIN_DETECTOR_FLAGS_H
#define IR_TRAIN_DETECTOR_FLAGS_H

#include <cstdint>

//Global variables (used for communication with isr)
extern volatile std::uint_fast8_t g_comparator_counter;
extern volatile bool g_comparator_capture_cycle_finished;

#endif //IR_TRAIN_DETECTOR_FLAGS_H
