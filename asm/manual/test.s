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

.globl test_opt_c908
.align 2
test_opt_c908:
    addi sp, sp, -8*15
    save_regs
    nop
    nop
    nop
    nop
    vsetivli t2, 4, e32, m1, tu, mu
    .rept 100
      //add x3, x4, x5
      vadd.vv v2, v3, v4
      //add x6, x7, x8
      vadd.vv v5, v6, v7
      //add x9, x10, x11
      vadd.vv v8, v9, v10
      //add x12, x13, x14
      vadd.vv v11, v12, v13
      vadd.vv v14, v15, v16
      vadd.vv v17, v18, v19
      vadd.vv v20, v21, v22
      vadd.vv v23, v24, v25
    .endr
    nop
    nop
    nop
    nop
restore_regs
addi sp, sp, 8*15
ret
