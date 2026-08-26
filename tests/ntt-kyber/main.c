/*
 * Copyright (c) 2026 Justus Bergermann
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

#define MAKE_TEST_NTT(var,func,ref_func,modulus)                            \
int test_ ## var ()                                                         \
{                                                                           \
    int16_t src[NTT_SIZE]      __attribute__((aligned(16)));                \
    int16_t src_copy[NTT_SIZE] __attribute__((aligned(16)));                \
                                                                            \
    fill_random_u16( (uint16_t*) src, NTT_SIZE );                           \
    mod_reduce_buf_s16( src, NTT_SIZE, modulus );                           \
                                                                            \
    memcpy( src_copy, src, sizeof( src ) );                                 \
    ref_func( src_copy );                                                   \
                                                                            \
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
                                                                            \
    return( 0 );                                                            \
}


#define MAKE_BENCH(var, func)                                       \
    int bench_##var()                                               \
    {                                                               \
        int16_t src[KYBER_N] __attribute__((aligned(16)));          \
                                                                    \
        fill_random_u16( (uint16_t*) src, KYBER_N );                 \
        mod_reduce_buf_s16( src, KYBER_N, KYBER_Q );                 \
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
        print_counter(#func);                                            \
        return (0);                                                 \
    }

/* === Equivalence tests: naive vs. its own _opt_c908 kernel === */

/* RV64IM scalar forward NTT, single-issue schedule */
MAKE_TEST_NTT(ntt_kyber_rv64im_opt_c908,         ntt_kyber_rv64im_opt_c908_wrap,         ntt_kyber_rv64im_wrap,         KYBER_Q)

/* RV64IM scalar forward NTT, dual-issue schedule */
MAKE_TEST_NTT(ntt_kyber_dual_rv64im_opt_c908,    ntt_kyber_dual_rv64im_opt_c908_wrap,    ntt_kyber_dual_rv64im_wrap,    KYBER_Q)
//MAKE_TEST_NTT(ntt_kyber_dual_l32_rv64im_opt_c908, ntt_kyber_dual_l32_rv64im_opt_c908_wrap, ntt_kyber_dual_l32_rv64im_wrap, KYBER_Q)  // skipped for now

/* RV64IM scalar INTT, single-issue schedule */
MAKE_TEST_NTT(intt_kyber_rv64im_opt_c908,      intt_kyber_rv64im_opt_c908_wrap,      intt_kyber_rv64im_wrap,      KYBER_Q)

/* RV64IM scalar INTT, dual-issue schedule (own naive kernel!) */
MAKE_TEST_NTT(intt_kyber_dual_rv64im_opt_c908, intt_kyber_dual_rv64im_opt_c908_wrap, intt_kyber_dual_rv64im_wrap, KYBER_Q)

#ifdef VECTOR128
/* RVV forward NTT */
MAKE_TEST_NTT(ntt_kyber_rvv_vlen128_opt_c908,  ntt_kyber_rvv_vlen128_opt_c908_wrap,  ntt_kyber_rvv_vlen128_wrap,  KYBER_Q)

/* RVV inverse NTT */
MAKE_TEST_NTT(intt_kyber_rvv_vlen128_opt_c908, intt_kyber_rvv_vlen128_opt_c908_wrap, intt_kyber_rvv_vlen128_wrap, KYBER_Q)

/* NTT reordering helpers */

MAKE_TEST_NTT(kyber_normal2ntt_order_rvv_vlen128_opt_c908, kyber_normal2ntt_order_rvv_vlen128_opt_c908_wrap, kyber_normal2ntt_order_rvv_vlen128_wrap, KYBER_Q)
MAKE_TEST_NTT(kyber_ntt2normal_order_rvv_vlen128_opt_c908, kyber_ntt2normal_order_rvv_vlen128_opt_c908_wrap, kyber_ntt2normal_order_rvv_vlen128_wrap, KYBER_Q)

#endif /* VECTOR128 */

/* === Benchmarks === */

/* RV64IM scalar single NTT */
MAKE_BENCH(ntt_kyber_rv64im, ntt_kyber_rv64im_wrap);
MAKE_BENCH(ntt_kyber_rv64im_opt_c908, ntt_kyber_rv64im_opt_c908_wrap);

/* RV64IM scalar dual NTT */
MAKE_BENCH(ntt_kyber_dual_rv64im, ntt_kyber_dual_rv64im_wrap);
MAKE_BENCH(ntt_kyber_dual_rv64im_opt_c908, ntt_kyber_dual_rv64im_opt_c908_wrap);

/* RV64IM scalar single INTT */
MAKE_BENCH(intt_kyber_rv64im, intt_kyber_rv64im_wrap);
MAKE_BENCH(intt_kyber_rv64im_opt_c908, intt_kyber_rv64im_opt_c908_wrap);

/* RV64IM scalar dual INTT */
MAKE_BENCH(intt_kyber_dual_rv64im, intt_kyber_dual_rv64im_wrap);
MAKE_BENCH(intt_kyber_dual_rv64im_opt_c908, intt_kyber_dual_rv64im_opt_c908_wrap);

#ifdef VECTOR128
/* RVV forward / inverse NTT */
MAKE_BENCH(ntt_kyber_rvv_vlen128, ntt_kyber_rvv_vlen128_wrap);
MAKE_BENCH(ntt_kyber_rvv_vlen128_opt_c908, ntt_kyber_rvv_vlen128_opt_c908_wrap);
MAKE_BENCH(intt_kyber_rvv_vlen128, intt_kyber_rvv_vlen128_wrap);
MAKE_BENCH(intt_kyber_rvv_vlen128_opt_c908, intt_kyber_rvv_vlen128_opt_c908_wrap);

/* RVV order conversion */
MAKE_BENCH(kyber_normal2ntt_order_rvv_vlen128, kyber_normal2ntt_order_rvv_vlen128_wrap);
MAKE_BENCH(kyber_ntt2normal_order_rvv_vlen128, kyber_ntt2normal_order_rvv_vlen128_wrap);
MAKE_BENCH(kyber_ntt2normal_order_rvv_vlen128_opt_c908, kyber_ntt2normal_order_rvv_vlen128_opt_c908_wrap);
MAKE_BENCH(kyber_normal2ntt_order_rvv_vlen128_opt_c908, kyber_normal2ntt_order_rvv_vlen128_opt_c908_wrap);
#endif /* VECTOR128 */

int main (void)
{
    int rc = 0;

    printf("========= Kyber NTT Test and Benchmarks =========\n");
    printf("function, cycles, instructions, IPC, speedup\n");

    /* RV64IM equivalence tests */
    rc |= test_ntt_kyber_rv64im_opt_c908();
    rc |= test_ntt_kyber_dual_rv64im_opt_c908();
    //rc |= test_ntt_kyber_dual_l32_rv64im_opt_c908();  // skipped for now
    rc |= test_intt_kyber_rv64im_opt_c908();
    rc |= test_intt_kyber_dual_rv64im_opt_c908();

#ifdef VECTOR128
    /* RVV equivalence tests */
    rc |= test_ntt_kyber_rvv_vlen128_opt_c908();
    rc |= test_intt_kyber_rvv_vlen128_opt_c908();
    rc |= test_kyber_ntt2normal_order_rvv_vlen128_opt_c908();
    rc |= test_kyber_normal2ntt_order_rvv_vlen128_opt_c908();
#endif


    /* RV64IM scalar single NTT benchmarks */
    bench_ntt_kyber_rv64im();
    bench_ntt_kyber_rv64im_opt_c908();

    /* RV64IM scalar dual NTT benchmarks */
    bench_ntt_kyber_dual_rv64im();
    bench_ntt_kyber_dual_rv64im_opt_c908();

    /* RV64IM scalar single INTT benchmarks */
    bench_intt_kyber_rv64im();
    bench_intt_kyber_rv64im_opt_c908();

     /* RV64IM scalar dual INTT benchmarks */
    bench_intt_kyber_dual_rv64im();
    bench_intt_kyber_dual_rv64im_opt_c908();

#ifdef VECTOR128
    /* RVV benchmarks */
    bench_ntt_kyber_rvv_vlen128();
    bench_ntt_kyber_rvv_vlen128_opt_c908();
    bench_intt_kyber_rvv_vlen128();
    bench_intt_kyber_rvv_vlen128_opt_c908();
    bench_kyber_normal2ntt_order_rvv_vlen128();
    bench_kyber_normal2ntt_order_rvv_vlen128_opt_c908();
    bench_kyber_ntt2normal_order_rvv_vlen128();
    bench_kyber_ntt2normal_order_rvv_vlen128_opt_c908();
#endif

    return rc;
}
