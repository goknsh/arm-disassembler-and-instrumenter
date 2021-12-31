#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "arm_context.h"
#include "arm_disas.h"
#include "macros.h"

/* addresses of asm callout glue code */
extern void *blCallout;
extern void *bCallout;
extern void *bxCallout;
extern void *bccCallout;
extern void *condCallout;
void *callTarget;

extern int user_prog(int);

void StartProfiling(void *func);

void StopProfiling(void);

void armDecode(uint8_t *ptr, ARMInstr *instr, int print);

void findAndPatchNextCF(uint8_t *addr);

void unpatchCF();

void *callTarget;

typedef struct {
  uint8_t *start;
  int count;
} BasicBlock;

/*********************************************************************
 *
 *  callout handlers
 *
 *   These get called by asm glue routines.
 *
 *********************************************************************/

uint8_t *restore_addr;
uint32_t restore_bytes;
BasicBlock block;

void handleBlCallout(SaveRegs *regs) {
  // From https://iitd-plos.github.io/col718/ref/arm-instructionset.pdf
  // Branch instructions contain a signed 2’s complement 24 bit offset. This is
  // shifted left two bits, sign extended to 32 bits, and added to the PC.
  printf("     └─── Handling BL callout:\n");
  unpatchCF();

  // First restore PC condition
  uint8_t *restore_pc = restore_addr + 8;
  // Calculate where the branch would have jumped to
  // Extract 0-23 bits
  uint32_t branch_offset = restore_bytes & ((1 << 24) - 1);
  // Check sign (23rd bit)
  if ((branch_offset >> 23) & ((1 << 1) - 1)) {
    branch_offset |= 0xff000000;  // 2s complement
  }
  // Align
  branch_offset <<= 2;
  // Would have jumped to
  restore_pc += branch_offset;
  printf(
      "          └─── [✓] Jump address calculated: <0x%08x>, Offset: "
      "<0x%08x>\n",
      restore_pc, branch_offset);

  regs->retPC = restore_pc;
  printf("          └─── [✓] Correct return PC set: <0x%08x>\n", regs->retPC);

  if (regs->retPC >= (uint8_t *)&user_prog) {
    findAndPatchNextCF(regs->retPC);
  } else {
    printf("          └─── [✓] Program has finished, profiling stopped\n");
  }
}

void handleBCallout(SaveRegs *regs) {
  // From https://iitd-plos.github.io/col718/ref/arm-instructionset.pdf
  // Branch instructions contain a signed 2’s complement 24 bit offset. This is
  // shifted left two bits, sign extended to 32 bits, and added to the PC.
  printf("     └─── Handling B callout:\n");
  unpatchCF();

  // First restore PC condition
  uint8_t *restore_pc = restore_addr + 8;
  // Calculate where the branch would have jumped to
  // Extract 0-23 bits
  uint32_t branch_offset = restore_bytes & ((1 << 24) - 1);
  // Check sign (23rd bit)
  if ((branch_offset >> 23) & ((1 << 1) - 1)) {
    branch_offset |= 0xff000000;  // 2s complement
  }
  // Align
  branch_offset <<= 2;
  // Would have jumped to
  restore_pc += branch_offset;

  printf(
      "          └─── [✓] Jump address calculated: <0x%08x>, Offset: "
      "<0x%08x>\n",
      restore_pc, branch_offset);

  regs->retPC = restore_pc;
  printf("          └─── [✓] Correct return PC set: <0x%08x>\n", regs->retPC);

  if (regs->retPC >= (uint8_t *)&user_prog) {
    findAndPatchNextCF(regs->retPC);
  } else {
    printf("          └─── [✓] Program has finished, profiling stopped\n");
  }
}

void evaluateBxRegister(SaveRegs *regs, char **bx_register) {
  int reg = (restore_bytes >> 0) & ((1 << 4) - 1);

  if (reg == 0x0) {
    regs->retPC = (uint8_t *)regs->r[0];
    *bx_register = "r0";
  } else if (reg == 0x1) {
    regs->retPC = (uint8_t *)regs->r[1];
    *bx_register = "r1";
  } else if (reg == 0x2) {
    regs->retPC = (uint8_t *)regs->r[2];
    *bx_register = "r2";
  } else if (reg == 0x3) {
    regs->retPC = (uint8_t *)regs->r[3];
    *bx_register = "r3";
  } else if (reg == 0x4) {
    regs->retPC = (uint8_t *)regs->r[4];
    *bx_register = "r4";
  } else if (reg == 0x5) {
    regs->retPC = (uint8_t *)regs->r[5];
    *bx_register = "r5";
  } else if (reg == 0x6) {
    regs->retPC = (uint8_t *)regs->r[6];
    *bx_register = "r6";
  } else if (reg == 0x7) {
    regs->retPC = (uint8_t *)regs->r[7];
    *bx_register = "r7";
  } else if (reg == 0x8) {
    regs->retPC = (uint8_t *)regs->r[8];
    *bx_register = "r8";
  } else if (reg == 0x9) {
    regs->retPC = (uint8_t *)regs->r[9];
    *bx_register = "r9";
  } else if (reg == 0xa) {
    regs->retPC = (uint8_t *)regs->r[10];
    *bx_register = "r10";
  } else if (reg == 0xb) {
    regs->retPC = (uint8_t *)regs->r[11];
    *bx_register = "r11";
  } else if (reg == 0xc) {
    regs->retPC = (uint8_t *)regs->r[12];
    *bx_register = "r12";
  } else if (reg == 0xd) {
    regs->retPC = (uint8_t *)regs->sp;
    *bx_register = "sp";
  } else if (reg == 0xe) {
    regs->retPC = (uint8_t *)regs->lr;
    *bx_register = "lr";
  } else if (reg == 0xf) {
    regs->retPC = (uint8_t *)regs->retPC;
    *bx_register = "pc";
  }
}

void handleBxCallout(SaveRegs *regs) {
  // From https://iitd-plos.github.io/col718/ref/arm-instructionset.pdf
  // This instruction performs a branch by copying the contents of a general
  // register, Rn, into the program counter, PC. The branch causes a pipeline
  // flush and refill from the address specified by Rn.
  printf("     └─── Handling BX callout:\n");
  unpatchCF();

  char *bx_register;
  evaluateBxRegister(regs, &bx_register);

  printf("          └─── [✓] Jump address calculated: <0x%08x> (BX %s)\n",
         regs->retPC, bx_register);
  printf("          └─── [✓] Correct return PC set: <0x%08x>\n", regs->retPC);

  if (regs->retPC >= (uint8_t *)&user_prog) {
    findAndPatchNextCF(regs->retPC);
  } else {
    printf("          └─── [✓] Program has finished, profiling stopped\n");
  }
}

int evaluateConditional(SaveRegs *regs, ARMInstr *instr,
                        char **conditional_type) {
  int taken = 0;
  int N = (regs->CPSR >> 31) & ((1 << 1) - 1);
  int Z = (regs->CPSR >> 30) & ((1 << 1) - 1);
  int C = (regs->CPSR >> 29) & ((1 << 1) - 1);
  int V = (regs->CPSR >> 28) & ((1 << 1) - 1);
  if (instr->cond == ARM_COND_EQ) {
    *conditional_type = "EQ";
    if (Z)
      taken = 1;
    else
      taken = 0;
  } else if (instr->cond == ARM_COND_NE) {
    *conditional_type = "NE";
    if (!Z)
      taken = 1;
    else
      taken = 0;
  } else if (instr->cond == ARM_COND_CS) {
    *conditional_type = "CS";
    if (C)
      taken = 1;
    else
      taken = 0;
  } else if (instr->cond == ARM_COND_CC) {
    *conditional_type = "CC";
    if (!C)
      taken = 1;
    else
      taken = 0;
  } else if (instr->cond == ARM_COND_MI) {
    *conditional_type = "MI";
    if (N)
      taken = 1;
    else
      taken = 0;
  } else if (instr->cond == ARM_COND_PL) {
    *conditional_type = "PL";
    if (!N)
      taken = 1;
    else
      taken = 0;
  } else if (instr->cond == ARM_COND_VS) {
    *conditional_type = "VS";
    if (V)
      taken = 1;
    else
      taken = 0;
  } else if (instr->cond == ARM_COND_VC) {
    *conditional_type = "VC";
    if (!V)
      taken = 1;
    else
      taken = 0;
  } else if (instr->cond == ARM_COND_HI) {
    *conditional_type = "HI";
    if (C && !Z)
      taken = 1;
    else
      taken = 0;
  } else if (instr->cond == ARM_COND_LS) {
    *conditional_type = "LS";
    if (!C && Z)
      taken = 1;
    else
      taken = 0;
  } else if (instr->cond == ARM_COND_GE) {
    *conditional_type = "GE";
    if (N == V)
      taken = 1;
    else
      taken = 0;
  } else if (instr->cond == ARM_COND_LT) {
    *conditional_type = "LT";
    if (N != V)
      taken = 1;
    else
      taken = 0;
  } else if (instr->cond == ARM_COND_GT) {
    *conditional_type = "GT";
    if (!Z && (N == V))
      taken = 1;
    else
      taken = 0;
  } else if (instr->cond == ARM_COND_LE) {
    *conditional_type = "LE";
    if (Z || (N != V))
      taken = 1;
    else
      taken = 0;
  }

  return taken;
}

void handleBccCallout(SaveRegs *regs) {
  // From https://iitd-plos.github.io/col718/ref/arm-instructionset.pdf
  // Branch instructions contain a signed 2’s complement 24 bit offset. This is
  // shifted left two bits, sign extended to 32 bits, and added to the PC.
  printf("     └─── Handling BCC callout:\n");
  unpatchCF();

  // Decode conditional
  char *conditional_type;
  ARMInstr *instr = malloc(sizeof(ARMInstr));
  armDecode(restore_addr, instr, 0);
  int jump = evaluateConditional(regs, instr, &conditional_type);

  uint8_t *restore_pc;
  uint32_t branch_offset;
  char *bx_register;
  if (jump) {
    if (IS_ARM_BX(instr->opcode)) {
      evaluateBxRegister(regs, &bx_register);
      restore_pc = regs->retPC;
    } else {
      // First restore PC condition
      restore_pc = restore_addr + 8;
      // Calculate where the branch would have jumped to
      // Extract 0-23 bits
      branch_offset = restore_bytes & ((1 << 24) - 1);
      // Check sign (23rd bit)
      if ((branch_offset >> 23) & ((1 << 1) - 1)) {
        branch_offset |= 0xff000000;  // 2s complement
      }
      // Align
      branch_offset <<= 2;
      // Would have jumped to
      restore_pc += branch_offset;
    }
  } else {
    restore_pc = restore_addr + 4;
    branch_offset = 4;
  }

  if (IS_ARM_BX(instr->opcode)) {
    printf(
        "          └─── [✓] Jump address calculated: <0x%08x> (BX%s %s), "
        "%s taken: %s\n",
        restore_pc, conditional_type, bx_register, conditional_type,
        jump ? "true" : "false");
  } else {
    printf(
        "          └─── [✓] Jump address calculated: <0x%08x> (%s%s), Offset: "
        "<0x%08x>, %s taken: %s\n",
        restore_pc, IS_ARM_B(instr->opcode) ? "B" : "BL", conditional_type,
        branch_offset, conditional_type, jump ? "true" : "false");
  }
  free(instr);

  regs->retPC = restore_pc;
  printf("          └─── [✓] Correct return PC set: <0x%08x>\n", regs->retPC);

  if (regs->retPC >= (uint8_t *)&user_prog) {
    findAndPatchNextCF(regs->retPC);
  } else {
    printf("          └─── [✓] Program has finished, profiling stopped\n");
  }
}

void handleCondCallout(SaveRegs *regs) {
  printf("     └─── Handling COND callout:\n");
  unpatchCF();

  char *conditional_type;
  ARMInstr *instr = malloc(sizeof(ARMInstr));
  armDecode(restore_addr, instr, 0);
  int taken = evaluateConditional(regs, instr, &conditional_type);
  free(instr);
  printf("          └─── [%s] %s taken: %s\n", taken ? "✓" : "✗",
         conditional_type, taken ? "true" : "false");

  uint8_t *restore_pc;
  if (taken) {
    restore_pc = restore_addr + 4;
  } else {
    restore_pc = restore_addr + 8;
  }

  regs->retPC = restore_pc;
  printf("          └─── [✓] Correct return PC set: <0x%08x>\n", regs->retPC);

  if (regs->retPC >= (uint8_t *)&user_prog) {
    findAndPatchNextCF(regs->retPC);
  } else {
    printf("          └─── [✓] Program has finished, profiling stopped\n");
  }
}

/*
 * Reserved for an extra credit assignment.
 */
void handlePopCall(SaveRegs *regs) { NOT_IMPLEMENTED(); }

/*********************************************************************
 *
 *  armDecode
 *
 *   Decode an ARM instruction.
 *
 *********************************************************************/

void armDecode(uint8_t *ptr, ARMInstr *instr, int print) {
  // type: 2 bytes -- 27 to 20 bits (maybe???)
  instr->opcode = ((*(uint32_t *)ptr) >> 20) & ((1 << 8) - 1);
  // type: 1 byte -- 31 to 28 bits
  instr->cond = ((*(uint32_t *)ptr) >> 28) & ((1 << 4) - 1);
  // type: 1 byte -- either 2 or 4
  instr->len = 4;

  if (print) {
    printf(
        "<%08x> ⟹  instr: %08x, opcode: 0x%02x, len: %d, isCFlow: %s, isCond: "
        "%s\n",
        ptr, *(uint32_t *)ptr, instr->opcode, instr->len,
        IS_ARM_CFLOW(instr->opcode) ? "true" : "false",
        IS_ARM_COND(instr->cond) ? "true" : "false");
  }
}

/*********************************************************************
 *
 *  StartProfiling, StopProfiling
 *
 *   Profiling hooks. This is your place to inspect and modify the profiled
 *   function.
 *
 *********************************************************************/

void findAndPatchNextCF(uint8_t *addr) {
  block.start = addr;
  block.count = 0;
  ARMInstr *instr = malloc(sizeof(ARMInstr));
  while (1) {
    block.count += 1;
    armDecode(addr, instr, 1);
    if (IS_ARM_CFLOW(instr->opcode) || IS_ARM_COND(instr->cond)) {
      break;
    }
    addr += instr->len;
  }

  restore_addr = addr;
  restore_bytes = *(uint32_t *)addr;

  void *call_target;
  int branch_type;

  if (IS_ARM_COND(instr->cond)) {
    call_target = IS_ARM_CFLOW(instr->opcode) ? &bccCallout : &condCallout;
    branch_type = IS_ARM_CFLOW(instr->opcode) && IS_ARM_BL(instr->opcode)
                      ? ARM_BL
                      : ARM_B;
  } else {
    if (IS_ARM_B(instr->opcode)) {
      call_target = &bCallout;
      branch_type = ARM_B;
    } else if (IS_ARM_BL(instr->opcode)) {
      call_target = &blCallout;
      branch_type = ARM_BL;
    } else if (IS_ARM_BX(instr->opcode)) {
      call_target = &bxCallout;
      branch_type = ARM_B;
    }
  }

  int32_t offset = ((int32_t)call_target - ((int32_t)addr + 8)) >> 2;
  addr[0] = ((int8_t *)&offset)[0];
  addr[1] = ((int8_t *)&offset)[1];
  addr[2] = ((int8_t *)&offset)[2];
  addr[3] = branch_type;

  __clear_cache(addr, addr + 4);

  if (IS_ARM_CFLOW(instr->opcode)) {
    printf("└─── block ⟹  start address: <0x%08x>, instruction count: %d\n",
           block.start, block.count);
  } else {
    printf("└──── cond ⟹  start address: <0x%08x>, instruction count: %d\n",
           block.start, block.count);
  }
  free(instr);
}

void unpatchCF() {
  restore_addr[0] = ((int8_t *)&restore_bytes)[0];
  restore_addr[1] = ((int8_t *)&restore_bytes)[1];
  restore_addr[2] = ((int8_t *)&restore_bytes)[2];
  restore_addr[3] = ((int8_t *)&restore_bytes)[3];
  __clear_cache(restore_addr, restore_addr + 4);
  printf("          └─── [✓] Old bytes restored, cache cleared\n");
}

void StartProfiling(void *func) {
  findAndPatchNextCF((uint8_t *)func);
}

void StopProfiling(void) {
}

int main(int argc, char *argv[]) {
  int value;
  char *end;

  struct timeval tval_before, tval_after, tval_result;

  char buf[16];

  if (argc != 1) {
    fprintf(stderr, "usage: %s\n", argv[0]);
    exit(1);
  }

#ifdef __FIB__
  printf("running fib()\n");
#endif

#ifdef __FACT__
  printf("running fact() \n");
#endif

#ifdef __FACT2__
  printf("running fact2()\n");
#endif

#ifdef __FACT3__
  printf("running fact3()\n");
#endif

  printf("input number: ");
  scanf("%15s", buf);

  value = strtol(buf, &end, 10);

  if (((errno == ERANGE) && ((value == LONG_MAX) || (value == LONG_MIN))) ||
      ((errno != 0) && (value == 0))) {
    perror("strtol");
    exit(1);
  }

  if (end == buf) {
    fprintf(stderr, "error: %s is not an integer\n", buf);
    exit(1);
  }

  if (*end != '\0') {
    fprintf(stderr, "error: junk at end of parameter: %s\n", end);
    exit(1);
  }

  gettimeofday(&tval_before, NULL);

  StartProfiling(user_prog);

  /*
   * function execution here.
   */
  value = user_prog(value);

  StopProfiling();

  gettimeofday(&tval_after, NULL);
  printf("%d\n", value);

  timersub(&tval_after, &tval_before, &tval_result);
  printf("Time elapsed: %ld.%06ld\n", (long int)tval_result.tv_sec,
         (long int)tval_result.tv_usec);
  exit(0);
}
