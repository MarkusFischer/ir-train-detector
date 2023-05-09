#ifndef IR_TRAIN_DETECTOR_INTERRUPTS_H
#define IR_TRAIN_DETECTOR_INTERRUPTS_H
#include <msp430.h>

__attribute__((interrupt(TIMER0_A1_VECTOR))) void handleTimer0A1Interrupts();

__attribute__((interrupt(COMPARATORA_VECTOR))) void handleComparatorInterrupt();

#endif //IR_TRAIN_DETECTOR_INTERRUPTS_H