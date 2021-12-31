//
// Created by Kangkook Jee on 10/15/20.
//
#ifndef ARM_DISASM_H
#define ARM_DISASM_H

#include <stdint.h>

#define ARM_BL 0xeb
#define ARM_B 0xea
#define ARM_BGT 0xca
#define ARM_BX 0xe1

// ARM conditional prefix
#define ARM_COND_EQ 0x0
#define ARM_COND_NE 0x1
#define ARM_COND_CS 0x2
#define ARM_COND_CC 0x3
#define ARM_COND_MI 0x4
#define ARM_COND_PL 0x5
#define ARM_COND_VS 0x6
#define ARM_COND_VC 0x7
#define ARM_COND_HI 0x8
#define ARM_COND_LS 0x9
#define ARM_COND_GE 0xa
#define ARM_COND_LT 0xb
#define ARM_COND_GT 0xc
#define ARM_COND_LE 0xd
#define ARM_COND_AL 0xe

#define IS_ARM_B(_XX) ((_XX & 0x00f0) == 0x00a0)
#define IS_ARM_BL(_XX) ((_XX & 0x00f0) == 0x00b0)
#define IS_ARM_BX(_XX) ((_XX & 0x00ff) == 0x0012)

#define IS_ARM_COND(_X)                                           \
  (_X == ARM_COND_EQ || _X == ARM_COND_NE || _X == ARM_COND_CS || \
   _X == ARM_COND_CC || _X == ARM_COND_MI || _X == ARM_COND_PL || \
   _X == ARM_COND_VS || _X == ARM_COND_VC || _X == ARM_COND_HI || \
   _X == ARM_COND_LS || _X == ARM_COND_GE || _X == ARM_COND_LT || \
   _X == ARM_COND_GT || _X == ARM_COND_LE)

typedef struct {
  uint8_t cond;
  uint16_t opcode;  // a single byte is enough to identify opcode.
  uint8_t len;
} ARMInstr;

#define IS_ARM_CFLOW(_XX) (IS_ARM_B(_XX) || IS_ARM_BL(_XX) || IS_ARM_BX(_XX))
#endif  // ARM_DISASM_H
