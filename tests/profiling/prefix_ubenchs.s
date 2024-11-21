#include "ubenchmarks.i"
#include "profiler_macros.i"

.macro prefix_0_core
padding
.endm
make_ubench prefix_0, nop, prefix_0_core, nop

.macro prefix_1_core
/* 00 */     sub t2, t3, t1
padding
.endm
make_ubench prefix_1, nop, prefix_1_core, nop

.macro prefix_2_core
/* 00 */     sub t2, t3, t1
/* 01 */     add t3, t3, t1
padding
.endm
make_ubench prefix_2, nop, prefix_2_core, nop

.macro prefix_3_core
/* 00 */     sub t2, t3, t1
/* 01 */     add t3, t3, t1
/* 02 */     mul  t2, t1, t5
padding
.endm
make_ubench prefix_3, nop, prefix_3_core, nop

.macro prefix_4_core
/* 00 */     sub t2, t3, t1
/* 01 */     add t3, t3, t1
/* 02 */     mul  t2, t1, t5
/* 03 */     srai t2, t2, 32
padding
.endm
make_ubench prefix_4, nop, prefix_4_core, nop

.macro prefix_5_core
/* 00 */     sub t2, t3, t1
/* 01 */     add t3, t3, t1
/* 02 */     mul  t2, t1, t5
/* 03 */     srai t2, t2, 32
/* 04 */     addi t2, t2, 256
padding
.endm
make_ubench prefix_5, nop, prefix_5_core, nop

.macro prefix_6_core
/* 00 */     sub t2, t3, t1
/* 01 */     add t3, t3, t1
/* 02 */     mul  t2, t1, t5
/* 03 */     srai t2, t2, 32
/* 04 */     addi t2, t2, 256
/* 05 */     mulh t2, t2, t4
padding
.endm
make_ubench prefix_6, nop, prefix_6_core, nop

.macro prefix_7_core
/* 00 */     sub t2, t3, t1
/* 01 */     add t3, t3, t1
/* 02 */     mul  t2, t1, t5
/* 03 */     srai t2, t2, 32
/* 04 */     addi t2, t2, 256
/* 05 */     mulh t2, t2, t4
/* 06 */     sub t6, t0, t1
padding
.endm
make_ubench prefix_7, nop, prefix_7_core, nop

.macro prefix_8_core
/* 00 */     sub t2, t3, t1
/* 01 */     add t3, t3, t1
/* 02 */     mul  t2, t1, t5
/* 03 */     srai t2, t2, 32
/* 04 */     addi t2, t2, 256
/* 05 */     mulh t2, t2, t4
/* 06 */     sub t6, t0, t1
/* 07 */     add t4, t0, t1
padding
.endm
make_ubench prefix_8, nop, prefix_8_core, nop

.macro prefix_9_core
/* 00 */     sub t2, t3, t1
/* 01 */     add t3, t3, t1
/* 02 */     mul  t2, t1, t5
/* 03 */     srai t2, t2, 32
/* 04 */     addi t2, t2, 256
/* 05 */     mulh t2, t2, t4
/* 06 */     sub t6, t0, t1
/* 07 */     add t4, t0, t1
/* 08 */     mul  t2, t1, t5
padding
.endm
make_ubench prefix_9, nop, prefix_9_core, nop

.macro prefix_10_core
/* 00 */     sub t2, t3, t1
/* 01 */     add t3, t3, t1
/* 02 */     mul  t2, t1, t5
/* 03 */     srai t2, t2, 32
/* 04 */     addi t2, t2, 256
/* 05 */     mulh t2, t2, t4
/* 06 */     sub t6, t0, t1
/* 07 */     add t4, t0, t1
/* 08 */     mul  t2, t1, t5
/* 09 */     srai t2, t2, 32
padding
.endm
make_ubench prefix_10, nop, prefix_10_core, nop

.macro prefix_11_core
/* 00 */     sub t2, t3, t1
/* 01 */     add t3, t3, t1
/* 02 */     mul  t2, t1, t5
/* 03 */     srai t2, t2, 32
/* 04 */     addi t2, t2, 256
/* 05 */     mulh t2, t2, t4
/* 06 */     sub t6, t0, t1
/* 07 */     add t4, t0, t1
/* 08 */     mul  t2, t1, t5
/* 09 */     srai t2, t2, 32
/* 10 */     addi t2, t2, 256
padding
.endm
make_ubench prefix_11, nop, prefix_11_core, nop
