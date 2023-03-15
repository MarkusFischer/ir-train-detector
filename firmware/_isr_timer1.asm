    .cdecls C,NOLIST,"msp430.h"
    .global _isr_handle_ta1iv
    .global _isr_handle_taccr0
    .sect ".text:_isr"
_isr_handle_ta1iv: .asmfunc                  ; Acceptance of ISR: 6 Cycles
    ADD &TA1IV,PC                   ; Add offset: 3 Cycles
    RETI                            ; Return from ISR 5 Cylces TOTAL: 14
    JMP ccifg_1_handling
    JMP ccifg_2_handling
    RETI                            ; Return from ISR 5 Cylces TOTAL: 14
    RETI                            ; Return from ISR 5 Cylces TOTAL: 14
timer_overflow:
    RETI                            ; Return from ISR 5 Cylces TOTAL: 14
ccifg_1_handling:
    BIS.B #0x08, &P2OUT             ; 5 Cycles
    RETI                            ; Return from ISR 5 Cylces TOTAL: 21
ccifg_2_handling:
    RETI                            ; Return from ISR 5 Cylces TOTAL: 16
    .endasmfunc

_isr_handle_taccr0: .asmfunc
    BIC.B #0x08, &P2OUT
    RETI
    .endasmfunc

    .intvec TIMER1_A1_VECTOR, _isr_handle_ta1iv
    .intvec TIMER1_A0_VECTOR, _isr_handle_taccr0
