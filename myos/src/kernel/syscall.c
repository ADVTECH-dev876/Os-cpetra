#include "syscall.h"
#include "serial.h"
#include "scheduler.h"

/* Syscall handler */
uint64_t handle_syscall(uint64_t syscall_num, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
    switch (syscall_num) {
        case SYS_WRITE:
            /* Write to serial for now */
            for (uint64_t i = 0; i < arg3; i++) {
                serial_write_char(((char*)arg1)[i]);
            }
            return arg3;
            
        case SYS_READ:
            /* Not implemented - would read from keyboard */
            return 0;
            
        case SYS_GETPID:
            return get_current_process()->pid;
            
        case SYS_EXIT:
            /* Process exit - kill current process */
            get_current_process()->state = 3;  /* zombie */
            yield();  /* Force reschedule */
            return 0;
            
        default:
            return -1;
    }
}

/* Syscall entry point (called from interrupt) */
void syscall_handler(struct regs* r) {
    /* Syscall convention: rax = number, rdi, rsi, rdx = args */
    uint64_t ret = handle_syscall(r->rax, r->rdi, r->rsi, r->rdx);
    r->rax = ret;  /* Return value */
}

void syscall_init(void) {
    /* Set up syscall interrupt (0x80) */
    /* In a full implementation, you'd use the SYSCALL instruction,
       but for simplicity we use INT 0x80 */
    
    serial_write_string("Syscall interface initialized\n");
}
