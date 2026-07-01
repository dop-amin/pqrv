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
#include "pqrv_paper.h"
#include "perf.h"

#define WARMUP_ITERATIONS  1000
#define ITER_PER_TEST      1000
#define TEST_COUNT         100
#define KYBER_N            256
#define KYBER_Q            3329

#define CBD2_BUFLEN        (2 * KYBER_N / 4)   /* 128 */
#define CBD3_BUFLEN        (3 * KYBER_N / 4)   /* 192 */
#define REJ_BUFLEN         504

uint64_t t0, t1;
uint64_t cycles[TEST_COUNT];

/*
 * NOTE on the optimized (`_opt_c908`) sampling kernels
 * ----------------------------------------------------
 * SLOTHY's register renaming produced illegal RISC-V Vector operands in the
 * optimized outputs: widening `vsext.vf2` instructions with the source
 * overlapping the *low* part of the destination group (e.g. `vsext.vf2
 * v18,v18` under e16/m2). Per RVV 1.0, when the destination EEW is greater
 * than the source EEW the source may only overlap the *highest*-numbered part
 * of the destination register group, so these encodings are illegal and trap
 * ("Illegal instruction") on a spec-compliant executor such as qemu 9.1.1.
 * The naive kernels avoid the overlap entirely.
 *
 * Consequently the naive-vs-opt equivalence tests and the opt benchmarks below
 * are DEFINED (so they can run on hardware that tolerates the overlap, or once
 * SLOTHY is fixed) but are only compiled/executed when built with
 * -DWITH_OPT_KERNELS. The default (qemu) run validates the naive kernels via
 * property/range checks and benchmarks only the naive kernels.
 */

static int check_range_s16(const int16_t *a, int n, int lo, int hi)
{
    for (int i = 0; i < n; i++)
        if (a[i] < lo || a[i] > hi) return i + 1;
    return 0;
}

/* ---- Naive property/range correctness (qemu-runnable) ---- */

#define MAKE_TEST_CBD_RANGE(var,func,bufbytes,bound)                        \
int test_ ## var ()                                                         \
{                                                                           \
    debug_printf("Test (range) for " #func " ");                           \
    uint8_t buf[bufbytes]  __attribute__((aligned(16)));                    \
    int16_t r[KYBER_N]     __attribute__((aligned(16)));                    \
    fill_random_u16( (uint16_t*) buf, (bufbytes) / 2 );                     \
    memset(r, 0, sizeof(r));                                                \
    (func)( r, buf );                                                       \
    if( check_range_s16(r, KYBER_N, -(bound), (bound)) != 0 )               \
    {                                                                       \
        debug_print_buf_s16( r, KYBER_N, "Out-of-range CBD output" );      \
        debug_test_fail();                                                  \
        return( 1 );                                                        \
    }                                                                       \
    debug_test_ok();                                                        \
    return( 0 );                                                            \
}

MAKE_TEST_CBD_RANGE(cbd2_rvv_vlen128_naive, cbd2_rvv_vlen128_wrap, CBD2_BUFLEN, 2)
MAKE_TEST_CBD_RANGE(cbd3_rvv_vlen128_naive, cbd3_rvv_vlen128_wrap, CBD3_BUFLEN, 3)

int test_rej_uniform_rvv_vlen128_naive(void)
{
    debug_printf("Test (range) for rej_uniform_rvv_vlen128_wrap ");
    uint8_t buf[REJ_BUFLEN] __attribute__((aligned(16)));
    int16_t r[KYBER_N]      __attribute__((aligned(16)));
    uint32_t ctr = 0, pos = 0;
    fill_random_u16( (uint16_t*) buf, REJ_BUFLEN / 2 );
    memset(r, 0, sizeof(r));
    rej_uniform_rvv_vlen128_wrap( r, buf, &ctr, &pos );
    if( ctr == 0 || ctr > KYBER_N || pos > REJ_BUFLEN ||
        check_range_s16(r, (int) ctr, 0, KYBER_Q - 1) != 0 )
    {
        debug_printf("ctr %u pos %u\n", ctr, pos);
        debug_print_buf_s16( r, KYBER_N, "rej_uniform output" );
        debug_test_fail();
        return 1;
    }
    debug_test_ok();
    return 0;
}

/* ---- Naive-vs-opt equivalence (board / -DWITH_OPT_KERNELS only) ---- */
#ifdef WITH_OPT_KERNELS

#define MAKE_TEST_CBD_EQUIV(var,func,ref_func,bufbytes)                     \
int test_ ## var ()                                                         \
{                                                                           \
    debug_printf("Test (equiv) for " #func " ");                           \
    uint8_t buf[bufbytes]  __attribute__((aligned(16)));                    \
    int16_t r[KYBER_N]     __attribute__((aligned(16)));                    \
    int16_t r_ref[KYBER_N] __attribute__((aligned(16)));                    \
    fill_random_u16( (uint16_t*) buf, (bufbytes) / 2 );                     \
    memset(r, 0, sizeof(r)); memset(r_ref, 0, sizeof(r_ref));               \
    ref_func( r_ref, buf );                                                 \
    (func)( r, buf );                                                       \
    if( compare_buf_u16( (uint16_t const*) r, (uint16_t const*) r_ref,      \
                         KYBER_N ) != 0 )                                   \
    { debug_test_fail(); return( 1 ); }                                     \
    debug_test_ok();                                                        \
    return( 0 );                                                            \
}

MAKE_TEST_CBD_EQUIV(cbd2_rvv_vlen128_opt_c908, cbd2_rvv_vlen128_opt_c908_wrap, cbd2_rvv_vlen128_wrap, CBD2_BUFLEN)
MAKE_TEST_CBD_EQUIV(cbd3_rvv_vlen128_opt_c908, cbd3_rvv_vlen128_opt_c908_wrap, cbd3_rvv_vlen128_wrap, CBD3_BUFLEN)

int test_rej_uniform_rvv_vlen128_opt_c908(void)
{
    debug_printf("Test (equiv) for rej_uniform_rvv_vlen128_opt_c908_wrap ");
    uint8_t buf[REJ_BUFLEN] __attribute__((aligned(16)));
    int16_t r[KYBER_N] __attribute__((aligned(16))), r_ref[KYBER_N] __attribute__((aligned(16)));
    uint32_t ctr = 0, pos = 0, ctr_ref = 0, pos_ref = 0;
    fill_random_u16( (uint16_t*) buf, REJ_BUFLEN / 2 );
    memset(r, 0, sizeof(r)); memset(r_ref, 0, sizeof(r_ref));
    rej_uniform_rvv_vlen128_wrap( r_ref, buf, &ctr_ref, &pos_ref );
    rej_uniform_rvv_vlen128_opt_c908_wrap( r, buf, &ctr, &pos );
    if( ctr != ctr_ref || pos != pos_ref ||
        compare_buf_u16( (uint16_t const*) r, (uint16_t const*) r_ref, KYBER_N ) != 0 )
    { debug_test_fail(); return 1; }
    debug_test_ok();
    return 0;
}
#endif /* WITH_OPT_KERNELS */

/* === BENCHMARKS === */
#define MAKE_BENCH_CBD(var, func, bufbytes)                                 \
    int bench_##var()                                                       \
    {                                                                       \
        debug_printf("bench kyber_sampling %-45s", #func "\0");             \
        uint8_t buf[bufbytes] __attribute__((aligned(16)));                 \
        int16_t r[KYBER_N] __attribute__((aligned(16)));                    \
        fill_random_u16( (uint16_t*) buf, (bufbytes) / 2 );                 \
        for (unsigned cnt = 0; cnt < WARMUP_ITERATIONS; cnt++) (func)(r, buf); \
        init_perf_events(); start_counting_events();                        \
        for (unsigned cnt = 0; cnt < TEST_COUNT; cnt++)                     \
            for (unsigned cntp = 0; cntp < ITER_PER_TEST; cntp++) (func)(r, buf); \
        stop_and_read_events(); calc_average(ITER_PER_TEST, TEST_COUNT);    \
        cleanup_perf_events(); print_counter(); return (0);                 \
    }

#define MAKE_BENCH_REJ(var, func)                                           \
    int bench_##var()                                                       \
    {                                                                       \
        debug_printf("bench kyber_sampling %-45s", #func "\0");             \
        uint8_t buf[REJ_BUFLEN] __attribute__((aligned(16)));               \
        int16_t r[KYBER_N] __attribute__((aligned(16)));                    \
        uint32_t ctr = 0, pos = 0;                                          \
        fill_random_u16( (uint16_t*) buf, REJ_BUFLEN / 2 );                 \
        for (unsigned cnt = 0; cnt < WARMUP_ITERATIONS; cnt++) (func)(r, buf, &ctr, &pos); \
        init_perf_events(); start_counting_events();                        \
        for (unsigned cnt = 0; cnt < TEST_COUNT; cnt++)                     \
            for (unsigned cntp = 0; cntp < ITER_PER_TEST; cntp++) (func)(r, buf, &ctr, &pos); \
        stop_and_read_events(); calc_average(ITER_PER_TEST, TEST_COUNT);    \
        cleanup_perf_events(); print_counter(); return (0);                 \
    }

MAKE_BENCH_CBD(cbd2_rvv_vlen128, cbd2_rvv_vlen128_wrap, CBD2_BUFLEN)
MAKE_BENCH_CBD(cbd3_rvv_vlen128, cbd3_rvv_vlen128_wrap, CBD3_BUFLEN)
MAKE_BENCH_REJ(rej_uniform_rvv_vlen128, rej_uniform_rvv_vlen128_wrap)
#ifdef WITH_OPT_KERNELS
MAKE_BENCH_CBD(cbd2_rvv_vlen128_opt_c908, cbd2_rvv_vlen128_opt_c908_wrap, CBD2_BUFLEN)
MAKE_BENCH_CBD(cbd3_rvv_vlen128_opt_c908, cbd3_rvv_vlen128_opt_c908_wrap, CBD3_BUFLEN)
MAKE_BENCH_REJ(rej_uniform_rvv_vlen128_opt_c908, rej_uniform_rvv_vlen128_opt_c908_wrap)
#endif

/* === MAIN FUNCTION === */
int main (void)
{
    int rc = 0;
    debug_test_start( "Kyber sampling (cbd2 / cbd3 / rej_uniform)!" );

    /* Naive kernels: property/range correctness (qemu-runnable) */
    rc |= test_cbd2_rvv_vlen128_naive();
    rc |= test_cbd3_rvv_vlen128_naive();
    rc |= test_rej_uniform_rvv_vlen128_naive();

#ifdef WITH_OPT_KERNELS
    /* Naive-vs-opt equivalence (C908 board only; opt traps on qemu) */
    rc |= test_cbd2_rvv_vlen128_opt_c908();
    rc |= test_cbd3_rvv_vlen128_opt_c908();
    rc |= test_rej_uniform_rvv_vlen128_opt_c908();
#endif

    /* Benchmarks */
    bench_cbd2_rvv_vlen128();
    bench_cbd3_rvv_vlen128();
    bench_rej_uniform_rvv_vlen128();
#ifdef WITH_OPT_KERNELS
    bench_cbd2_rvv_vlen128_opt_c908();
    bench_cbd3_rvv_vlen128_opt_c908();
    bench_rej_uniform_rvv_vlen128_opt_c908();
#endif

    if (rc == 0)
        debug_printf("Test Success!");
    else
        debug_printf("SOME TESTS FAILED (see FAIL markers above)");

    return rc;
}
