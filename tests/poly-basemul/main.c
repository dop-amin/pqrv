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
#include "dilithium.h"
#include "pqrv_paper.h"
#include "perf.h"

#define WARMUP_ITERATIONS  1000
#define ITER_PER_TEST      1000
#define TEST_COUNT         100

uint64_t t0, t1;
uint64_t cycles[TEST_COUNT];

/*
 * Test cases
 */

#define MAKE_TEST_POLY_BASEMUL(var,func,ref_func,modulus)                            \
int test_ ## var ()                                                         \
{                                                                           \
    /* debug_test_start( "Test for " #func );*/                             \
    debug_printf("Test for " #func " ");                                    \
    int32_t src[NTT_SIZE]      __attribute__((aligned(16)));                \
    int32_t src_copy[NTT_SIZE] __attribute__((aligned(16)));                \
                                                                            \
    /* Setup input */                                                       \
    fill_random_u32( (uint32_t*) src, NTT_SIZE );                           \
    mod_reduce_buf_s32( src, NTT_SIZE, modulus );                           \
                                                                            \
    /* Step 1: Reference NTT */                                             \
    memcpy( src_copy, src, sizeof( src ) );                                 \
    ref_func( src_copy);                                                    \
                                                                            \
                                                                            \
    /* Step 2: Optimized NTT */                                             \
    (func)( src );                                                          \
                                                                            \
    if( compare_buf_u32( (uint32_t const*) src, (uint32_t const*) src_copy, \
                         NTT_SIZE ) != 0 )                                  \
    {                                                                       \
        debug_print_buf_s32( src_copy, NTT_SIZE, "Reference" );             \
        debug_print_buf_s32( src, NTT_SIZE, "This" );                       \
        debug_test_fail();                                                  \
        return( 1 );                                                        \
    }                                                                       \
    debug_test_ok();                                                        \
                                                                            \
    return( 0 );                                                            \
}

//MAKE_TEST_POLY_BASEMUL(poly_basemul_8l_acc_rv64im, poly_basemul_8l_acc_rv64im_wrap, poly_basemul_8l_acc_rv64im_wrap, DILITHIUM_Q)
MAKE_TEST_POLY_BASEMUL(poly_basemul_8l_acc_dual_rv64im, poly_basemul_8l_acc_dual_rv64im_wrap, poly_basemul_8l_acc_rv64im_wrap, DILITHIUM_Q)
MAKE_TEST_POLY_BASEMUL(poly_basemul_8l_acc_rv64im_opt, poly_basemul_8l_acc_rv64im_opt_wrap, poly_basemul_8l_acc_rv64im_wrap, DILITHIUM_Q)

#define MAKE_BENCH(var, func)                                           \
    int bench_##var()                                                   \
    {                                                                   \
        debug_printf("bench poly_basemul %-50s", #func "\0");      \
        int32_t src[DILITHIUM_N] __attribute__((aligned(16)));         \
        \
        for (unsigned cnt = 0; cnt < WARMUP_ITERATIONS; cnt++) {        \
            (func)(src);\
        }                                                               \
        init_perf_events(); \
        start_counting_events(); \
        for (unsigned cnt = 0; cnt < TEST_COUNT; cnt++)             \
        { \
            for (unsigned cntp = 0; cntp < ITER_PER_TEST; cntp++) {   \
                (func)(src);                                       \
            } \
              \
        }                                                           \
        stop_and_read_events(); \
        calc_average(ITER_PER_TEST, TEST_COUNT); \
        cleanup_perf_events(); \
        print_counter(); \
        return (0);                                                 \
    }

//MAKE_BENCH(poly_basemul_8l_acc_rv64im, poly_basemul_8l_acc_rv64im_wrap)
//MAKE_BENCH(poly_basemul_8l_acc_dual_rv64im, poly_basemul_8l_acc_dual_rv64im_wrap)
MAKE_BENCH(poly_basemul_8l_acc_rv64im_opt, poly_basemul_8l_acc_rv64im_opt_wrap)


int main (void)
{
    /* Test preamble */
    debug_test_start( "Poly basemul!" );

    //if( test_poly_basemul_8l_acc_rv64im() != 0 ) return 1 ;
    //if( test_poly_basemul_8l_acc_dual_rv64im() != 0) return 1;
    if( test_poly_basemul_8l_acc_rv64im_opt() != 0 ) return 1 ;

    //bench_poly_basemul_8l_acc_rv64im();
    //bench_poly_basemul_8l_acc_dual_rv64im();
    bench_poly_basemul_8l_acc_rv64im_opt();

    return( 0 );
}
