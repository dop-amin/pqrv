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
#include "ntt.h"
#include "params.h"

#define WARMUP_ITERATIONS  1000
#define ITER_PER_TEST      1000
#define TEST_COUNT         100

uint64_t t0, t1;
uint64_t cycles[TEST_COUNT];

/*
 * Test cases
 */

#define MAKE_TEST_NTT(var,func,ref_func,modulus)                            \
int test_ ## var ()                                                         \
{                                                                           \
    int32_t src[NTT_SIZE]      __attribute__((aligned(16)));                \
    int32_t src_copy[NTT_SIZE] __attribute__((aligned(16)));                \
                                                                            \
    /* Setup input */                                                       \
    fill_random_u32( (uint32_t*) src, NTT_SIZE );                           \
    mod_reduce_buf_s32( src, NTT_SIZE, modulus );                           \
                                                                      \
    /* Step 1: Reference NTT */                                             \
    memcpy( src_copy, src, sizeof( src ) );                                 \
    (ref_func)( src_copy);                                                  \
                                                                            \
                                                                            \
    /* Step 2: Optimized NTT */                                             \
    (func)( src );                                                          \
    /* Reduce both buffers mod Q before comparing: NTT outputs are lazy     \
     * (unreduced) and Barrett vs Montgomery pick different representatives, \
     * so compare residue classes, not raw lazy values. */                  \
    mod_reduce_buf_s32_signed( src,      NTT_SIZE, modulus );               \
    mod_reduce_buf_s32_signed( src_copy, NTT_SIZE, modulus );               \
    if( compare_buf_u32( (uint32_t const*) src, (uint32_t const*) src_copy, \
                         NTT_SIZE ) != 0 )                                  \
    {                                                                       \
        debug_print_buf_s32( src_copy, NTT_SIZE, "Reference" );             \
        debug_print_buf_s32( src, NTT_SIZE, "This" );                       \
        debug_test_fail();                                                  \
        return( 1 );                                                        \
    }                                                                       \
    return( 0 );                                                            \
}

// NTT Tests
MAKE_TEST_NTT(ntt_8l_rv64im, ntt_8l_rv64im_wrap, ntt, DILITHIUM_Q)
MAKE_TEST_NTT(ntt_8l_dual_rv64im, ntt_8l_dual_rv64im_wrap, ntt, DILITHIUM_Q)
MAKE_TEST_NTT(ntt_8l_rv64im_opt, ntt_8l_rv64im_opt_wrap, ntt, DILITHIUM_Q)
MAKE_TEST_NTT(ntt_8l_dual_rv64im_opt, ntt_8l_dual_rv64im_opt_wrap, ntt_8l_dual_rv64im_wrap, DILITHIUM_Q)

// INTT Tests - All variants tested against basic non-dual non-optimized implementation
MAKE_TEST_NTT(intt_dilithium_8l_plant_rv64im, intt_dilithium_8l_plant_rv64im_wrap, intt_dilithium_8l_plant_rv64im_wrap, DILITHIUM_Q)
MAKE_TEST_NTT(intt_dilithium_8l_plant_rv64im_dual, intt_dilithium_8l_plant_rv64im_dual_wrap, intt_dilithium_8l_plant_rv64im_wrap, DILITHIUM_Q)
MAKE_TEST_NTT(intt_dilithium_8l_plant_rv64im_opt_c908, intt_dilithium_8l_plant_rv64im_opt_c908_wrap, intt_dilithium_8l_plant_rv64im_wrap, DILITHIUM_Q)
MAKE_TEST_NTT(intt_dilithium_8l_plant_rv64im_dual_opt_c908, intt_dilithium_8l_plant_rv64im_dual_opt_c908_wrap, intt_dilithium_8l_plant_rv64im_wrap, DILITHIUM_Q)

#ifdef VECTOR128
// RVV Tests
MAKE_TEST_NTT(ntt_rvv_vlen128, ntt_rvv_vlen128_wrap, ntt, DILITHIUM_Q)
MAKE_TEST_NTT(ntt_rvv_vlen128_opt_c908, ntt_rvv_vlen128_opt_c908_wrap, ntt_rvv_vlen128_wrap, DILITHIUM_Q)  // tested against the non-optimized rvv ntt (same permuted layout)
MAKE_TEST_NTT(ntt_rvv_vlen128_barret_mul, ntt_rvv_vlen128_barret_mul_wrap, ntt, DILITHIUM_Q)  // tested against non-optimized ntt
MAKE_TEST_NTT(ntt_rvv_vlen128_barret_mul_opt_c908, ntt_rvv_vlen128_barret_mul_opt_c908_wrap, ntt_rvv_vlen128_barret_mul_wrap, DILITHIUM_Q)  // tested against the non-optimized barret mul
MAKE_TEST_NTT(ntt2normal_order_rvv_vlen128_opt_c908, ntt2normal_order_rvv_vlen128_opt_c908_wrap, ntt2normal_order_rvv_vlen128_wrap, DILITHIUM_Q)
MAKE_TEST_NTT(normal2ntt_order_rvv_vlen128_opt_c908, normal2ntt_order_rvv_vlen128_opt_c908_wrap, normal2ntt_order_rvv_vlen128_wrap, DILITHIUM_Q)
#endif // VECTOR128

#define MAKE_BENCH(var, func)                                       \
    int bench_ntt_##var()                                           \
    {                                                               \
        int32_t src[DILITHIUM_N] __attribute__((aligned(16)));      \
                                                                    \
        for (unsigned cnt = 0; cnt < WARMUP_ITERATIONS; cnt++)      \
            (func)(src);                                            \
        init_perf_events();                                         \
        start_counting_events();                                    \
        for (unsigned cnt = 0; cnt < TEST_COUNT; cnt++)             \
        {                                                           \
            for (unsigned cntp = 0; cntp < ITER_PER_TEST; cntp++)   \
                (func)(src);                                        \
        }                                                           \
        stop_and_read_events();                                     \
        calc_average(ITER_PER_TEST, TEST_COUNT);                    \
        cleanup_perf_events();                                      \
        print_counter(#func);                                       \
        return (0);                                                 \
    }

// NTT Benchmarks
MAKE_BENCH(8l_rv64im, ntt_8l_rv64im_wrap);
MAKE_BENCH(8l_dual_rv64im, ntt_8l_dual_rv64im_wrap);
MAKE_BENCH(8l_rv64im_opt, ntt_8l_rv64im_opt_wrap);
MAKE_BENCH(8l_dual_rv64im_opt, ntt_8l_dual_rv64im_opt_wrap);

// INTT Benchmarks
MAKE_BENCH(intt_8l_plant_rv64im, intt_dilithium_8l_plant_rv64im_wrap);
MAKE_BENCH(intt_8l_plant_rv64im_dual, intt_dilithium_8l_plant_rv64im_dual_wrap);
MAKE_BENCH(intt_8l_plant_rv64im_opt_c908, intt_dilithium_8l_plant_rv64im_opt_c908_wrap);
MAKE_BENCH(intt_8l_plant_rv64im_dual_opt_c908, intt_dilithium_8l_plant_rv64im_dual_opt_c908_wrap);

#ifdef VECTOR128
// RVV Benchmarks
MAKE_BENCH(rvv_vlen128, ntt_rvv_vlen128_wrap);
MAKE_BENCH(rvv_vlen128_opt_c908, ntt_rvv_vlen128_opt_c908_wrap);
MAKE_BENCH(rvv_vlen128_barret_mul, ntt_rvv_vlen128_barret_mul_wrap);
MAKE_BENCH(rvv_vlen128_barret_mul_opt_c908, ntt_rvv_vlen128_barret_mul_opt_c908_wrap);
MAKE_BENCH(ntt2normal_order_rvv_vlen128, ntt2normal_order_rvv_vlen128_wrap);
MAKE_BENCH(ntt2normal_order_rvv_vlen128_opt_c908, ntt2normal_order_rvv_vlen128_opt_c908_wrap);
MAKE_BENCH(normal2ntt_order_rvv_vlen128, normal2ntt_order_rvv_vlen128_wrap);
MAKE_BENCH(normal2ntt_order_rvv_vlen128_opt_c908, normal2ntt_order_rvv_vlen128_opt_c908_wrap);
#endif // VECTOR128

int main (void)
{
    int32_t a[256];
    ntt(a);
    /* Test preamble */
    printf("========= Dilithium NTT Test and Benchmarks =========\n");
    printf("function, cycles, instructions, IPC, speedup\n");
    // NTT Tests
    if( test_ntt_8l_rv64im() != 0 ){return( 1 );}
    if( test_ntt_8l_dual_rv64im() != 0 ){return( 1 );}
    if( test_ntt_8l_dual_rv64im_opt() != 0 ){return( 1 );}
    if( test_ntt_8l_rv64im_opt() != 0 ){return( 1 );}

    // INTT Tests
    if( test_intt_dilithium_8l_plant_rv64im() != 0 ){return( 1 );}
    if( test_intt_dilithium_8l_plant_rv64im_dual() != 0 ){return( 1 );}
    if( test_intt_dilithium_8l_plant_rv64im_opt_c908() != 0 ){return( 1 );}
    if( test_intt_dilithium_8l_plant_rv64im_dual_opt_c908() != 0 ){return( 1 );}

    #ifdef VECTOR128
    if( test_ntt_rvv_vlen128_opt_c908() != 0 ){return( 1 );}
    if( test_ntt_rvv_vlen128_barret_mul() != 0){return( 1 );}
    if( test_ntt_rvv_vlen128_barret_mul_opt_c908() != 0){return( 1 );}
    if( test_ntt2normal_order_rvv_vlen128_opt_c908() != 0 ){return( 1 );}
    if( test_normal2ntt_order_rvv_vlen128_opt_c908() != 0 ){return( 1 );}
    #endif // VECTOR128

    // NTT Benchmarks (each naive immediately followed by its optimized variant)
    bench_ntt_8l_rv64im();
    bench_ntt_8l_rv64im_opt();
    bench_ntt_8l_dual_rv64im();
    bench_ntt_8l_dual_rv64im_opt();

    // INTT Benchmarks
    bench_ntt_intt_8l_plant_rv64im();
    bench_ntt_intt_8l_plant_rv64im_opt_c908();
    bench_ntt_intt_8l_plant_rv64im_dual();
    bench_ntt_intt_8l_plant_rv64im_dual_opt_c908();

    #ifdef VECTOR128
    // RVV Benchmarks
    bench_ntt_rvv_vlen128();
    bench_ntt_rvv_vlen128_opt_c908();
    bench_ntt_rvv_vlen128_barret_mul();
    bench_ntt_rvv_vlen128_barret_mul_opt_c908();
    bench_ntt_ntt2normal_order_rvv_vlen128();
    bench_ntt_ntt2normal_order_rvv_vlen128_opt_c908();
    bench_ntt_normal2ntt_order_rvv_vlen128();
    bench_ntt_normal2ntt_order_rvv_vlen128_opt_c908();
    #endif // VECTOR128

    return( 0 );
}
