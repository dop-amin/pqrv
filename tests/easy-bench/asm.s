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

.macro pre_pad
    nop
    nop
    nop
    nop
.endm

.macro post_pad
    nop
    nop
    nop
    nop
.endm

.align 4
.text

.type empty_func, %function
.global empty_func
empty_func:
    save_gprs

    .rept 1
    pre_pad
    post_pad
    .endr

    restore_gprs
    ret

.type asm_func, %function
.global asm_func
asm_func:
    save_gprs

    pre_pad
    /* CODE */
.rept 50
    add x30, x30, x30
    add x29, x29, x29
    add x30, x30, x30
    add x29, x29, x29
    add x30, x30, x30
    add x29, x29, x29
    add x30, x30, x30
    add x29, x29, x29
    
    add x5, x5, x5
    add x7, x7, x7
    mul x9, x5, x10
    add x8, x8, x8
.endr
    post_pad

    restore_gprs
    ret