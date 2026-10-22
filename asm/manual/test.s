.macro save_regs
  sd s0,  0*8(sp)
  sd s1,  1*8(sp)
  sd s2,  2*8(sp)
  sd s3,  3*8(sp)
  sd s4,  4*8(sp)
  sd s5,  5*8(sp)
  sd s6,  6*8(sp)
  sd s7,  7*8(sp)
  sd s8,  8*8(sp)
  sd s9,  9*8(sp)
  sd s10, 10*8(sp)
  sd s11, 11*8(sp)
  sd gp,  12*8(sp)
  sd tp,  13*8(sp)
  sd ra,  14*8(sp)
.endm

.macro restore_regs
  ld s0,  0*8(sp)
  ld s1,  1*8(sp)
  ld s2,  2*8(sp)
  ld s3,  3*8(sp)
  ld s4,  4*8(sp)
  ld s5,  5*8(sp)
  ld s6,  6*8(sp)
  ld s7,  7*8(sp)
  ld s8,  8*8(sp)
  ld s9,  9*8(sp)
  ld s10, 10*8(sp)
  ld s11, 11*8(sp)
  ld gp,  12*8(sp)
  ld tp,  13*8(sp)
  ld ra,  14*8(sp)
.endm

.data
.align 3
mul_ops:
  //.dword 0x123456789abcdef1          // a1: full 64-bit
  //.dword 0x0fedcba987654321          // a2: full 64-bit
  .dword 1234                        // a1: 16-bit coefficient
  .dword 0x3c0f12886ba8f301          // a2: 64-bit constant (qinv), as in plant_mul_const

.text
.globl test_opt_c908
.align 2
test_opt_c908:
    addi sp, sp, -8*15
    save_regs
    la a0, mul_ops
    ld a1, 0(a0)
    ld a2, 8(a0)
    nop
    nop
    nop
    nop
    .rept 12
      mul s0, a1, a2
      mul s1, a1, a2
      mul s2, a1, a2
      mul s3, a1, a2
      mul s4, a1, a2
      mul s5, a1, a2
      mul s6, a1, a2
      mul s7, a1, a2
    .endr
    nop
    nop
    nop
    nop
    restore_regs
    addi sp, sp, 8*15
    ret
