#ifndef MEC_CO_SYSV_X86_64_H
#define MEC_CO_SYSV_X86_64_H

#include "base.h"

__attribute__((naked, noinline))
void co_switch(void **from_sp $unused, void *to_sp $unused)
{
    __asm__ volatile(
        "pushq %rbp\n"
        "pushq %rbx\n"
        "pushq %r12\n"
        "pushq %r13\n"
        "pushq %r14\n"
        "pushq %r15\n"
        "movq  %rsp, (%rdi)\n"   // *from_sp = rsp
        "movq  %rsi, %rsp\n"     // rsp = to_sp
        "popq  %r15\n"
        "popq  %r14\n"
        "popq  %r13\n"
        "popq  %r12\n"
        "popq  %rbx\n"
        "popq  %rbp\n"
        "ret\n"
    );
}

void *co_init_stack(byte *stack, usize size, void (*entry)(void))
{
    uintptr top = ((uintptr)stack + size) & ~(uintptr)15;

    void **sp = (void**)top;
    *--sp = NULL; // fake return address for entry
    *--sp = (void*)(uintptr)entry; // popped by the ret in co_switch
    for (int i = 0; i < 6; ++i) *--sp = NULL; // rbp rbx r12 r13 r14 r15

    return sp;
}

#endif // MEC_CO_SYSV_X86_64_H
