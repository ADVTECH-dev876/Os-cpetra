#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

/* Syscall numbers */
#define SYS_WRITE 1
#define SYS_READ 2
#define SYS_GETPID 20
#define SYS_EXIT 60

void syscall_init(void);
uint64_t handle_syscall(uint64_t syscall_num, uint64_t arg1, uint64_t arg2, uint64_t arg3);

#endif
