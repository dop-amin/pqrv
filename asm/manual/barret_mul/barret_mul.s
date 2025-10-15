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

.globl barret_mul
.align 2
// uint64_t barrett_mul(uint64_t a, uint64_t b, uint64_t n,
//                      unsigned k, uint64_t C)
// Arguments:
//   a0 = a
//   a1 = b
//   a2 = n
//   a3 = k
//   a4 = C
// Returns:
//   a0 = result
barret_mul:
    addi sp, sp, -8*15
    save_regs
    // z = a*b
    mul t0, a0, a1

    // t = (a * C) >> k
    mul t1, a0, a4
    srl t1, t1, a3

    // r = z - n * t
    mul t2, a2, t1        // n * t
    sub a0, t0, t2        // r = z - n*t

    restore_regs
    addi sp, sp, 8*15
    ret