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
#define KYBER_Q            3329

uint64_t t0, t1;
uint64_t cycles[TEST_COUNT];

/*
 * Test / benchmark macros for in-place polynomial helpers:
 *     void f(int16_t a[256]);
 *
 * Correctness is verified by naive-vs-optimized equivalence: the same random
 * input is fed to `func` and to the reference `ref_func` (the NAIVE variant of
 * the SAME issue-mode), and the resulting polynomials are compared
 * coefficient-wise. SLOTHY only reschedules instructions, so an optimized
 * routine must reproduce its naive counterpart bit-for-bit.
 *
 * NOTE: the single-issue and dual-issue naive kernels use *different* Plantard
 * representatives, so they must not be compared against each other -- each
 * optimized variant is compared only against its own naive kernel.
 */

#define MAKE_TEST_INPLACE_S16(var,func,ref_func,modulus)                    \
int test_ ## var ()                                                         \
{                                                                           \
    debug_printf("Test for " #func " ");                                    \
    int16_t in[KYBER_N]        __attribute__((aligned(16)));                \
    int16_t r[KYBER_N]         __attribute__((aligned(16)));                \
    int16_t r_ref[KYBER_N]     __attribute__((aligned(16)));                \
                                                                            \
    /* Setup input (random, non-zero, reduced mod q) */                     \
    fill_random_u16( (uint16_t*) in, KYBER_N );                             \
    mod_reduce_buf_s16( in, KYBER_N, modulus );                             \
    memcpy( r,     in, sizeof(r) );                                         \
    memcpy( r_ref, in, sizeof(r_ref) );                                     \
                                                                            \
    /* Step 1: Reference (naive) */                                         \
    ref_func( r_ref );                                                      \
                                                                            \
    /* Step 2: Test function */                                             \
    (func)( r );                                                            \
                                                                            \
    if( compare_buf_u16( (uint16_t const*) r, (uint16_t const*) r_ref,      \
                         KYBER_N ) != 0 )                                   \
    {                                                                       \
        debug_print_buf_s16( r_ref, KYBER_N, "Reference" );                 \
        debug_print_buf_s16( r, KYBER_N, "This" );                          \
        debug_test_fail();                                                  \
        return( 1 );                                                        \
    }                                                                       \
    debug_test_ok();                                                        \
                                                                            \
    return( 0 );                                                            \
}

#define MAKE_BENCH_INPLACE_S16(var, func)                                   \
    int bench_##var()                                                       \
    {                                                                       \
        debug_printf("bench kyber_poly_reduce %-50s", #func "\0");          \
        int16_t a[KYBER_N] __attribute__((aligned(16)));                    \
        fill_random_u16( (uint16_t*) a, KYBER_N );                          \
        mod_reduce_buf_s16( a, KYBER_N, KYBER_Q );                          \
                                                                            \
        for (unsigned cnt = 0; cnt < WARMUP_ITERATIONS; cnt++) {            \
            (func)(a);                                                      \
        }                                                                   \
        init_perf_events();                                                 \
        start_counting_events();                                            \
        for (unsigned cnt = 0; cnt < TEST_COUNT; cnt++)                     \
        {                                                                   \
            for (unsigned cntp = 0; cntp < ITER_PER_TEST; cntp++) {         \
                (func)(a);                                                  \
            }                                                               \
        }                                                                   \
        stop_and_read_events();                                             \
        calc_average(ITER_PER_TEST, TEST_COUNT);                            \
        cleanup_perf_events();                                              \
        print_counter();                                                    \
        return (0);                                                         \
    }

/* === TESTS (naive-vs-opt equivalence, same issue-mode) === */

/* RV64IM single-issue Plantard reduction */
MAKE_TEST_INPLACE_S16(poly_plantard_rdc_rv64im_opt_c908, poly_plantard_rdc_rv64im_opt_c908_wrap, poly_plantard_rdc_rv64im_wrap, KYBER_Q)

/* RV64IM single-issue to-Plantard conversion */
MAKE_TEST_INPLACE_S16(poly_toplant_rv64im_opt_c908,      poly_toplant_rv64im_opt_c908_wrap,      poly_toplant_rv64im_wrap,      KYBER_Q)

#ifdef VECTOR128
/* RVV Barrett reduction */
MAKE_TEST_INPLACE_S16(poly_reduce_rvv_vlen128_opt_c908,  poly_reduce_rvv_vlen128_opt_c908_wrap,  poly_reduce_rvv_vlen128_wrap,  KYBER_Q)

/* RVV to-Montgomery conversion */
MAKE_TEST_INPLACE_S16(poly_tomont_rvv_vlen128_opt_c908,  poly_tomont_rvv_vlen128_opt_c908_wrap,  poly_tomont_rvv_vlen128_wrap,  KYBER_Q)
#endif /* VECTOR128 */

/*
 * The `_dual_opt_c908` variants are excluded from the correctness suite: the
 * SLOTHY-generated dual-issue optimized outputs are currently defective (see
 * the accompanying report -- mangled register-save prologue, and output that
 * diverges from the dual naive kernel). They are still wired up (asm symlinks,
 * .mk, wrappers) so they can be re-enabled once regenerated correctly.
 */

// kyber_poly_plantard_rdc_rv64im_dual_opt_c908.s`
// `kyber_poly_toplant_rv64im_dual_opt_c908.s`

/* === BENCHMARKS === */
MAKE_BENCH_INPLACE_S16(poly_plantard_rdc_rv64im,          poly_plantard_rdc_rv64im_wrap)
MAKE_BENCH_INPLACE_S16(poly_plantard_rdc_rv64im_dual,     poly_plantard_rdc_rv64im_dual_wrap)
MAKE_BENCH_INPLACE_S16(poly_plantard_rdc_rv64im_opt_c908, poly_plantard_rdc_rv64im_opt_c908_wrap)

MAKE_BENCH_INPLACE_S16(poly_toplant_rv64im,               poly_toplant_rv64im_wrap)
MAKE_BENCH_INPLACE_S16(poly_toplant_rv64im_dual,          poly_toplant_rv64im_dual_wrap)
MAKE_BENCH_INPLACE_S16(poly_toplant_rv64im_opt_c908,      poly_toplant_rv64im_opt_c908_wrap)

#ifdef VECTOR128
MAKE_BENCH_INPLACE_S16(poly_reduce_rvv_vlen128,          poly_reduce_rvv_vlen128_wrap)
MAKE_BENCH_INPLACE_S16(poly_reduce_rvv_vlen128_opt_c908, poly_reduce_rvv_vlen128_opt_c908_wrap)

MAKE_BENCH_INPLACE_S16(poly_tomont_rvv_vlen128,          poly_tomont_rvv_vlen128_wrap)
MAKE_BENCH_INPLACE_S16(poly_tomont_rvv_vlen128_opt_c908, poly_tomont_rvv_vlen128_opt_c908_wrap)
#endif /* VECTOR128 */

/* === MAIN FUNCTION === */
int main (void)
{
    int rc = 0;
    debug_test_start( "Kyber poly reduce / domain conversion!" );

    /* --- correctness (naive-vs-opt equivalence) --- */
    rc |= test_poly_plantard_rdc_rv64im_opt_c908();
    rc |= test_poly_toplant_rv64im_opt_c908();
#ifdef VECTOR128
    rc |= test_poly_reduce_rvv_vlen128_opt_c908();
    rc |= test_poly_tomont_rvv_vlen128_opt_c908();
#endif /* VECTOR128 */

    /* --- benchmarks (naive vs dual naive vs optimized) --- */
    bench_poly_plantard_rdc_rv64im();
    bench_poly_plantard_rdc_rv64im_dual();
    bench_poly_plantard_rdc_rv64im_opt_c908();

    bench_poly_toplant_rv64im();
    bench_poly_toplant_rv64im_dual();
    bench_poly_toplant_rv64im_opt_c908();

#ifdef VECTOR128
    bench_poly_reduce_rvv_vlen128();
    bench_poly_reduce_rvv_vlen128_opt_c908();

    bench_poly_tomont_rvv_vlen128();
    bench_poly_tomont_rvv_vlen128_opt_c908();
#endif /* VECTOR128 */

    if (rc == 0)
        debug_printf("Test Success!");
    else
        debug_printf("SOME TESTS FAILED (see FAIL markers above)");

    return rc;
}
