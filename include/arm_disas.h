//
// Created by Kangkook Jee on 10/15/20.
//
#ifndef ARM_DISASM_H
#define ARM_DISASM_H

#include <stdint.h>

#define ARM_BL      0xeb
#define ARM_B       0xea
#define ARM_BGT     0xca
#define ARM_BX      0xe1

// ARM conditional prefix
#define ARM_COND_GT 0xc
#define ARM_COND_LE 0xd
#define ARM_COND_AL 0xe
// ....

#define IS_ARM_B(_XX)  ((_XX & 0x00f0) == 0x00a0)
#define IS_ARM_BL(_XX) ((_XX & 0x00f0) == 0x00b0)
#define IS_ARM_BX(_XX) ((_XX & 0x00ff) == 0x0012)

#define IS_ARM_COND(_XX) (  0 /* Implement */)

typedef struct {
    uint8_t     cond;
    uint16_t    opcode; // a single byte is enough to identify opcode.
    uint8_t     len;
} ARMInstr;

#define IS_ARM_CFLOW(_XX) (IS_ARM_B(_XX) || IS_ARM_BL(_XX) || IS_ARM_BX(_XX) || IS_ARM_COND(_XX))
#endif // ARM_DISASM_H
