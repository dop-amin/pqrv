#include "profiling.h"
#include "prefix_ubenchs.h"

ubench_t ubenchs_prefix[] = {
     &ubench_prefix_0,
     &ubench_prefix_1,
     &ubench_prefix_2,
     &ubench_prefix_3,
     &ubench_prefix_4,
     &ubench_prefix_5,
     &ubench_prefix_6,
     &ubench_prefix_7,
     &ubench_prefix_8,
     &ubench_prefix_9,
     &ubench_prefix_10,
     &ubench_prefix_11,
};

char* ubench_prefix_instructions[] = {
     "sub t2, t3, t1",
     "add t3, t3, t1",
     "mul  t2, t1, t5",
     "srai t2, t2, 32",
     "addi t2, t2, 256",
     "mulh t2, t2, t4",
     "sub t6, t0, t1",
     "add t4, t0, t1",
     "mul  t2, t1, t5",
     "srai t2, t2, 32",
     "addi t2, t2, 256",
     "mulh t2, t2, t4",
};

const unsigned int num_ubenchs_prefix = 12;
