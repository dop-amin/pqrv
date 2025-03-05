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
    // raw boilerplate code: 38 cycles
    addi sp, sp, -8*15
    save_regs
    nop
    nop
    nop
    nop
    .rept 1000

                                 // Instructions:    6
                                 // Expected cycles: 3
                                 // Expected IPC:    2.00
                                 //
                                 // Cycle bound:     3.0
                                 // IPC bound:       2.00
                                 //
                                 // Wall time:     0.01s
                                 // User time:     0.01s
                                 //
                                 // ----- cycle (expected) ------>
                                 // 0                        25
                                 // |------------------------|----
        addi x21, x14, 64        // *.............................
        addi x22, x15, 64        // *.............................
        addi x23, x16, 64        // .*............................
        addi x24, x17, 64        // .*............................
        addi x25, x18, 64        // ..*...........................
        addi x26, x19, 64        // ..*...........................
        mul x27, x14, x15

                                  // ------ cycle (expected) ------>
                                  // 0                        25
                                  // |------------------------|-----
        // addi x21, x14, 64      // *..............................
        // addi x22, x15, 64      // *..............................
        // addi x23, x16, 64      // .*.............................
        // addi x24, x17, 64      // .*.............................
        // addi x25, x18, 64      // ..*............................
        // addi x26, x19, 64      // ..*............................



    .endr
    nop
    nop
    nop
    nop
    restore_regs
    addi sp, sp, 8*15
    ret