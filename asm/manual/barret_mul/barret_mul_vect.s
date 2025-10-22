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

.globl barret_mul_vect
.align 2
// uint64_t void barret_mul_vect(void* a, void* b, uint64_t n,
//          void* c, void* result)
// Arguments:
//   a0 = a*
//   a1 = b*
//   a2 = n
//   a3 = c*
//   a4 = result*
// Returns:
//   void (result in a4)
barret_mul_vect:
    li a5, 4
    vsetvli t0, a5, e64, m1  // 4*64 bit elements per vector

    vle64.v v0, (a0)  // load vector a
    vle64.v v1, (a1)  // load vector b
    vle64.v v3, (a3)  // load vector c

    // z = a*b
    vmul.vv v2, v0, v1

    // t = (a * C) >> k
    vmulhu.vv v4, v0, v3

    // r = z - n * t
    vmul.vx v6, v4, a2        // t*n
    vsub.vv v7, v2, v6        // r = z - n*t
    vse64.v v7, (a4)  // store result
    ret