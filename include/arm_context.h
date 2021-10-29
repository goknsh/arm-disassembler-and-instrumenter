#ifndef ARM_CONTEXT_H
#define ARM_CONTEXT_H

#include <stdint.h>

typedef struct {
    uint32_t   CPSR;
    uint32_t   r[13];
    uint32_t   sp;  
    uint32_t   lr;
    uint8_t*   retPC;
} SaveRegs;
#endif
