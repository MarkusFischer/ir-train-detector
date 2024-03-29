#ifndef IR_TRAIN_DETECTOR_INTERRUPTS_H
#define IR_TRAIN_DETECTOR_INTERRUPTS_H
#include <msp430.h>

__attribute__((interrupt(TIMER0_A0_VECTOR))) void handleTimer0A0Interrupt();

__attribute__((interrupt(COMPARATORA_VECTOR))) void handleComparatorInterrupt();

__attribute__((interrupt(USCIAB0RX_VECTOR))) void handleUSCIRXInterrupt();


#endif //IR_TRAIN_DETECTOR_INTERRUPTS_H
