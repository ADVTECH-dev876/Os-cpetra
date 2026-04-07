#include "scheduler.h"
#include "paging.h"
#include "heap.h"
#include "serial.h"
#include <string.h>

static struct process* processes[MAX_PROCESSES];
static struct process* current_process = NULL;
static int next_pid = 1;
static int process_count = 0;

/* Idle process */
static void idle_process(void) {
    while(1) {
        __asm__ volatile ("hlt");
    }
}

/* Context switch function (assembly) */
extern void switch_context(struct regs* old, struct regs* new);

void scheduler_init(void) {
    /* Create idle process */
    int idle_pid = create_process((uint64_t)idle_process);
    if (idle_pid >= 0) {
        current_process = processes[idle_pid];
        current_process->state = 1;  /* running */
    }
    
    serial_write_string("Scheduler initialized\n");
}

int create_process(uint64_t entry_point) {
    if (process_count >= MAX_PROCESSES) {
        return -1;
    }
    
    struct process* proc = (struct process*)malloc(sizeof(struct process));
    if (!proc) return -1;
    
    /* Allocate kernel stack */
    proc->kernel_stack = (uint64_t*)vmm_alloc_page();
    if (!proc->kernel_stack) {
        free(proc);
        return -1;
    }
    
    /* Set up initial context */
    proc->context = (struct regs*)((uint64_t)proc->kernel_stack + STACK_SIZE - sizeof(struct regs));
    memset(proc->context, 0, sizeof(struct regs));
    
    proc->context->rip = entry_point;
    proc->context->cs = 0x08;  /* Kernel code segment */
    proc->context->rflags = 0x202;  /* Interrupts enabled */
    proc->context->rsp = (uint64_t)proc->kernel_stack + STACK_SIZE;
    proc->context->ss = 0x10;  /* Kernel data segment */
    
    proc->pid = next_pid++;
    proc->state = 0;  /* ready */
    proc->next = NULL;
    
    /* Add to process list */
    processes[proc->pid] = proc;
    process_count++;
    
    /* Add to round-robin queue */
    if (current_process) {
        struct process* p = current_process;
        while (p->next && p->next != current_process) {
            p = p->next;
        }
        p->next = proc;
        proc->next = current_process;
    } else {
        current_process = proc;
        proc->next = proc;
    }
    
    char buf[32];
    serial_write_string("Created process with PID ");
    /* Convert pid to string - simplified */
    serial_write_string("\n");
    
    return proc->pid;
}

void scheduler_tick(void) {
    if (!current_process) return;
    
    /* Round-robin scheduling */
    if (current_process->state == 1) {  /* running */
        current_process->state = 0;  /* ready */
        current_process = current_process->next;
        
        /* Find next ready process */
        while (current_process->state != 0) {
            current_process = current_process->next;
            if (current_process == current_process->next) break;
        }
        
        current_process->state = 1;  /* running */
        
        /* Context switch - will be called from interrupt handler */
        /* The actual switch happens in the IRQ handler via switch_context */
    }
}

void yield(void) {
    /* Trigger timer interrupt manually */
    __asm__ volatile ("int $0x20");
}

struct process* get_current_process(void) {
    return current_process;
}
