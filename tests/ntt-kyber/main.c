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
#include "kyber.h"
#include "pqrv_paper.h"
#include "perf.h"

#define WARMUP_ITERATIONS  1000
#define ITER_PER_TEST      1000
#define TEST_COUNT         100

uint64_t t0, t1;
uint64_t cycles[TEST_COUNT];

/*
 * Test cases
 *
 * The Kyber NTT operates on int16_t arrays. We use the baseline RVV
 * implementation (ntt_kyber_rvv_vlen128_wrap) as the reference and check
 * that the optimized variant produces an identical result on the same
 * input and the same zetas/mask table.
 */

#define MAKE_TEST_NTT(var,func,ref_func,modulus)                            \
int test_ ## var ()                                                         \
{                                                                           \
    debug_printf("Test for " #func " ");                                    \
    int16_t src[NTT_SIZE]      __attribute__((aligned(16)));                \
    int16_t src_copy[NTT_SIZE] __attribute__((aligned(16)));                \
                                                                            \
    /* Setup input */                                                       \
    fill_random_u16( (uint16_t*) src, NTT_SIZE );                           \
    mod_reduce_buf_s16( src, NTT_SIZE, modulus );                           \
                                                                            \
    /* Step 1: Reference NTT */                                             \
    memcpy( src_copy, src, sizeof( src ) );                                 \
    ref_func( src_copy );                                                   \
                                                                            \
    /* Step 2: Optimized NTT */                                             \
    (func)( src );                                                          \
                                                                            \
    if( compare_buf_u16( (uint16_t const*) src, (uint16_t const*) src_copy, \
                         NTT_SIZE ) != 0 )                                  \
    {                                                                       \
        debug_print_buf_s16( src_copy, NTT_SIZE, "Reference" );             \
        debug_print_buf_s16( src, NTT_SIZE, "This" );                       \
        debug_test_fail();                                                  \
        return( 1 );                                                        \
    }                                                                       \
    debug_test_ok();                                                        \
                                                                            \
    return( 0 );                                                            \
}

// RVV Tests - the unoptimized (baseline) version acts as the reference
MAKE_TEST_NTT(ntt_kyber_rvv_vlen128,           ntt_kyber_rvv_vlen128_wrap,           ntt_kyber_rvv_vlen128_wrap, KYBER_Q)
MAKE_TEST_NTT(ntt_kyber_rvv_vlen128_opt_c908,  ntt_kyber_rvv_vlen128_opt_c908_wrap,  ntt_kyber_rvv_vlen128_wrap, KYBER_Q)

#define MAKE_BENCH(var, func)                                       \
    int bench_ntt_##var()                                           \
    {                                                               \
        debug_printf("bench ntt_kyber %-50s", #func "\0");          \
        int16_t src[KYBER_N] __attribute__((aligned(16)));          \
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
        print_counter();                                            \
        return (0);                                                 \
    }

// RVV Benchmarks
MAKE_BENCH(rvv_vlen128,          ntt_kyber_rvv_vlen128_wrap);
MAKE_BENCH(rvv_vlen128_opt_c908, ntt_kyber_rvv_vlen128_opt_c908_wrap);


/* Initialize the shuffle-mask region of the zetas table used by the Kyber
 * RVV NTT. Without correct masks, the vrgather-based shuffles produce
 * degenerate output that depends on the instruction ordering, so the
 * baseline and the slothy-optimized variant would not necessarily agree.
 *
 * Offsets and contents are taken directly from the assembly header
 * (KYBER_NTT_RVV_VLEN128_CONSTS_H). The _ZETAS_EXP region is left at
 * zero, which keeps the NTT deterministic and identical between the two
 * implementations while still exercising the full instruction sequence. */
static void init_kyber_ntt_table(void)
{
    static const int16_t mask_45674567[8] = {4,5,6,7,4,5,6,7};
    static const int16_t mask_01230123[8] = {0,1,2,3,0,1,2,3};
    static const int16_t mask_01014545[8] = {0,1,0,1,4,5,4,5};
    static const int16_t mask_23236767[8] = {2,3,2,3,6,7,6,7};
    static const int16_t mask_10325476[8] = {1,0,3,2,5,4,7,6};

    /* The named offsets (e.g., _MASK_01230123 = 8) are byte/2 offsets:
     * the assembly accesses them as a1 + offset*2, so the int16_t index
     * is identical to the offset value. */
    memcpy( &zetas[ 0], mask_45674567, sizeof(mask_45674567) );
    memcpy( &zetas[ 8], mask_01230123, sizeof(mask_01230123) );
    memcpy( &zetas[16], mask_01014545, sizeof(mask_01014545) );
    memcpy( &zetas[24], mask_23236767, sizeof(mask_23236767) );
    memcpy( &zetas[32], mask_10325476, sizeof(mask_10325476) );
}

int main (void)
{
    /* Test preamble */
    debug_test_start( "NTT Kyber!" );
    init_kyber_ntt_table();

    // RVV Tests
    if( test_ntt_kyber_rvv_vlen128() != 0 ){return( 1 );}
    if( test_ntt_kyber_rvv_vlen128_opt_c908() != 0 ){return( 1 );}

    debug_printf("Starting benchmarks...\n");

    // RVV Benchmarks
    bench_ntt_rvv_vlen128();
    bench_ntt_rvv_vlen128_opt_c908();

    debug_printf("Test Success!");
    return( 0 );
}
