/*
 * Copyright (c) 2021 Arm Limited
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <hal.h>
#include <string.h>
#include <common.h>
#include <misc.h>
#include <poly_tools.h>
#include "test.h"
#include "perf.h"

#define WARMUP_ITERATIONS  1000
#define ITER_PER_TEST      1000
#define TEST_COUNT         100

uint64_t t0, t1;
uint64_t cycles[TEST_COUNT];

/*
 * Test cases
 */


#define MAKE_BENCH(var, func)                                           \
    int bench_##var()                                                   \
    {                                                                   \
        debug_printf("bench test %-50s", #func "\0");                   \
        for (unsigned cnt = 0; cnt < WARMUP_ITERATIONS; cnt++) {        \
            (func)();\
        }                                                               \
        init_perf_events();                                                                \
        for (unsigned cnt = 0; cnt < TEST_COUNT; cnt++)             \
        { \
            start_counting_events(); \
            for (unsigned cntp = 0; cntp < ITER_PER_TEST; cntp++) {   \
                (func)();                                       \
            } \
            stop_and_read_events(); \
              \
        }                                                           \
                                                                    \
        calc_average(ITER_PER_TEST, TEST_COUNT); \
        return (0);                                                 \
    }

MAKE_BENCH(test, test_opt_c908)



int main (void)
{
    /* Test preamble */
    debug_test_start( "Test!" );

    bench_test();

    return( 0 );
}
