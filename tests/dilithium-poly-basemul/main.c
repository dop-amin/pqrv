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

/*
 * Test macros for different function signatures
 */

// For functions: func(int32_t r[256], const int32_t a[256], const int32_t b[256])
#define MAKE_TEST_POLY_BASEMUL_3P_INT32(var,func,ref_func,modulus)                   \
int test_ ## var ()                                                         \
{                                                                           \
    debug_printf("Test for " #func " ");                                    \
    int32_t a[NTT_SIZE]        __attribute__((aligned(16)));                \
    int32_t b[NTT_SIZE]        __attribute__((aligned(16)));                \
    int32_t r[NTT_SIZE]        __attribute__((aligned(16)));                \
    int32_t r_ref[NTT_SIZE]    __attribute__((aligned(16)));                \
                                                                            \
    /* Setup input */                                                       \
    fill_random_u32( (uint32_t*) a, NTT_SIZE );                             \
    mod_reduce_buf_s32( a, NTT_SIZE, modulus );                             \
    fill_random_u32( (uint32_t*) b, NTT_SIZE );                             \
    mod_reduce_buf_s32( b, NTT_SIZE, modulus );                             \
                                                                            \
    /* Initialize output arrays */                                          \
    memset(r, 0, sizeof(r));                                               \
    memset(r_ref, 0, sizeof(r_ref));                                       \
                                                                            \
    /* Step 1: Reference */                                                 \
    ref_func( r_ref, a, b );                                               \
                                                                            \
    /* Step 2: Test function */                                             \
    (func)( r, a, b );                                                      \
                                                                            \
    if( compare_buf_u32( (uint32_t const*) r, (uint32_t const*) r_ref,     \
                         NTT_SIZE ) != 0 )                                  \
    {                                                                       \
        debug_print_buf_s32( r_ref, NTT_SIZE, "Reference" );               \
        debug_print_buf_s32( r, NTT_SIZE, "This" );                        \
        debug_test_fail();                                                  \
        return( 1 );                                                        \
    }                                                                       \
    debug_test_ok();                                                        \
                                                                            \
    return( 0 );                                                            \
}

// For functions: func(int64_t r[256], const int32_t a[256], const int32_t b[256])
#define MAKE_TEST_POLY_BASEMUL_3P_INT64(var,func,ref_func,modulus)                  \
int test_ ## var ()                                                         \
{                                                                           \
    debug_printf("Test for " #func " ");                                    \
    int32_t a[NTT_SIZE]        __attribute__((aligned(16)));                \
    int32_t b[NTT_SIZE]        __attribute__((aligned(16)));                \
    int64_t r[NTT_SIZE]        __attribute__((aligned(16)));                \
    int64_t r_ref[NTT_SIZE]    __attribute__((aligned(16)));                \
                                                                            \
    /* Setup input */                                                       \
    fill_random_u32( (uint32_t*) a, NTT_SIZE );                             \
    mod_reduce_buf_s32( a, NTT_SIZE, modulus );                             \
    fill_random_u32( (uint32_t*) b, NTT_SIZE );                             \
    mod_reduce_buf_s32( b, NTT_SIZE, modulus );                             \
                                                                            \
    /* Initialize output arrays */                                          \
    memset(r, 0, sizeof(r));                                               \
    memset(r_ref, 0, sizeof(r_ref));                                       \
                                                                            \
    /* Step 1: Reference */                                                 \
    ref_func( r_ref, a, b );                                               \
                                                                            \
    /* Step 2: Test function */                                             \
    (func)( r, a, b );                                                      \
                                                                            \
    if( memcmp( r, r_ref, sizeof(r) ) != 0 )                               \
    {                                                                       \
        debug_print_buf_s64( r_ref, NTT_SIZE, "Reference" );               \
        debug_print_buf_s64( r, NTT_SIZE, "This" );                        \
        debug_test_fail();                                                  \
        return( 1 );                                                        \
    }                                                                       \
    debug_test_ok();                                                        \
                                                                            \
    return( 0 );                                                            \
}

// For functions: func(int32_t r[256], const int32_t a[256], const int32_t b[256], int64_t r_double[256])
#define MAKE_TEST_POLY_BASEMUL_4P_INT32(var,func,ref_func,modulus)                  \
int test_ ## var ()                                                         \
{                                                                           \
    debug_printf("Test for " #func " ");                                    \
    int32_t a[NTT_SIZE]        __attribute__((aligned(16)));                \
    int32_t b[NTT_SIZE]        __attribute__((aligned(16)));                \
    int32_t r[NTT_SIZE]        __attribute__((aligned(16)));                \
    int32_t r_ref[NTT_SIZE]    __attribute__((aligned(16)));                \
    int64_t r_double[NTT_SIZE] __attribute__((aligned(16)));                \
    int64_t r_double_ref[NTT_SIZE] __attribute__((aligned(16)));            \
                                                                            \
    /* Setup input */                                                       \
    fill_random_u32( (uint32_t*) a, NTT_SIZE );                             \
    mod_reduce_buf_s32( a, NTT_SIZE, modulus );                             \
    fill_random_u32( (uint32_t*) b, NTT_SIZE );                             \
    mod_reduce_buf_s32( b, NTT_SIZE, modulus );                             \
                                                                            \
    /* Initialize output arrays */                                          \
    memset(r, 0, sizeof(r));                                               \
    memset(r_ref, 0, sizeof(r_ref));                                       \
    memset(r_double, 0, sizeof(r_double));                                 \
    memset(r_double_ref, 0, sizeof(r_double_ref));                         \
                                                                            \
    /* Step 1: Reference */                                                 \
    ref_func( r_ref, a, b, r_double_ref );                                 \
                                                                            \
    /* Step 2: Test function */                                             \
    (func)( r, a, b, r_double );                                           \
                                                                            \
    if( compare_buf_u32( (uint32_t const*) r, (uint32_t const*) r_ref,     \
                         NTT_SIZE ) != 0 )                                  \
    {                                                                       \
        debug_print_buf_s32( r_ref, NTT_SIZE, "Reference" );               \
        debug_print_buf_s32( r, NTT_SIZE, "This" );                        \
        debug_test_fail();                                                  \
        return( 1 );                                                        \
    }                                                                       \
    debug_test_ok();                                                        \
                                                                            \
    return( 0 );                                                            \
}

// For in-place functions: func(int32_t a[256])
#define MAKE_TEST_INPLACE_S32(var,func,ref_func,modulus)                    \
int test_ ## var ()                                                         \
{                                                                           \
    debug_printf("Test for " #func " ");                                    \
    int32_t in[NTT_SIZE]       __attribute__((aligned(16)));                \
    int32_t r[NTT_SIZE]        __attribute__((aligned(16)));                \
    int32_t r_ref[NTT_SIZE]    __attribute__((aligned(16)));                \
                                                                            \
    fill_random_u32( (uint32_t*) in, NTT_SIZE );                            \
    mod_reduce_buf_s32( in, NTT_SIZE, modulus );                            \
    memcpy( r,     in, sizeof(r) );                                         \
    memcpy( r_ref, in, sizeof(r_ref) );                                     \
                                                                            \
    ref_func( r_ref );                                                      \
    (func)( r );                                                            \
                                                                            \
    if( compare_buf_u32( (uint32_t const*) r, (uint32_t const*) r_ref,      \
                         NTT_SIZE ) != 0 )                                  \
    {                                                                       \
        debug_print_buf_s32( r_ref, NTT_SIZE, "Reference" );                \
        debug_print_buf_s32( r, NTT_SIZE, "This" );                         \
        debug_test_fail();                                                  \
        return( 1 );                                                        \
    }                                                                       \
    debug_test_ok();                                                        \
                                                                            \
    return( 0 );                                                            \
}

#define MAKE_BENCH_INPLACE_S32(var, func)                                   \
    int bench_##var()                                                       \
    {                                                                       \
        debug_printf("bench dilithium_poly_basemul %-50s", #func "\0");     \
        int32_t a[NTT_SIZE] __attribute__((aligned(16)));                   \
        fill_random_u32( (uint32_t*) a, NTT_SIZE );                         \
        mod_reduce_buf_s32( a, NTT_SIZE, DILITHIUM_Q );                     \
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

#define MAKE_BENCH_3P_INT32(var, func)                                     \
    int bench_##var()                                                   \
    {                                                                   \
        debug_printf("bench poly_basemul %-50s", #func "\0");          \
        int32_t a[DILITHIUM_N] __attribute__((aligned(16)));            \
        int32_t b[DILITHIUM_N] __attribute__((aligned(16)));            \
        int32_t r[DILITHIUM_N] __attribute__((aligned(16)));            \
        \
        for (unsigned cnt = 0; cnt < WARMUP_ITERATIONS; cnt++) {        \
            (func)(r, a, b);\
        }                                                               \
        init_perf_events(); \
        start_counting_events(); \
        for (unsigned cnt = 0; cnt < TEST_COUNT; cnt++)                 \
        { \
            for (unsigned cntp = 0; cntp < ITER_PER_TEST; cntp++) {     \
                (func)(r, a, b);                                        \
            } \
              \
        }                                                               \
        stop_and_read_events(); \
        calc_average(ITER_PER_TEST, TEST_COUNT); \
        cleanup_perf_events(); \
        print_counter(); \
        return (0);                                                     \
    }

#define MAKE_BENCH_3P_INT64(var, func)                                     \
    int bench_##var()                                                   \
    {                                                                   \
        debug_printf("bench poly_basemul %-50s", #func "\0");          \
        int32_t a[DILITHIUM_N] __attribute__((aligned(16)));            \
        int32_t b[DILITHIUM_N] __attribute__((aligned(16)));            \
        int64_t r[DILITHIUM_N] __attribute__((aligned(16)));            \
        \
        for (unsigned cnt = 0; cnt < WARMUP_ITERATIONS; cnt++) {        \
            (func)(r, a, b);\
        }                                                               \
        init_perf_events(); \
        start_counting_events(); \
        for (unsigned cnt = 0; cnt < TEST_COUNT; cnt++)                 \
        { \
            for (unsigned cntp = 0; cntp < ITER_PER_TEST; cntp++) {     \
                (func)(r, a, b);                                        \
            } \
              \
        }                                                               \
        stop_and_read_events(); \
        calc_average(ITER_PER_TEST, TEST_COUNT); \
        cleanup_perf_events(); \
        print_counter(); \
        return (0);                                                     \
    }

#define MAKE_BENCH_4P_INT32(var, func)                                     \
    int bench_##var()                                                   \
    {                                                                   \
        debug_printf("bench poly_basemul %-50s", #func "\0");          \
        int32_t a[DILITHIUM_N] __attribute__((aligned(16)));            \
        int32_t b[DILITHIUM_N] __attribute__((aligned(16)));            \
        int32_t r[DILITHIUM_N] __attribute__((aligned(16)));            \
        int64_t r_double[DILITHIUM_N] __attribute__((aligned(16)));     \
        \
        for (unsigned cnt = 0; cnt < WARMUP_ITERATIONS; cnt++) {        \
            (func)(r, a, b, r_double);\
        }                                                               \
        init_perf_events(); \
        start_counting_events(); \
        for (unsigned cnt = 0; cnt < TEST_COUNT; cnt++)                 \
        { \
            for (unsigned cntp = 0; cntp < ITER_PER_TEST; cntp++) {     \
                (func)(r, a, b, r_double);                              \
            } \
              \
        }                                                               \
        stop_and_read_events(); \
        calc_average(ITER_PER_TEST, TEST_COUNT); \
        cleanup_perf_events(); \
        print_counter(); \
        return (0);                                                     \
    }

// === TESTS ===
// Basic functions (sanity check - compare against themselves)
MAKE_TEST_POLY_BASEMUL_3P_INT64(poly_basemul_8l_acc_rv64im,                  poly_basemul_8l_acc_rv64im_wrap,              poly_basemul_8l_acc_rv64im_wrap,   DILITHIUM_Q)
MAKE_TEST_POLY_BASEMUL_4P_INT32(poly_basemul_8l_acc_end_rv64im,              poly_basemul_8l_acc_end_rv64im_wrap,          poly_basemul_8l_acc_end_rv64im_wrap, DILITHIUM_Q)
MAKE_TEST_POLY_BASEMUL_3P_INT64(poly_basemul_8l_init_rv64im,                 poly_basemul_8l_init_rv64im_wrap,             poly_basemul_8l_init_rv64im_wrap,  DILITHIUM_Q)
MAKE_TEST_POLY_BASEMUL_3P_INT32(poly_basemul_8l_rv64im,                      poly_basemul_8l_rv64im_wrap,                  poly_basemul_8l_rv64im_wrap,       DILITHIUM_Q)

// Dual functions (compare against basic versions with same signature)
MAKE_TEST_POLY_BASEMUL_3P_INT64(poly_basemul_8l_acc_rv64im_dual,             poly_basemul_8l_acc_rv64im_dual_wrap,         poly_basemul_8l_acc_rv64im_wrap,   DILITHIUM_Q)
MAKE_TEST_POLY_BASEMUL_4P_INT32(poly_basemul_8l_acc_end_rv64im_dual,         poly_basemul_8l_acc_end_rv64im_dual_wrap,     poly_basemul_8l_acc_end_rv64im_wrap, DILITHIUM_Q)
MAKE_TEST_POLY_BASEMUL_3P_INT64(poly_basemul_8l_init_rv64im_dual,            poly_basemul_8l_init_rv64im_dual_wrap,        poly_basemul_8l_init_rv64im_wrap,  DILITHIUM_Q)
MAKE_TEST_POLY_BASEMUL_3P_INT32(poly_basemul_8l_rv64im_dual,                 poly_basemul_8l_rv64im_dual_wrap,             poly_basemul_8l_rv64im_wrap,       DILITHIUM_Q)

// Optimized functions (compare against basic versions with same signature)
MAKE_TEST_POLY_BASEMUL_3P_INT64(poly_basemul_8l_acc_rv64im_opt_c908,         poly_basemul_8l_acc_rv64im_opt_c908_wrap,     poly_basemul_8l_acc_rv64im_wrap,   DILITHIUM_Q)
MAKE_TEST_POLY_BASEMUL_4P_INT32(poly_basemul_8l_acc_end_rv64im_opt_c908,     poly_basemul_8l_acc_end_rv64im_opt_c908_wrap, poly_basemul_8l_acc_end_rv64im_wrap, DILITHIUM_Q)
MAKE_TEST_POLY_BASEMUL_3P_INT64(poly_basemul_8l_init_rv64im_opt_c908,        poly_basemul_8l_init_rv64im_opt_c908_wrap,    poly_basemul_8l_init_rv64im_wrap,  DILITHIUM_Q)
MAKE_TEST_POLY_BASEMUL_3P_INT32(poly_basemul_8l_rv64im_opt_c908,             poly_basemul_8l_rv64im_opt_c908_wrap,         poly_basemul_8l_rv64im_wrap,       DILITHIUM_Q)

MAKE_TEST_POLY_BASEMUL_3P_INT64(poly_basemul_8l_acc_rv64im_opt_c908_dual,    poly_basemul_8l_acc_rv64im_opt_c908_dual_wrap,    poly_basemul_8l_acc_rv64im_wrap,   DILITHIUM_Q)
// MAKE_TEST_POLY_BASEMUL_4P_INT32(poly_basemul_8l_acc_end_rv64im_opt_c908_dual,poly_basemul_8l_acc_end_rv64im_opt_c908_dual_wrap,poly_basemul_8l_acc_end_rv64im_wrap, DILITHIUM_Q)
MAKE_TEST_POLY_BASEMUL_3P_INT64(poly_basemul_8l_init_rv64im_opt_c908_dual,   poly_basemul_8l_init_rv64im_opt_c908_dual_wrap,   poly_basemul_8l_init_rv64im_wrap,  DILITHIUM_Q)
MAKE_TEST_POLY_BASEMUL_3P_INT32(poly_basemul_8l_rv64im_opt_c908_dual,        poly_basemul_8l_rv64im_opt_c908_dual_wrap,        poly_basemul_8l_rv64im_wrap,       DILITHIUM_Q)

// RVV functions (sanity check - compare against themselves)
MAKE_TEST_POLY_BASEMUL_3P_INT32(poly_basemul_rvv_vlen128,                     poly_basemul_rvv_vlen128_wrap,                    poly_basemul_rvv_vlen128_wrap,       DILITHIUM_Q)
MAKE_TEST_POLY_BASEMUL_3P_INT32(poly_basemul_acc_rvv_vlen128,                 poly_basemul_acc_rvv_vlen128_wrap,                poly_basemul_acc_rvv_vlen128_wrap,       DILITHIUM_Q)

// RVV optimized functions (compare against basic RVV versions)
MAKE_TEST_POLY_BASEMUL_3P_INT32(poly_basemul_rvv_vlen128_opt_c908,            poly_basemul_rvv_vlen128_opt_c908_wrap,           poly_basemul_rvv_vlen128_wrap,     DILITHIUM_Q)
MAKE_TEST_POLY_BASEMUL_3P_INT32(poly_basemul_acc_rvv_vlen128_opt_c908,        poly_basemul_acc_rvv_vlen128_opt_c908_wrap,       poly_basemul_acc_rvv_vlen128_wrap, DILITHIUM_Q)

// poly_reduce (RVV) - optimized tested against the non-optimized version
MAKE_TEST_INPLACE_S32(poly_reduce_rvv_vlen128_opt_c908, poly_reduce_rvv_vlen128_opt_c908_wrap, poly_reduce_rvv_vlen128_wrap, DILITHIUM_Q)

// === BENCHMARKS ===
MAKE_BENCH_3P_INT64(poly_basemul_8l_acc_rv64im,              poly_basemul_8l_acc_rv64im_wrap)
MAKE_BENCH_4P_INT32(poly_basemul_8l_acc_end_rv64im,          poly_basemul_8l_acc_end_rv64im_wrap)
MAKE_BENCH_3P_INT64(poly_basemul_8l_init_rv64im,             poly_basemul_8l_init_rv64im_wrap)
MAKE_BENCH_3P_INT32(poly_basemul_8l_rv64im,                  poly_basemul_8l_rv64im_wrap)

MAKE_BENCH_3P_INT64(poly_basemul_8l_acc_rv64im_dual,         poly_basemul_8l_acc_rv64im_dual_wrap)
MAKE_BENCH_4P_INT32(poly_basemul_8l_acc_end_rv64im_dual,     poly_basemul_8l_acc_end_rv64im_dual_wrap)
MAKE_BENCH_3P_INT64(poly_basemul_8l_init_rv64im_dual,        poly_basemul_8l_init_rv64im_dual_wrap)
MAKE_BENCH_3P_INT32(poly_basemul_8l_rv64im_dual,             poly_basemul_8l_rv64im_dual_wrap)

// Optimized benchmarks
MAKE_BENCH_3P_INT64(poly_basemul_8l_acc_rv64im_opt_c908,     poly_basemul_8l_acc_rv64im_opt_c908_wrap)
MAKE_BENCH_4P_INT32(poly_basemul_8l_acc_end_rv64im_opt_c908, poly_basemul_8l_acc_end_rv64im_opt_c908_wrap)
MAKE_BENCH_3P_INT64(poly_basemul_8l_init_rv64im_opt_c908,    poly_basemul_8l_init_rv64im_opt_c908_wrap)
MAKE_BENCH_3P_INT32(poly_basemul_8l_rv64im_opt_c908,         poly_basemul_8l_rv64im_opt_c908_wrap)

MAKE_BENCH_3P_INT64(poly_basemul_8l_acc_rv64im_opt_c908_dual,         poly_basemul_8l_acc_rv64im_opt_c908_dual_wrap)
// MAKE_BENCH_4P_INT32(poly_basemul_8l_acc_end_rv64im_opt_c908_dual,     poly_basemul_8l_acc_end_rv64im_opt_c908_dual_wrap)
MAKE_BENCH_3P_INT64(poly_basemul_8l_init_rv64im_opt_c908_dual,        poly_basemul_8l_init_rv64im_opt_c908_dual_wrap)
MAKE_BENCH_3P_INT32(poly_basemul_8l_rv64im_opt_c908_dual,             poly_basemul_8l_rv64im_opt_c908_dual_wrap)

// poly_reduce (RVV)
MAKE_BENCH_INPLACE_S32(poly_reduce_rvv_vlen128,          poly_reduce_rvv_vlen128_wrap)
MAKE_BENCH_INPLACE_S32(poly_reduce_rvv_vlen128_opt_c908, poly_reduce_rvv_vlen128_opt_c908_wrap)


// === MAIN FUNCTION ===
int main (void)
{
    debug_test_start( "Poly basemul!" );

    if (test_poly_basemul_8l_acc_rv64im()           != 0) return 1;
    if (test_poly_basemul_8l_acc_end_rv64im()       != 0) return 1;
    if (test_poly_basemul_8l_init_rv64im()          != 0) return 1;
    if (test_poly_basemul_8l_rv64im()               != 0) return 1;

    if (test_poly_basemul_8l_acc_rv64im_dual()      != 0) return 1;
    if (test_poly_basemul_8l_acc_end_rv64im_dual()  != 0) return 1;
    if (test_poly_basemul_8l_init_rv64im_dual()     != 0) return 1;
    if (test_poly_basemul_8l_rv64im_dual()          != 0) return 1;

    if (test_poly_basemul_8l_acc_rv64im_opt_c908()      != 0) return 1;
    if (test_poly_basemul_8l_acc_end_rv64im_opt_c908()  != 0) return 1;
    if (test_poly_basemul_8l_init_rv64im_opt_c908()     != 0) return 1;
    if (test_poly_basemul_8l_rv64im_opt_c908()          != 0) return 1;

    if (test_poly_basemul_8l_acc_rv64im_opt_c908_dual()     != 0) return 1;
    // if (test_poly_basemul_8l_acc_end_rv64im_opt_c908_dual() != 0) return 1;
    if (test_poly_basemul_8l_init_rv64im_opt_c908_dual()    != 0) return 1;
    if (test_poly_basemul_8l_rv64im_opt_c908_dual()         != 0) return 1;

    // RVV function tests
    if (test_poly_basemul_rvv_vlen128()                     != 0) return 1;
    if (test_poly_basemul_acc_rvv_vlen128()                 != 0) return 1;

    // RVV optimized function tests
    if (test_poly_basemul_rvv_vlen128_opt_c908()            != 0) return 1;
    if (test_poly_basemul_acc_rvv_vlen128_opt_c908()        != 0) return 1;
    if (test_poly_reduce_rvv_vlen128_opt_c908()             != 0) return 1;

    bench_poly_basemul_8l_acc_rv64im();
    bench_poly_basemul_8l_acc_end_rv64im();
    bench_poly_basemul_8l_init_rv64im();
    bench_poly_basemul_8l_rv64im();

    bench_poly_basemul_8l_acc_rv64im_dual();
    bench_poly_basemul_8l_acc_end_rv64im_dual();
    bench_poly_basemul_8l_init_rv64im_dual();
    bench_poly_basemul_8l_rv64im_dual();

    bench_poly_basemul_8l_acc_rv64im_opt_c908();
    bench_poly_basemul_8l_acc_end_rv64im_opt_c908();
    bench_poly_basemul_8l_init_rv64im_opt_c908();
    bench_poly_basemul_8l_rv64im_opt_c908();

    bench_poly_basemul_8l_acc_rv64im_opt_c908_dual();
    // bench_poly_basemul_8l_acc_end_rv64im_opt_c908_dual();
    bench_poly_basemul_8l_init_rv64im_opt_c908_dual();
    bench_poly_basemul_8l_rv64im_opt_c908_dual();

    bench_poly_reduce_rvv_vlen128();
    bench_poly_reduce_rvv_vlen128_opt_c908();

    debug_printf("Test Success!"); 

    return 0;
}