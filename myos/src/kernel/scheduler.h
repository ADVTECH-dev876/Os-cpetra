#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>

#define MAX_PROCESSES 32
#define STACK_SIZE 4096

/* Register save area for context switching */
struct regs {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t int_no, err_code;
    uint64_t rip, cs, rflags, rsp, ss;
};

struct process {
    int pid;
    uint64_t* stack;
    uint64_t* kernel_stack;
    struct regs* context;
    int state;  /* 0: ready, 1: running, 2: waiting, 3: zombie */
    struct process* next;
};

void scheduler_init(void);
int create_process(uint64_t entry_point);
void scheduler_tick(void);
void yield(void);
struct process* get_current_process(void);

#endif
