#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <sys/time.h>

#include "macros.h"
#include "arm_context.h"
#include "arm_disas.h"

/* addresses of asm callout glue code */
extern void *blCallout;
extern void *bCallout;
extern void *bxCallout;
extern void *bgtCallout;
void *callTarget;

extern int user_prog(int);

void StartProfiling(void *func);

void StopProfiling(void);

void armDecode(uint8_t *ptr, ARMInstr *instr);

void *callTarget;


/*********************************************************************
 *
 *  callout handlers
 *
 *   These get called by asm glue routines.
 *
 *********************************************************************/
void
handleBlCallout(SaveRegs *regs) {
    NOT_IMPLEMENTED();
}

void
handleBCallout(SaveRegs *regs) {
    NOT_IMPLEMENTED();
}

void
handleBxCallout(SaveRegs *regs) {
    NOT_IMPLEMENTED();
}

void
handleBccCallout(SaveRegs *regs) {
    NOT_IMPLEMENTED();
}

void
handleCondCallout(SaveRegs *regs) {
    NOT_IMPLEMENTED();
}

/*
 * Reserved for an extra credit assignment.
 */
void
handlePopCall(SaveRegs *regs) {
    NOT_IMPLEMENTED();
}

/*********************************************************************
 *
 *  armDecode
 *
 *   Decode an ARM instruction.
 *
 *********************************************************************/

void
armDecode(uint8_t *ptr, ARMInstr *instr) {
    NOT_IMPLEMENTED();
}


/*********************************************************************
 *
 *  StartProfiling, StopProfiling
 *
 *   Profiling hooks. This is your place to inspect and modify the profiled
 *   function.
 *
 *********************************************************************/

void
StartProfiling(void *func) {
    /*
     * TODO: Add your instrumentation code here.
     */
}

void
StopProfiling(void) {
    /*
     * TODO: Add your instrumentation code here.
     */

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

    if (((errno == ERANGE)
         && ((value == LONG_MAX) || (value == LONG_MIN)))
        || ((errno != 0) && (value == 0))) {
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
    printf("Time elapsed: %ld.%06ld\n", (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);
    exit(0);
}
