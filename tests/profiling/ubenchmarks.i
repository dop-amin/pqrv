// Needed to provide ASM_LOAD directive
#include <hal_env.h>
.macro save_gprs
        addi sp, sp, -256
        sd ra, 248(sp)
        sd s0, 240(sp)
        sd s1, 232(sp)
        sd s2, 224(sp)
        sd s3, 216(sp)
        sd s4, 208(sp)
        sd s5, 200(sp)
        sd s6, 192(sp)
        sd s7, 184(sp)
        sd s8, 176(sp)
        sd s9, 168(sp)
        sd s10, 160(sp)
        sd s11, 152(sp)

.endm

.macro restore_gprs
        ld ra, 248(sp)
        ld s0, 240(sp) 
        ld s1, 232(sp) 
        ld s2, 224(sp) 
        ld s3, 216(sp) 
        ld s4, 208(sp) 
        ld s5, 200(sp) 
        ld s6, 192(sp) 
        ld s7, 184(sp) 
        ld s8, 176(sp) 
        ld s9, 168(sp) 
        ld s10, 160(sp)
        ld s11, 152(sp)

        addi sp, sp, 256
.endm

// .macro save_vregs
//         sub sp, sp, #(16*4)
//         stp  d8,  d9, [sp, #16*0]
//         stp d10, d11, [sp, #16*1]
//         stp d12, d13, [sp, #16*2]
//         stp d14, d15, [sp, #16*3]
// .endm

// .macro restore_vregs
//         ldp  d8,  d9, [sp, #16*0]
//         ldp d10, d11, [sp, #16*1]
//         ldp d12, d13, [sp, #16*2]
//         ldp d14, d15, [sp, #16*3]
//         add sp, sp, #(16*4)
// .endm

#define STACK_SIZE 1024

.macro push_stack
        save_gprs
        // save_vregs
        addi sp, sp, -STACK_SIZE
.endm

.macro pop_stack
        addi sp, sp, STACK_SIZE
        // restore_vregs
        restore_gprs
.endm

.macro make_ubench name, preamble, code, end_of_iteration
       .p2align 4
       .text
       .global ubench_\name\()
       .global _ubench_\name\()
ubench_\name\():
_ubench_\name\():
        push_stack
        \preamble\()
        .p2align 2
1:
.rept 20
        \code\()
.endr
        \end_of_iteration\()
        pop_stack
        ret
.endm

.macro padding
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
.endm
