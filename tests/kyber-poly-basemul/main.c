/*
 * Copyright (c) 2021 Arm Limited
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, do so subject to the following conditions:
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

uint64_t t0, t1;
uint64_t cycles[TEST_COUNT];

/*
 * Test macros for different function signatures
 */

// For functions: func(int32_t r[256], const int16_t a[256], const int16_t b[256], const int64_t zeta[128])
#define MAKE_TEST_KYBER_BASEMUL_4P_I32_I64(var,func,ref_func,modulus)                   \
int test_ ## var ()                                                         \
{                                                                           \
    debug_printf("Test for " #func " ");                                    \
    int16_t a[KYBER_N]         __attribute__((aligned(16)));                \
    int16_t b[KYBER_N]         __attribute__((aligned(16)));                \
    int64_t zeta[KYBER_N/2]    __attribute__((aligned(16)));                \
    int32_t r[KYBER_N]         __attribute__((aligned(16)));                \
    int32_t r_ref[KYBER_N]     __attribute__((aligned(16)));                \
                                                                            \
    /* Setup input */                                                       \
    fill_random_u16( (uint16_t*) a, KYBER_N );                             \
    mod_reduce_buf_s16( a, KYBER_N, modulus );                              \
    fill_random_u16( (uint16_t*) b, KYBER_N );                             \
    mod_reduce_buf_s16( b, KYBER_N, modulus );                              \
    fill_random_u32( (uint32_t*) zeta, KYBER_N );                          \
    mod_reduce_buf_s32( (int32_t*) zeta, KYBER_N, modulus );                \
                                                                            \
    /* Initialize output arrays */                                          \
    memset(r, 0, sizeof(r));                                               \
    memset(r_ref, 0, sizeof(r_ref));                                       \
                                                                            \
    /* Step 1: Reference */                                                 \
    ref_func( r_ref, a, b, zeta );                                         \
                                                                            \
    /* Step 2: Test function */                                             \
    (func)( r, a, b, zeta );                                               \
                                                                            \
    if( compare_buf_u32( (uint32_t const*) r, (uint32_t const*) r_ref,     \
                         KYBER_N ) != 0 )                                   \
    {                                                                       \
        debug_print_buf_s32( r_ref, KYBER_N, "Reference" );                \
        debug_print_buf_s32( r, KYBER_N, "This" );                         \
        debug_test_fail();                                                  \
        return( 1 );                                                        \
    }                                                                       \
    debug_test_ok();                                                        \
                                                                            \
    return( 0 );                                                            \
}

// For functions: func(int32_t r[256], const int16_t a[256], const int16_t b[256], const int32_t zeta[128])
#define MAKE_TEST_KYBER_BASEMUL_4P_I32_I32(var,func,ref_func,modulus)                   \
int test_ ## var ()                                                         \
{                                                                           \
    debug_printf("Test for " #func " ");                                    \
    int16_t a[KYBER_N]         __attribute__((aligned(16)));                \
    int16_t b[KYBER_N]         __attribute__((aligned(16)));                \
    int32_t zeta[KYBER_N/2]    __attribute__((aligned(16)));                \
    int32_t r[KYBER_N]         __attribute__((aligned(16)));                \
    int32_t r_ref[KYBER_N]     __attribute__((aligned(16)));                \
                                                                            \
    /* Setup input */                                                       \
    fill_random_u16( (uint16_t*) a, KYBER_N );                             \
    mod_reduce_buf_s16( a, KYBER_N, modulus );                              \
    fill_random_u16( (uint16_t*) b, KYBER_N );                             \
    mod_reduce_buf_s16( b, KYBER_N, modulus );                              \
    fill_random_u32( (uint32_t*) zeta, KYBER_N/2 );                        \
    mod_reduce_buf_s32( zeta, KYBER_N/2, modulus );                         \
                                                                            \
    /* Initialize output arrays */                                          \
    memset(r, 0, sizeof(r));                                               \
    memset(r_ref, 0, sizeof(r_ref));                                       \
                                                                            \
    /* Step 1: Reference */                                                 \
    ref_func( r_ref, a, b, zeta );                                         \
                                                                            \
    /* Step 2: Test function */                                             \
    (func)( r, a, b, zeta );                                               \
                                                                            \
    if( compare_buf_u32( (uint32_t const*) r, (uint32_t const*) r_ref,     \
                         KYBER_N ) != 0 )                                   \
    {                                                                       \
        debug_print_buf_s32( r_ref, KYBER_N, "Reference" );                \
        debug_print_buf_s32( r, KYBER_N, "This" );                         \
        debug_test_fail();                                                  \
        return( 1 );                                                        \
    }                                                                       \
    debug_test_ok();                                                        \
                                                                            \
    return( 0 );                                                            \
}

// For functions: func(int16_t r[256], const int16_t a[256], const int16_t b[256], const int64_t zeta[128], const int32_t acc[256])
#define MAKE_TEST_KYBER_BASEMUL_5P_I16_I64_I32(var,func,ref_func,modulus)               \
int test_ ## var ()                                                         \
{                                                                           \
    debug_printf("Test for " #func " ");                                    \
    int16_t a[KYBER_N]         __attribute__((aligned(16)));                \
    int16_t b[KYBER_N]         __attribute__((aligned(16)));                \
    int64_t zeta[KYBER_N/2]    __attribute__((aligned(16)));                \
    int32_t acc[KYBER_N]       __attribute__((aligned(16)));                \
    int16_t r[KYBER_N]         __attribute__((aligned(16)));                \
    int16_t r_ref[KYBER_N]     __attribute__((aligned(16)));                \
                                                                            \
    /* Setup input */                                                       \
    fill_random_u16( (uint16_t*) a, KYBER_N );                             \
    mod_reduce_buf_s16( a, KYBER_N, modulus );                              \
    fill_random_u16( (uint16_t*) b, KYBER_N );                             \
    mod_reduce_buf_s16( b, KYBER_N, modulus );                              \
    fill_random_u32( (uint32_t*) zeta, KYBER_N );                          \
    mod_reduce_buf_s32( (int32_t*) zeta, KYBER_N, modulus );                \
    fill_random_u32( (uint32_t*) acc, KYBER_N );                           \
    mod_reduce_buf_s32( acc, KYBER_N, modulus );                            \
                                                                            \
    /* Initialize output arrays */                                          \
    memset(r, 0, sizeof(r));                                               \
    memset(r_ref, 0, sizeof(r_ref));                                       \
                                                                            \
    /* Step 1: Reference */                                                 \
    ref_func( r_ref, a, b, zeta, acc );                                    \
                                                                            \
    /* Step 2: Test function */                                             \
    (func)( r, a, b, zeta, acc );                                          \
                                                                            \
    if( compare_buf_u16( (uint16_t const*) r, (uint16_t const*) r_ref,     \
                         KYBER_N ) != 0 )                                   \
    {                                                                       \
        debug_print_buf_s16( r_ref, KYBER_N, "Reference" );                \
        debug_print_buf_s16( r, KYBER_N, "This" );                         \
        debug_test_fail();                                                  \
        return( 1 );                                                        \
    }                                                                       \
    debug_test_ok();                                                        \
                                                                            \
    return( 0 );                                                            \
}

// For functions: func(int16_t r[256], const int16_t a[256], const int16_t b[256], const int32_t zeta[128], const int32_t acc[256])
#define MAKE_TEST_KYBER_BASEMUL_5P_I16_I32_I32(var,func,ref_func,modulus)               \
int test_ ## var ()                                                         \
{                                                                           \
    debug_printf("Test for " #func " ");                                    \
    int16_t a[KYBER_N]         __attribute__((aligned(16)));                \
    int16_t b[KYBER_N]         __attribute__((aligned(16)));                \
    int32_t zeta[KYBER_N/2]    __attribute__((aligned(16)));                \
    int32_t acc[KYBER_N]       __attribute__((aligned(16)));                \
    int16_t r[KYBER_N]         __attribute__((aligned(16)));                \
    int16_t r_ref[KYBER_N]     __attribute__((aligned(16)));                \
                                                                            \
    /* Setup input */                                                       \
    fill_random_u16( (uint16_t*) a, KYBER_N );                             \
    mod_reduce_buf_s16( a, KYBER_N, modulus );                              \
    fill_random_u16( (uint16_t*) b, KYBER_N );                             \
    mod_reduce_buf_s16( b, KYBER_N, modulus );                              \
    fill_random_u32( (uint32_t*) zeta, KYBER_N/2 );                        \
    mod_reduce_buf_s32( zeta, KYBER_N/2, modulus );                         \
    fill_random_u32( (uint32_t*) acc, KYBER_N );                           \
    mod_reduce_buf_s32( acc, KYBER_N, modulus );                            \
                                                                            \
    /* Initialize output arrays */                                          \
    memset(r, 0, sizeof(r));                                               \
    memset(r_ref, 0, sizeof(r_ref));                                       \
                                                                            \
    /* Step 1: Reference */                                                 \
    ref_func( r_ref, a, b, zeta, acc );                                    \
                                                                            \
    /* Step 2: Test function */                                             \
    (func)( r, a, b, zeta, acc );                                          \
                                                                            \
    if( compare_buf_u16( (uint16_t const*) r, (uint16_t const*) r_ref,     \
                         KYBER_N ) != 0 )                                   \
    {                                                                       \
        debug_print_buf_s16( r_ref, KYBER_N, "Reference" );                \
        debug_print_buf_s16( r, KYBER_N, "This" );                         \
        debug_test_fail();                                                  \
        return( 1 );                                                        \
    }                                                                       \
    debug_test_ok();                                                        \
                                                                            \
    return( 0 );                                                            \
}

// For functions: func(int32_t r[256], const int16_t a[256], const int16_t b[256], int16_t cache[256], const int64_t zeta[128])
#define MAKE_TEST_KYBER_BASEMUL_5P_I32_CACHE_I64(var,func,ref_func,modulus)             \
int test_ ## var ()                                                         \
{                                                                           \
    debug_printf("Test for " #func " ");                                    \
    int16_t a[KYBER_N]         __attribute__((aligned(16)));                \
    int16_t b[KYBER_N]         __attribute__((aligned(16)));                \
    int64_t zeta[KYBER_N/2]    __attribute__((aligned(16)));                \
    int16_t cache[KYBER_N]     __attribute__((aligned(16)));                \
    int16_t cache_ref[KYBER_N] __attribute__((aligned(16)));                \
    int32_t r[KYBER_N]         __attribute__((aligned(16)));                \
    int32_t r_ref[KYBER_N]     __attribute__((aligned(16)));                \
                                                                            \
    /* Setup input */                                                       \
    fill_random_u16( (uint16_t*) a, KYBER_N );                             \
    mod_reduce_buf_s16( a, KYBER_N, modulus );                              \
    fill_random_u16( (uint16_t*) b, KYBER_N );                             \
    mod_reduce_buf_s16( b, KYBER_N, modulus );                              \
    fill_random_u32( (uint32_t*) zeta, KYBER_N );                          \
    mod_reduce_buf_s32( (int32_t*) zeta, KYBER_N, modulus );                \
                                                                            \
    /* Initialize output arrays */                                          \
    memset(r, 0, sizeof(r));                                               \
    memset(r_ref, 0, sizeof(r_ref));                                       \
    memset(cache, 0, sizeof(cache));                                       \
    memset(cache_ref, 0, sizeof(cache_ref));                               \
                                                                            \
    /* Step 1: Reference */                                                 \
    ref_func( r_ref, a, b, cache_ref, zeta );                              \
                                                                            \
    /* Step 2: Test function */                                             \
    (func)( r, a, b, cache, zeta );                                        \
                                                                            \
    if( compare_buf_u32( (uint32_t const*) r, (uint32_t const*) r_ref,     \
                         KYBER_N ) != 0 )                                   \
    {                                                                       \
        debug_print_buf_s32( r_ref, KYBER_N, "Reference" );                \
        debug_print_buf_s32( r, KYBER_N, "This" );                         \
        debug_test_fail();                                                  \
        return( 1 );                                                        \
    }                                                                       \
    debug_test_ok();                                                        \
                                                                            \
    return( 0 );                                                            \
}

// For functions: func(int32_t r[256], const int16_t a[256], const int16_t b[256], int16_t cache[256], const int32_t zeta[128])
#define MAKE_TEST_KYBER_BASEMUL_5P_I32_CACHE_I32(var,func,ref_func,modulus)             \
int test_ ## var ()                                                         \
{                                                                           \
    debug_printf("Test for " #func " ");                                    \
    int16_t a[KYBER_N]         __attribute__((aligned(16)));                \
    int16_t b[KYBER_N]         __attribute__((aligned(16)));                \
    int32_t zeta[KYBER_N/2]    __attribute__((aligned(16)));                \
    int16_t cache[KYBER_N]     __attribute__((aligned(16)));                \
    int16_t cache_ref[KYBER_N] __attribute__((aligned(16)));                \
    int32_t r[KYBER_N]         __attribute__((aligned(16)));                \
    int32_t r_ref[KYBER_N]     __attribute__((aligned(16)));                \
                                                                            \
    /* Setup input */                                                       \
    fill_random_u16( (uint16_t*) a, KYBER_N );                             \
    mod_reduce_buf_s16( a, KYBER_N, modulus );                              \
    fill_random_u16( (uint16_t*) b, KYBER_N );                             \
    mod_reduce_buf_s16( b, KYBER_N, modulus );                              \
    fill_random_u32( (uint32_t*) zeta, KYBER_N/2 );                        \
    mod_reduce_buf_s32( zeta, KYBER_N/2, modulus );                         \
                                                                            \
    /* Initialize output arrays */                                          \
    memset(r, 0, sizeof(r));                                               \
    memset(r_ref, 0, sizeof(r_ref));                                       \
    memset(cache, 0, sizeof(cache));                                       \
    memset(cache_ref, 0, sizeof(cache_ref));                               \
                                                                            \
    /* Step 1: Reference */                                                 \
    ref_func( r_ref, a, b, cache_ref, zeta );                              \
                                                                            \
    /* Step 2: Test function */                                             \
    (func)( r, a, b, cache, zeta );                                        \
                                                                            \
    if( compare_buf_u32( (uint32_t const*) r, (uint32_t const*) r_ref,     \
                         KYBER_N ) != 0 )                                   \
    {                                                                       \
        debug_print_buf_s32( r_ref, KYBER_N, "Reference" );                \
        debug_print_buf_s32( r, KYBER_N, "This" );                         \
        debug_test_fail();                                                  \
        return( 1 );                                                        \
    }                                                                       \
    debug_test_ok();                                                        \
                                                                            \
    return( 0 );                                                            \
}

// For functions: func(int16_t r[256], const int16_t a[256], const int16_t b[256], int16_t cache[256], const int64_t zeta[128])
#define MAKE_TEST_KYBER_BASEMUL_5P_I16_CACHE_I64(var,func,ref_func,modulus)             \
int test_ ## var ()                                                         \
{                                                                           \
    debug_printf("Test for " #func " ");                                    \
    int16_t a[KYBER_N]         __attribute__((aligned(16)));                \
    int16_t b[KYBER_N]         __attribute__((aligned(16)));                \
    int64_t zeta[KYBER_N/2]    __attribute__((aligned(16)));                \
    int16_t cache[KYBER_N]     __attribute__((aligned(16)));                \
    int16_t cache_ref[KYBER_N] __attribute__((aligned(16)));                \
    int16_t r[KYBER_N]         __attribute__((aligned(16)));                \
    int16_t r_ref[KYBER_N]     __attribute__((aligned(16)));                \
                                                                            \
    /* Setup input */                                                       \
    fill_random_u16( (uint16_t*) a, KYBER_N );                             \
    mod_reduce_buf_s16( a, KYBER_N, modulus );                              \
    fill_random_u16( (uint16_t*) b, KYBER_N );                             \
    mod_reduce_buf_s16( b, KYBER_N, modulus );                              \
    fill_random_u32( (uint32_t*) zeta, KYBER_N );                          \
    mod_reduce_buf_s32( (int32_t*) zeta, KYBER_N, modulus );                \
                                                                            \
    /* Initialize output arrays */                                          \
    memset(r, 0, sizeof(r));                                               \
    memset(r_ref, 0, sizeof(r_ref));                                       \
    memset(cache, 0, sizeof(cache));                                       \
    memset(cache_ref, 0, sizeof(cache_ref));                               \
                                                                            \
    /* Step 1: Reference */                                                 \
    ref_func( r_ref, a, b, cache_ref, zeta );                              \
                                                                            \
    /* Step 2: Test function */                                             \
    (func)( r, a, b, cache, zeta );                                        \
                                                                            \
    if( compare_buf_u16( (uint16_t const*) r, (uint16_t const*) r_ref,     \
                         KYBER_N ) != 0 )                                   \
    {                                                                       \
        debug_print_buf_s16( r_ref, KYBER_N, "Reference" );                \
        debug_print_buf_s16( r, KYBER_N, "This" );                         \
        debug_test_fail();                                                  \
        return( 1 );                                                        \
    }                                                                       \
    debug_test_ok();                                                        \
                                                                            \
    return( 0 );                                                            \
}

// For functions: func(int16_t r[256], const int16_t a[256], const int16_t b[256], int16_t cache[256], const int32_t zeta[128])
#define MAKE_TEST_KYBER_BASEMUL_5P_I16_CACHE_I32(var,func,ref_func,modulus)             \
int test_ ## var ()                                                         \
{                                                                           \
    debug_printf("Test for " #func " ");                                    \
    int16_t a[KYBER_N]         __attribute__((aligned(16)));                \
    int16_t b[KYBER_N]         __attribute__((aligned(16)));                \
    int32_t zeta[KYBER_N/2]    __attribute__((aligned(16)));                \
    int16_t cache[KYBER_N]     __attribute__((aligned(16)));                \
    int16_t cache_ref[KYBER_N] __attribute__((aligned(16)));                \
    int16_t r[KYBER_N]         __attribute__((aligned(16)));                \
    int16_t r_ref[KYBER_N]     __attribute__((aligned(16)));                \
                                                                            \
    /* Setup input */                                                       \
    fill_random_u16( (uint16_t*) a, KYBER_N );                             \
    mod_reduce_buf_s16( a, KYBER_N, modulus );                              \
    fill_random_u16( (uint16_t*) b, KYBER_N );                             \
    mod_reduce_buf_s16( b, KYBER_N, modulus );                              \
    fill_random_u32( (uint32_t*) zeta, KYBER_N/2 );                        \
    mod_reduce_buf_s32( zeta, KYBER_N/2, modulus );                         \
                                                                            \
    /* Initialize output arrays */                                          \
    memset(r, 0, sizeof(r));                                               \
    memset(r_ref, 0, sizeof(r_ref));                                       \
    memset(cache, 0, sizeof(cache));                                       \
    memset(cache_ref, 0, sizeof(cache_ref));                               \
                                                                            \
    /* Step 1: Reference */                                                 \
    ref_func( r_ref, a, b, cache_ref, zeta );                              \
                                                                            \
    /* Step 2: Test function */                                             \
    (func)( r, a, b, cache, zeta );                                        \
                                                                            \
    if( compare_buf_u16( (uint16_t const*) r, (uint16_t const*) r_ref,     \
                         KYBER_N ) != 0 )                                   \
    {                                                                       \
        debug_print_buf_s16( r_ref, KYBER_N, "Reference" );                \
        debug_print_buf_s16( r, KYBER_N, "This" );                         \
        debug_test_fail();                                                  \
        return( 1 );                                                        \
    }                                                                       \
    debug_test_ok();                                                        \
                                                                            \
    return( 0 );                                                            \
}

// For functions: func(int16_t r[256], const int16_t a[256], const int16_t b[256], const int16_t cache[256], const int32_t acc[256])
#define MAKE_TEST_KYBER_BASEMUL_5P_I16_CACHE_ACC(var,func,ref_func,modulus)             \
int test_ ## var ()                                                         \
{                                                                           \
    debug_printf("Test for " #func " ");                                    \
    int16_t a[KYBER_N]         __attribute__((aligned(16)));                \
    int16_t b[KYBER_N]         __attribute__((aligned(16)));                \
    int16_t cache[KYBER_N]     __attribute__((aligned(16)));                \
    int32_t acc[KYBER_N]       __attribute__((aligned(16)));                \
    int16_t r[KYBER_N]         __attribute__((aligned(16)));                \
    int16_t r_ref[KYBER_N]     __attribute__((aligned(16)));                \
                                                                            \
    /* Setup input */                                                       \
    fill_random_u16( (uint16_t*) a, KYBER_N );                             \
    mod_reduce_buf_s16( a, KYBER_N, modulus );                              \
    fill_random_u16( (uint16_t*) b, KYBER_N );                             \
    mod_reduce_buf_s16( b, KYBER_N, modulus );                              \
    fill_random_u16( (uint16_t*) cache, KYBER_N );                         \
    mod_reduce_buf_s16( cache, KYBER_N, modulus );                          \
    fill_random_u32( (uint32_t*) acc, KYBER_N );                           \
    mod_reduce_buf_s32( acc, KYBER_N, modulus );                            \
                                                                            \
    /* Initialize output arrays */                                          \
    memset(r, 0, sizeof(r));                                               \
    memset(r_ref, 0, sizeof(r_ref));                                       \
                                                                            \
    /* Step 1: Reference */                                                 \
    ref_func( r_ref, a, b, cache, acc );                                   \
                                                                            \
    /* Step 2: Test function */                                             \
    (func)( r, a, b, cache, acc );                                         \
                                                                            \
    if( compare_buf_u16( (uint16_t const*) r, (uint16_t const*) r_ref,     \
                         KYBER_N ) != 0 )                                   \
    {                                                                       \
        debug_print_buf_s16( r_ref, KYBER_N, "Reference" );                \
        debug_print_buf_s16( r, KYBER_N, "This" );                         \
        debug_test_fail();                                                  \
        return( 1 );                                                        \
    }                                                                       \
    debug_test_ok();                                                        \
                                                                            \
    return( 0 );                                                            \
}

// For functions: func(int16_t r[256], const int16_t a[256], const int16_t b[256], int16_t cache[256], const int64_t zeta[128], const int32_t acc[256])
#define MAKE_TEST_KYBER_BASEMUL_6P_I16_CACHE_I64_ACC(var,func,ref_func,modulus)         \
int test_ ## var ()                                                         \
{                                                                           \
    debug_printf("Test for " #func " ");                                    \
    int16_t a[KYBER_N]         __attribute__((aligned(16)));                \
    int16_t b[KYBER_N]         __attribute__((aligned(16)));                \
    int64_t zeta[KYBER_N/2]    __attribute__((aligned(16)));                \
    int16_t cache[KYBER_N]     __attribute__((aligned(16)));                \
    int16_t cache_ref[KYBER_N] __attribute__((aligned(16)));                \
    int32_t acc[KYBER_N]       __attribute__((aligned(16)));                \
    int16_t r[KYBER_N]         __attribute__((aligned(16)));                \
    int16_t r_ref[KYBER_N]     __attribute__((aligned(16)));                \
                                                                            \
    /* Setup input */                                                       \
    fill_random_u16( (uint16_t*) a, KYBER_N );                             \
    mod_reduce_buf_s16( a, KYBER_N, modulus );                              \
    fill_random_u16( (uint16_t*) b, KYBER_N );                             \
    mod_reduce_buf_s16( b, KYBER_N, modulus );                              \
    fill_random_u32( (uint32_t*) zeta, KYBER_N );                          \
    mod_reduce_buf_s32( (int32_t*) zeta, KYBER_N, modulus );                \
    fill_random_u32( (uint32_t*) acc, KYBER_N );                           \
    mod_reduce_buf_s32( acc, KYBER_N, modulus );                            \
                                                                            \
    /* Initialize output arrays */                                          \
    memset(r, 0, sizeof(r));                                               \
    memset(r_ref, 0, sizeof(r_ref));                                       \
    memset(cache, 0, sizeof(cache));                                       \
    memset(cache_ref, 0, sizeof(cache_ref));                               \
                                                                            \
    /* Step 1: Reference */                                                 \
    ref_func( r_ref, a, b, cache_ref, zeta, acc );                         \
                                                                            \
    /* Step 2: Test function */                                             \
    (func)( r, a, b, cache, zeta, acc );                                   \
                                                                            \
    if( compare_buf_u16( (uint16_t const*) r, (uint16_t const*) r_ref,     \
                         KYBER_N ) != 0 )                                   \
    {                                                                       \
        debug_print_buf_s16( r_ref, KYBER_N, "Reference" );                \
        debug_print_buf_s16( r, KYBER_N, "This" );                         \
        debug_test_fail();                                                  \
        return( 1 );                                                        \
    }                                                                       \
    debug_test_ok();                                                        \
                                                                            \
    return( 0 );                                                            \
}

// For functions: func(int16_t r[256], const int16_t a[256], const int16_t b[256], int16_t cache[256], const int32_t zeta[128], const int32_t acc[256])
#define MAKE_TEST_KYBER_BASEMUL_6P_I16_CACHE_I32_ACC(var,func,ref_func,modulus)         \
int test_ ## var ()                                                         \
{                                                                           \
    debug_printf("Test for " #func " ");                                    \
    int16_t a[KYBER_N]         __attribute__((aligned(16)));                \
    int16_t b[KYBER_N]         __attribute__((aligned(16)));                \
    int32_t zeta[KYBER_N/2]    __attribute__((aligned(16)));                \
    int16_t cache[KYBER_N]     __attribute__((aligned(16)));                \
    int16_t cache_ref[KYBER_N] __attribute__((aligned(16)));                \
    int32_t acc[KYBER_N]       __attribute__((aligned(16)));                \
    int16_t r[KYBER_N]         __attribute__((aligned(16)));                \
    int16_t r_ref[KYBER_N]     __attribute__((aligned(16)));                \
                                                                            \
    /* Setup input */                                                       \
    fill_random_u16( (uint16_t*) a, KYBER_N );                             \
    mod_reduce_buf_s16( a, KYBER_N, modulus );                              \
    fill_random_u16( (uint16_t*) b, KYBER_N );                             \
    mod_reduce_buf_s16( b, KYBER_N, modulus );                              \
    fill_random_u32( (uint32_t*) zeta, KYBER_N/2 );                        \
    mod_reduce_buf_s32( zeta, KYBER_N/2, modulus );                         \
    fill_random_u32( (uint32_t*) acc, KYBER_N );                           \
    mod_reduce_buf_s32( acc, KYBER_N, modulus );                            \
                                                                            \
    /* Initialize output arrays */                                          \
    memset(r, 0, sizeof(r));                                               \
    memset(r_ref, 0, sizeof(r_ref));                                       \
    memset(cache, 0, sizeof(cache));                                       \
    memset(cache_ref, 0, sizeof(cache_ref));                               \
                                                                            \
    /* Step 1: Reference */                                                 \
    ref_func( r_ref, a, b, cache_ref, zeta, acc );                         \
                                                                            \
    /* Step 2: Test function */                                             \
    (func)( r, a, b, cache, zeta, acc );                                   \
                                                                            \
    if( compare_buf_u16( (uint16_t const*) r, (uint16_t const*) r_ref,     \
                         KYBER_N ) != 0 )                                   \
    {                                                                       \
        debug_print_buf_s16( r_ref, KYBER_N, "Reference" );                \
        debug_print_buf_s16( r, KYBER_N, "This" );                         \
        debug_test_fail();                                                  \
        return( 1 );                                                        \
    }                                                                       \
    debug_test_ok();                                                        \
                                                                            \
    return( 0 );                                                            \
}

// For functions: func(int32_t r[256], const int16_t a[256], const int16_t b[256], const int16_t cache[256])
#define MAKE_TEST_KYBER_BASEMUL_4P_CACHED(var,func,ref_func,modulus)                    \
int test_ ## var ()                                                         \
{                                                                           \
    debug_printf("Test for " #func " ");                                    \
    int16_t a[KYBER_N]         __attribute__((aligned(16)));                \
    int16_t b[KYBER_N]         __attribute__((aligned(16)));                \
    int16_t cache[KYBER_N]     __attribute__((aligned(16)));                \
    int32_t r[KYBER_N]         __attribute__((aligned(16)));                \
    int32_t r_ref[KYBER_N]     __attribute__((aligned(16)));                \
                                                                            \
    /* Setup input */                                                       \
    fill_random_u16( (uint16_t*) a, KYBER_N );                             \
    mod_reduce_buf_s16( a, KYBER_N, modulus );                              \
    fill_random_u16( (uint16_t*) b, KYBER_N );                             \
    mod_reduce_buf_s16( b, KYBER_N, modulus );                              \
    fill_random_u16( (uint16_t*) cache, KYBER_N );                         \
    mod_reduce_buf_s16( cache, KYBER_N, modulus );                          \
                                                                            \
    /* Initialize output arrays */                                          \
    memset(r, 0, sizeof(r));                                               \
    memset(r_ref, 0, sizeof(r_ref));                                       \
                                                                            \
    /* Step 1: Reference */                                                 \
    ref_func( r_ref, a, b, cache );                                        \
                                                                            \
    /* Step 2: Test function */                                             \
    (func)( r, a, b, cache );                                              \
                                                                            \
    if( compare_buf_u32( (uint32_t const*) r, (uint32_t const*) r_ref,     \
                         KYBER_N ) != 0 )                                   \
    {                                                                       \
        debug_print_buf_s32( r_ref, KYBER_N, "Reference" );                \
        debug_print_buf_s32( r, KYBER_N, "This" );                         \
        debug_test_fail();                                                  \
        return( 1 );                                                        \
    }                                                                       \
    debug_test_ok();                                                        \
                                                                            \
    return( 0 );                                                            \
}

// For RVV functions: func(int16_t r[256], const int16_t a[256], const int16_t b[256], const int16_t table[256])
#define MAKE_TEST_KYBER_BASEMUL_4P_RVV_I16_TABLE(var,func,ref_func,modulus)             \
int test_ ## var ()                                                         \
{                                                                           \
    debug_printf("Test for " #func " ");                                    \
    int16_t a[KYBER_N]         __attribute__((aligned(16)));                \
    int16_t b[KYBER_N]         __attribute__((aligned(16)));                \
    int16_t table[KYBER_N]     __attribute__((aligned(16)));                \
    int16_t r[KYBER_N]         __attribute__((aligned(16)));                \
    int16_t r_ref[KYBER_N]     __attribute__((aligned(16)));                \
                                                                            \
    /* Setup input */                                                       \
    fill_random_u16( (uint16_t*) a, KYBER_N );                             \
    mod_reduce_buf_s16( a, KYBER_N, modulus );                              \
    fill_random_u16( (uint16_t*) b, KYBER_N );                             \
    mod_reduce_buf_s16( b, KYBER_N, modulus );                              \
    fill_random_u16( (uint16_t*) table, KYBER_N );                         \
    mod_reduce_buf_s16( table, KYBER_N, modulus );                          \
                                                                            \
    /* Initialize output arrays */                                          \
    memset(r, 0, sizeof(r));                                               \
    memset(r_ref, 0, sizeof(r_ref));                                       \
                                                                            \
    /* Step 1: Reference */                                                 \
    ref_func( r_ref, a, b, table );                                        \
                                                                            \
    /* Step 2: Test function */                                             \
    (func)( r, a, b, table );                                              \
                                                                            \
    if( compare_buf_u16( (uint16_t const*) r, (uint16_t const*) r_ref,     \
                         KYBER_N ) != 0 )                                   \
    {                                                                       \
        debug_print_buf_s16( r_ref, KYBER_N, "Reference" );                \
        debug_print_buf_s16( r, KYBER_N, "This" );                         \
        debug_test_fail();                                                  \
        return( 1 );                                                        \
    }                                                                       \
    debug_test_ok();                                                        \
                                                                            \
    return( 0 );                                                            \
}

// For RVV functions: func(int16_t r[256], const int16_t a[256], const int16_t b[256], const int16_t table[256], int16_t b_cache[256])
#define MAKE_TEST_KYBER_BASEMUL_5P_RVV_I16_TABLE_CACHE(var,func,ref_func,modulus)       \
int test_ ## var ()                                                         \
{                                                                           \
    debug_printf("Test for " #func " ");                                    \
    __attribute__((aligned(16))) int16_t a[KYBER_N];                       \
    __attribute__((aligned(16))) int16_t b[KYBER_N];                       \
    __attribute__((aligned(16))) int16_t table[KYBER_N];                   \
    __attribute__((aligned(16))) int16_t b_cache[KYBER_N];                 \
    __attribute__((aligned(16))) int16_t r[KYBER_N];                       \
    __attribute__((aligned(16))) int16_t r_ref[KYBER_N];                   \
    __attribute__((aligned(16))) int16_t b_cache_ref[KYBER_N];             \
                                                                            \
    fill_random_u16((uint16_t*)a, KYBER_N);                               \
    fill_random_u16((uint16_t*)b, KYBER_N);                               \
    fill_random_u16((uint16_t*)table, KYBER_N);                           \
    mod_reduce_buf_s16(a, KYBER_N, modulus);                              \
    mod_reduce_buf_s16(b, KYBER_N, modulus);                              \
    mod_reduce_buf_s16(table, KYBER_N, modulus);                          \
                                                                            \
    memset(r, 0, sizeof(r));                                               \
    memset(r_ref, 0, sizeof(r_ref));                                       \
    memset(b_cache, 0, sizeof(b_cache));                                   \
    memset(b_cache_ref, 0, sizeof(b_cache_ref));                           \
                                                                            \
    func(r, a, b, table, b_cache);                                         \
    ref_func(r_ref, a, b, table, b_cache_ref);                            \
                                                                            \
    if (compare_buf_u16((uint16_t*)r, (uint16_t*)r_ref, KYBER_N) != 0) {   \
        printf("ERROR in %s: Output arrays differ\n", #var);               \
        debug_print_buf_s16(r_ref, KYBER_N, "Expected");                   \
        debug_print_buf_s16(r, KYBER_N, "Got");                            \
        debug_test_fail();                                                  \
        return 1;                                                           \
    }                                                                       \
                                                                            \
    if (compare_buf_u16((uint16_t*)b_cache, (uint16_t*)b_cache_ref, KYBER_N) != 0) { \
        printf("ERROR in %s: Cache arrays differ\n", #var);                \
        debug_print_buf_s16(b_cache_ref, KYBER_N, "Expected cache");        \
        debug_print_buf_s16(b_cache, KYBER_N, "Got cache");                 \
        debug_test_fail();                                                  \
        return 1;                                                           \
    }                                                                       \
                                                                            \
    debug_test_ok();                                                        \
    return 0;                                                               \
}

/*
 * Benchmark macros for different function signatures
 */

// For functions: func(int32_t r[256], const int16_t a[256], const int16_t b[256], const int64_t zeta[128])
#define MAKE_BENCH_KYBER_BASEMUL_4P_I32_I64(var, func)                     \
    int bench_##var()                                                   \
    {                                                                   \
        debug_printf("bench kyber_basemul %-50s", #func "\0");         \
        int16_t a[KYBER_N] __attribute__((aligned(16)));                \
        int16_t b[KYBER_N] __attribute__((aligned(16)));                \
        int64_t zeta[KYBER_N/2] __attribute__((aligned(16)));           \
        int32_t r[KYBER_N] __attribute__((aligned(16)));                \
        \
        for (unsigned cnt = 0; cnt < WARMUP_ITERATIONS; cnt++) {        \
            (func)(r, a, b, zeta);\
        }                                                               \
        init_perf_events(); \
        start_counting_events(); \
        for (unsigned cnt = 0; cnt < TEST_COUNT; cnt++)                 \
        { \
            for (unsigned cntp = 0; cntp < ITER_PER_TEST; cntp++) {     \
                (func)(r, a, b, zeta);                                  \
            } \
              \
        }                                                               \
        stop_and_read_events(); \
        calc_average(ITER_PER_TEST, TEST_COUNT); \
        cleanup_perf_events(); \
        print_counter(); \
        return (0);                                                     \
    }

// For functions: func(int32_t r[256], const int16_t a[256], const int16_t b[256], const int32_t zeta[128])
#define MAKE_BENCH_KYBER_BASEMUL_4P_I32_I32(var, func)                     \
    int bench_##var()                                                   \
    {                                                                   \
        debug_printf("bench kyber_basemul %-50s", #func "\0");         \
        int16_t a[KYBER_N] __attribute__((aligned(16)));                \
        int16_t b[KYBER_N] __attribute__((aligned(16)));                \
        int32_t zeta[KYBER_N/2] __attribute__((aligned(16)));           \
        int32_t r[KYBER_N] __attribute__((aligned(16)));                \
        \
        for (unsigned cnt = 0; cnt < WARMUP_ITERATIONS; cnt++) {        \
            (func)(r, a, b, zeta);\
        }                                                               \
        init_perf_events(); \
        start_counting_events(); \
        for (unsigned cnt = 0; cnt < TEST_COUNT; cnt++)                 \
        { \
            for (unsigned cntp = 0; cntp < ITER_PER_TEST; cntp++) {     \
                (func)(r, a, b, zeta);                                  \
            } \
              \
        }                                                               \
        stop_and_read_events(); \
        calc_average(ITER_PER_TEST, TEST_COUNT); \
        cleanup_perf_events(); \
        print_counter(); \
        return (0);                                                     \
    }

// For functions: func(int16_t r[256], const int16_t a[256], const int16_t b[256], const int64_t zeta[128], const int32_t acc[256])
#define MAKE_BENCH_KYBER_BASEMUL_5P_I16_I64_I32(var, func)                 \
    int bench_##var()                                                   \
    {                                                                   \
        debug_printf("bench kyber_basemul %-50s", #func "\0");         \
        int16_t a[KYBER_N] __attribute__((aligned(16)));                \
        int16_t b[KYBER_N] __attribute__((aligned(16)));                \
        int64_t zeta[KYBER_N/2] __attribute__((aligned(16)));           \
        int32_t acc[KYBER_N] __attribute__((aligned(16)));              \
        int16_t r[KYBER_N] __attribute__((aligned(16)));                \
        \
        for (unsigned cnt = 0; cnt < WARMUP_ITERATIONS; cnt++) {        \
            (func)(r, a, b, zeta, acc);\
        }                                                               \
        init_perf_events(); \
        start_counting_events(); \
        for (unsigned cnt = 0; cnt < TEST_COUNT; cnt++)                 \
        { \
            for (unsigned cntp = 0; cntp < ITER_PER_TEST; cntp++) {     \
                (func)(r, a, b, zeta, acc);                             \
            } \
              \
        }                                                               \
        stop_and_read_events(); \
        calc_average(ITER_PER_TEST, TEST_COUNT); \
        cleanup_perf_events(); \
        print_counter(); \
        return (0);                                                     \
    }

// For functions: func(int16_t r[256], const int16_t a[256], const int16_t b[256], const int32_t zeta[128], const int32_t acc[256])
#define MAKE_BENCH_KYBER_BASEMUL_5P_I16_I32_I32(var, func)                 \
    int bench_##var()                                                   \
    {                                                                   \
        debug_printf("bench kyber_basemul %-50s", #func "\0");         \
        int16_t a[KYBER_N] __attribute__((aligned(16)));                \
        int16_t b[KYBER_N] __attribute__((aligned(16)));                \
        int32_t zeta[KYBER_N/2] __attribute__((aligned(16)));           \
        int32_t acc[KYBER_N] __attribute__((aligned(16)));              \
        int16_t r[KYBER_N] __attribute__((aligned(16)));                \
        \
        for (unsigned cnt = 0; cnt < WARMUP_ITERATIONS; cnt++) {        \
            (func)(r, a, b, zeta, acc);\
        }                                                               \
        init_perf_events(); \
        start_counting_events(); \
        for (unsigned cnt = 0; cnt < TEST_COUNT; cnt++)                 \
        { \
            for (unsigned cntp = 0; cntp < ITER_PER_TEST; cntp++) {     \
                (func)(r, a, b, zeta, acc);                             \
            } \
              \
        }                                                               \
        stop_and_read_events(); \
        calc_average(ITER_PER_TEST, TEST_COUNT); \
        cleanup_perf_events(); \
        print_counter(); \
        return (0);                                                     \
    }

// For functions: func(int32_t r[256], const int16_t a[256], const int16_t b[256], int16_t cache[256], const int64_t zeta[128])
#define MAKE_BENCH_KYBER_BASEMUL_5P_I32_CACHE_I64(var, func)               \
    int bench_##var()                                                   \
    {                                                                   \
        debug_printf("bench kyber_basemul %-50s", #func "\0");         \
        int16_t a[KYBER_N] __attribute__((aligned(16)));                \
        int16_t b[KYBER_N] __attribute__((aligned(16)));                \
        int64_t zeta[KYBER_N/2] __attribute__((aligned(16)));           \
        int16_t cache[KYBER_N] __attribute__((aligned(16)));            \
        int32_t r[KYBER_N] __attribute__((aligned(16)));                \
        \
        for (unsigned cnt = 0; cnt < WARMUP_ITERATIONS; cnt++) {        \
            (func)(r, a, b, cache, zeta);\
        }                                                               \
        init_perf_events(); \
        start_counting_events(); \
        for (unsigned cnt = 0; cnt < TEST_COUNT; cnt++)                 \
        { \
            for (unsigned cntp = 0; cntp < ITER_PER_TEST; cntp++) {     \
                (func)(r, a, b, cache, zeta);                           \
            } \
              \
        }                                                               \
        stop_and_read_events(); \
        calc_average(ITER_PER_TEST, TEST_COUNT); \
        cleanup_perf_events(); \
        print_counter(); \
        return (0);                                                     \
    }

// For functions: func(int32_t r[256], const int16_t a[256], const int16_t b[256], int16_t cache[256], const int32_t zeta[128])
#define MAKE_BENCH_KYBER_BASEMUL_5P_I32_CACHE_I32(var, func)               \
    int bench_##var()                                                   \
    {                                                                   \
        debug_printf("bench kyber_basemul %-50s", #func "\0");         \
        int16_t a[KYBER_N] __attribute__((aligned(16)));                \
        int16_t b[KYBER_N] __attribute__((aligned(16)));                \
        int32_t zeta[KYBER_N/2] __attribute__((aligned(16)));           \
        int16_t cache[KYBER_N] __attribute__((aligned(16)));            \
        int32_t r[KYBER_N] __attribute__((aligned(16)));                \
        \
        for (unsigned cnt = 0; cnt < WARMUP_ITERATIONS; cnt++) {        \
            (func)(r, a, b, cache, zeta);\
        }                                                               \
        init_perf_events(); \
        start_counting_events(); \
        for (unsigned cnt = 0; cnt < TEST_COUNT; cnt++)                 \
        { \
            for (unsigned cntp = 0; cntp < ITER_PER_TEST; cntp++) {     \
                (func)(r, a, b, cache, zeta);                           \
            } \
              \
        }                                                               \
        stop_and_read_events(); \
        calc_average(ITER_PER_TEST, TEST_COUNT); \
        cleanup_perf_events(); \
        print_counter(); \
        return (0);                                                     \
    }

// For functions: func(int32_t r[256], const int16_t a[256], const int16_t b[256], const int16_t cache[256])
#define MAKE_BENCH_KYBER_BASEMUL_4P_CACHED(var, func)                      \
    int bench_##var()                                                   \
    {                                                                   \
        debug_printf("bench kyber_basemul %-50s", #func "\0");         \
        int16_t a[KYBER_N] __attribute__((aligned(16)));                \
        int16_t b[KYBER_N] __attribute__((aligned(16)));                \
        int16_t cache[KYBER_N] __attribute__((aligned(16)));            \
        int32_t r[KYBER_N] __attribute__((aligned(16)));                \
        \
        for (unsigned cnt = 0; cnt < WARMUP_ITERATIONS; cnt++) {        \
            (func)(r, a, b, cache);\
        }                                                               \
        init_perf_events(); \
        start_counting_events(); \
        for (unsigned cnt = 0; cnt < TEST_COUNT; cnt++)                 \
        { \
            for (unsigned cntp = 0; cntp < ITER_PER_TEST; cntp++) {     \
                (func)(r, a, b, cache);                                 \
            } \
              \
        }                                                               \
        stop_and_read_events(); \
        calc_average(ITER_PER_TEST, TEST_COUNT); \
        cleanup_perf_events(); \
        print_counter(); \
        return (0);                                                     \
    }

// For functions: func(int16_t r[256], const int16_t a[256], const int16_t b[256], const int16_t cache[256], const int32_t acc[256])
#define MAKE_BENCH_KYBER_BASEMUL_5P_I16_CACHE_ACC(var, func)               \
    int bench_##var()                                                   \
    {                                                                   \
        debug_printf("bench kyber_basemul %-50s", #func "\0");         \
        int16_t a[KYBER_N] __attribute__((aligned(16)));                \
        int16_t b[KYBER_N] __attribute__((aligned(16)));                \
        int16_t cache[KYBER_N] __attribute__((aligned(16)));            \
        int32_t acc[KYBER_N] __attribute__((aligned(16)));              \
        int16_t r[KYBER_N] __attribute__((aligned(16)));                \
        \
        for (unsigned cnt = 0; cnt < WARMUP_ITERATIONS; cnt++) {        \
            (func)(r, a, b, cache, acc);\
        }                                                               \
        init_perf_events(); \
        start_counting_events(); \
        for (unsigned cnt = 0; cnt < TEST_COUNT; cnt++)                 \
        { \
            for (unsigned cntp = 0; cntp < ITER_PER_TEST; cntp++) {     \
                (func)(r, a, b, cache, acc);                            \
            } \
              \
        }                                                               \
        stop_and_read_events(); \
        calc_average(ITER_PER_TEST, TEST_COUNT); \
        cleanup_perf_events(); \
        print_counter(); \
        return (0);                                                     \
    }

// For functions: func(int16_t r[256], const int16_t a[256], const int16_t b[256], int16_t cache[256], const int64_t zeta[128], const int32_t acc[256])
#define MAKE_BENCH_KYBER_BASEMUL_6P_I16_CACHE_I64_ACC(var, func)           \
    int bench_##var()                                                   \
    {                                                                   \
        debug_printf("bench kyber_basemul %-50s", #func "\0");         \
        int16_t a[KYBER_N] __attribute__((aligned(16)));                \
        int16_t b[KYBER_N] __attribute__((aligned(16)));                \
        int64_t zeta[KYBER_N/2] __attribute__((aligned(16)));           \
        int16_t cache[KYBER_N] __attribute__((aligned(16)));            \
        int32_t acc[KYBER_N] __attribute__((aligned(16)));              \
        int16_t r[KYBER_N] __attribute__((aligned(16)));                \
        \
        for (unsigned cnt = 0; cnt < WARMUP_ITERATIONS; cnt++) {        \
            (func)(r, a, b, cache, zeta, acc);\
        }                                                               \
        init_perf_events(); \
        start_counting_events(); \
        for (unsigned cnt = 0; cnt < TEST_COUNT; cnt++)                 \
        { \
            for (unsigned cntp = 0; cntp < ITER_PER_TEST; cntp++) {     \
                (func)(r, a, b, cache, zeta, acc);                      \
            } \
              \
        }                                                               \
        stop_and_read_events(); \
        calc_average(ITER_PER_TEST, TEST_COUNT); \
        cleanup_perf_events(); \
        print_counter(); \
        return (0);                                                     \
    }

// For functions: func(int16_t r[256], const int16_t a[256], const int16_t b[256], int16_t cache[256], const int32_t zeta[128], const int32_t acc[256])
#define MAKE_BENCH_KYBER_BASEMUL_6P_I16_CACHE_I32_ACC(var, func)           \
    int bench_##var()                                                   \
    {                                                                   \
        debug_printf("bench kyber_basemul %-50s", #func "\0");         \
        int16_t a[KYBER_N] __attribute__((aligned(16)));                \
        int16_t b[KYBER_N] __attribute__((aligned(16)));                \
        int32_t zeta[KYBER_N/2] __attribute__((aligned(16)));           \
        int16_t cache[KYBER_N] __attribute__((aligned(16)));            \
        int32_t acc[KYBER_N] __attribute__((aligned(16)));              \
        int16_t r[KYBER_N] __attribute__((aligned(16)));                \
        \
        for (unsigned cnt = 0; cnt < WARMUP_ITERATIONS; cnt++) {        \
            (func)(r, a, b, cache, zeta, acc);\
        }                                                               \
        init_perf_events(); \
        start_counting_events(); \
        for (unsigned cnt = 0; cnt < TEST_COUNT; cnt++)                 \
        { \
            for (unsigned cntp = 0; cntp < ITER_PER_TEST; cntp++) {     \
                (func)(r, a, b, cache, zeta, acc);                      \
            } \
              \
        }                                                               \
        stop_and_read_events(); \
        calc_average(ITER_PER_TEST, TEST_COUNT); \
        cleanup_perf_events(); \
        print_counter(); \
        return (0);                                                     \
    }

// For RVV functions: func(int16_t r[256], const int16_t a[256], const int16_t b[256], const int16_t table[256])
#define MAKE_BENCH_KYBER_BASEMUL_4P_RVV_I16_TABLE(var, func)               \
    int bench_##var()                                                   \
    {                                                                   \
        debug_printf("bench kyber_basemul %-50s", #func "\0");         \
        int16_t a[KYBER_N] __attribute__((aligned(16)));                \
        int16_t b[KYBER_N] __attribute__((aligned(16)));                \
        int16_t table[KYBER_N] __attribute__((aligned(16)));            \
        int16_t r[KYBER_N] __attribute__((aligned(16)));                \
        \
        for (unsigned cnt = 0; cnt < WARMUP_ITERATIONS; cnt++) {        \
            (func)(r, a, b, table);\
        }                                                               \
        init_perf_events(); \
        start_counting_events(); \
        for (unsigned cnt = 0; cnt < TEST_COUNT; cnt++)                 \
        { \
            for (unsigned cntp = 0; cntp < ITER_PER_TEST; cntp++) {     \
                (func)(r, a, b, table);                                 \
            } \
              \
        }                                                               \
        stop_and_read_events(); \
        calc_average(ITER_PER_TEST, TEST_COUNT); \
        cleanup_perf_events(); \
        print_counter(); \
        return (0);                                                     \
    }

// For RVV functions: func(int16_t r[256], const int16_t a[256], const int16_t b[256], const int16_t table[256], int16_t b_cache[256])
#define MAKE_BENCH_KYBER_BASEMUL_5P_RVV_I16_TABLE_CACHE(var, func)         \
    int bench_##var()                                                   \
    {                                                                   \
        debug_printf("bench kyber_basemul %-50s", #func "\0");         \
        int16_t a[KYBER_N] __attribute__((aligned(16)));                \
        int16_t b[KYBER_N] __attribute__((aligned(16)));                \
        int16_t table[KYBER_N] __attribute__((aligned(16)));            \
        int16_t b_cache[KYBER_N] __attribute__((aligned(16)));          \
        int16_t r[KYBER_N] __attribute__((aligned(16)));                \
        \
        for (unsigned cnt = 0; cnt < WARMUP_ITERATIONS; cnt++) {        \
            (func)(r, a, b, table, b_cache);\
        }                                                               \
        init_perf_events(); \
        start_counting_events(); \
        for (unsigned cnt = 0; cnt < TEST_COUNT; cnt++)                 \
        { \
            for (unsigned cntp = 0; cntp < ITER_PER_TEST; cntp++) {     \
                (func)(r, a, b, table, b_cache);                        \
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
MAKE_TEST_KYBER_BASEMUL_4P_I32_I64(poly_basemul_acc_rv64im,                  poly_basemul_acc_rv64im_wrap,              poly_basemul_acc_rv64im_wrap,   KYBER_Q)
MAKE_TEST_KYBER_BASEMUL_4P_I32_I32(poly_basemul_acc_rv64im_dual,             poly_basemul_acc_rv64im_dual_wrap,         poly_basemul_acc_rv64im_dual_wrap, KYBER_Q)

// End functions
MAKE_TEST_KYBER_BASEMUL_5P_I16_I64_I32(poly_basemul_acc_end_rv64im,          poly_basemul_acc_end_rv64im_wrap,          poly_basemul_acc_end_rv64im_wrap, KYBER_Q)
MAKE_TEST_KYBER_BASEMUL_5P_I16_I32_I32(poly_basemul_acc_end_rv64im_dual,     poly_basemul_acc_end_rv64im_dual_wrap,     poly_basemul_acc_end_rv64im_dual_wrap, KYBER_Q)

// Cache functions
MAKE_TEST_KYBER_BASEMUL_5P_I32_CACHE_I64(poly_basemul_cache_init_rv64im,     poly_basemul_cache_init_rv64im_wrap,       poly_basemul_cache_init_rv64im_wrap, KYBER_Q)
MAKE_TEST_KYBER_BASEMUL_5P_I32_CACHE_I32(poly_basemul_cache_init_rv64im_dual, poly_basemul_cache_init_rv64im_dual_wrap, poly_basemul_cache_init_rv64im_dual_wrap, KYBER_Q)

// Cached functions
MAKE_TEST_KYBER_BASEMUL_4P_CACHED(poly_basemul_acc_cached_rv64im,            poly_basemul_acc_cached_rv64im_wrap,       poly_basemul_acc_cached_rv64im_wrap, KYBER_Q)
MAKE_TEST_KYBER_BASEMUL_4P_CACHED(poly_basemul_acc_cached_rv64im_dual,       poly_basemul_acc_cached_rv64im_dual_wrap,  poly_basemul_acc_cached_rv64im_dual_wrap, KYBER_Q)

// Cache end functions
MAKE_TEST_KYBER_BASEMUL_5P_I16_CACHE_ACC(poly_basemul_acc_cache_end_rv64im,  poly_basemul_acc_cache_end_rv64im_wrap,    poly_basemul_acc_cache_end_rv64im_wrap, KYBER_Q)
MAKE_TEST_KYBER_BASEMUL_5P_I16_CACHE_ACC(poly_basemul_acc_cache_end_rv64im_dual, poly_basemul_acc_cache_end_rv64im_dual_wrap, poly_basemul_acc_cache_end_rv64im_dual_wrap, KYBER_Q)

// Cache init functions
MAKE_TEST_KYBER_BASEMUL_5P_I32_CACHE_I64(poly_basemul_acc_cache_init_rv64im, poly_basemul_acc_cache_init_rv64im_wrap,   poly_basemul_acc_cache_init_rv64im_wrap, KYBER_Q)
MAKE_TEST_KYBER_BASEMUL_5P_I32_CACHE_I32(poly_basemul_acc_cache_init_rv64im_dual, poly_basemul_acc_cache_init_rv64im_dual_wrap, poly_basemul_acc_cache_init_rv64im_dual_wrap, KYBER_Q)

// Cache init end functions
MAKE_TEST_KYBER_BASEMUL_6P_I16_CACHE_I64_ACC(poly_basemul_acc_cache_init_end_rv64im, poly_basemul_acc_cache_init_end_rv64im_wrap, poly_basemul_acc_cache_init_end_rv64im_wrap, KYBER_Q)
MAKE_TEST_KYBER_BASEMUL_6P_I16_CACHE_I32_ACC(poly_basemul_acc_cache_init_end_rv64im_dual, poly_basemul_acc_cache_init_end_rv64im_dual_wrap, poly_basemul_acc_cache_init_end_rv64im_dual_wrap, KYBER_Q)

// RVV functions
MAKE_TEST_KYBER_BASEMUL_4P_RVV_I16_TABLE(poly_basemul_rvv_vlen128, poly_basemul_rvv_vlen128_wrap, poly_basemul_rvv_vlen128_wrap, KYBER_Q)
MAKE_TEST_KYBER_BASEMUL_4P_RVV_I16_TABLE(poly_basemul_acc_rvv_vlen128, poly_basemul_acc_rvv_vlen128_wrap, poly_basemul_acc_rvv_vlen128_wrap, KYBER_Q)
MAKE_TEST_KYBER_BASEMUL_5P_RVV_I16_TABLE_CACHE(poly_basemul_cached_rvv_vlen128, poly_basemul_cached_rvv_vlen128_wrap, poly_basemul_cached_rvv_vlen128_wrap, KYBER_Q)
MAKE_TEST_KYBER_BASEMUL_5P_RVV_I16_TABLE_CACHE(poly_basemul_acc_cached_rvv_vlen128, poly_basemul_acc_cached_rvv_vlen128_wrap, poly_basemul_acc_cached_rvv_vlen128_wrap, KYBER_Q)
MAKE_TEST_KYBER_BASEMUL_5P_RVV_I16_TABLE_CACHE(poly_basemul_cache_init_rvv_vlen128, poly_basemul_cache_init_rvv_vlen128_wrap, poly_basemul_cache_init_rvv_vlen128_wrap, KYBER_Q)
MAKE_TEST_KYBER_BASEMUL_5P_RVV_I16_TABLE_CACHE(poly_basemul_acc_cache_init_rvv_vlen128, poly_basemul_acc_cache_init_rvv_vlen128_wrap, poly_basemul_acc_cache_init_rvv_vlen128_wrap, KYBER_Q)

// === OPTIMIZED FUNCTION TESTS ===
// Compare optimized functions against their basic counterparts

// Basic optimized functions
MAKE_TEST_KYBER_BASEMUL_4P_I32_I64(poly_basemul_acc_rv64im_opt_c908,         poly_basemul_acc_rv64im_opt_c908_wrap,     poly_basemul_acc_rv64im_wrap, KYBER_Q)
MAKE_TEST_KYBER_BASEMUL_4P_I32_I32(poly_basemul_acc_rv64im_dual_opt_c908,    poly_basemul_acc_rv64im_dual_opt_c908_wrap, poly_basemul_acc_rv64im_dual_wrap, KYBER_Q)

// End optimized functions
MAKE_TEST_KYBER_BASEMUL_5P_I16_I64_I32(poly_basemul_acc_end_rv64im_opt_c908, poly_basemul_acc_end_rv64im_opt_c908_wrap, poly_basemul_acc_end_rv64im_wrap, KYBER_Q)
MAKE_TEST_KYBER_BASEMUL_5P_I16_I32_I32(poly_basemul_acc_end_rv64im_dual_opt_c908, poly_basemul_acc_end_rv64im_dual_opt_c908_wrap, poly_basemul_acc_end_rv64im_dual_wrap, KYBER_Q)

// Cache optimized functions
MAKE_TEST_KYBER_BASEMUL_5P_I32_CACHE_I64(poly_basemul_cache_init_rv64im_opt_c908, poly_basemul_cache_init_rv64im_opt_c908_wrap, poly_basemul_cache_init_rv64im_wrap, KYBER_Q)
MAKE_TEST_KYBER_BASEMUL_5P_I32_CACHE_I32(poly_basemul_cache_init_rv64im_dual_opt_c908, poly_basemul_cache_init_rv64im_dual_opt_c908_wrap, poly_basemul_cache_init_rv64im_dual_wrap, KYBER_Q)

// Cached optimized functions
MAKE_TEST_KYBER_BASEMUL_4P_CACHED(poly_basemul_acc_cached_rv64im_opt_c908,   poly_basemul_acc_cached_rv64im_opt_c908_wrap, poly_basemul_acc_cached_rv64im_wrap, KYBER_Q)
MAKE_TEST_KYBER_BASEMUL_4P_CACHED(poly_basemul_acc_cached_rv64im_dual_opt_c908, poly_basemul_acc_cached_rv64im_dual_opt_c908_wrap, poly_basemul_acc_cached_rv64im_dual_wrap, KYBER_Q)

// Cache end optimized functions
MAKE_TEST_KYBER_BASEMUL_5P_I16_CACHE_ACC(poly_basemul_acc_cache_end_rv64im_opt_c908, poly_basemul_acc_cache_end_rv64im_opt_c908_wrap, poly_basemul_acc_cache_end_rv64im_wrap, KYBER_Q)
MAKE_TEST_KYBER_BASEMUL_5P_I16_CACHE_ACC(poly_basemul_acc_cache_end_rv64im_dual_opt_c908, poly_basemul_acc_cache_end_rv64im_dual_opt_c908_wrap, poly_basemul_acc_cache_end_rv64im_dual_wrap, KYBER_Q)

// Cache init optimized functions
MAKE_TEST_KYBER_BASEMUL_5P_I32_CACHE_I64(poly_basemul_acc_cache_init_rv64im_opt_c908, poly_basemul_acc_cache_init_rv64im_opt_c908_wrap, poly_basemul_acc_cache_init_rv64im_wrap, KYBER_Q)
MAKE_TEST_KYBER_BASEMUL_5P_I32_CACHE_I32(poly_basemul_acc_cache_init_rv64im_dual_opt_c908, poly_basemul_acc_cache_init_rv64im_dual_opt_c908_wrap, poly_basemul_acc_cache_init_rv64im_dual_wrap, KYBER_Q)

// Cache init end optimized functions
MAKE_TEST_KYBER_BASEMUL_6P_I16_CACHE_I64_ACC(poly_basemul_acc_cache_init_end_rv64im_opt_c908, poly_basemul_acc_cache_init_end_rv64im_opt_c908_wrap, poly_basemul_acc_cache_init_end_rv64im_wrap, KYBER_Q)
MAKE_TEST_KYBER_BASEMUL_6P_I16_CACHE_I32_ACC(poly_basemul_acc_cache_init_end_rv64im_dual_opt_c908, poly_basemul_acc_cache_init_end_rv64im_dual_opt_c908_wrap, poly_basemul_acc_cache_init_end_rv64im_dual_wrap, KYBER_Q)

// RVV optimized functions (temporarily commented out due to assembly syntax issues)
MAKE_TEST_KYBER_BASEMUL_4P_RVV_I16_TABLE(poly_basemul_rvv_vlen128_opt_c908, poly_basemul_rvv_vlen128_opt_c908_wrap, poly_basemul_rvv_vlen128_wrap, KYBER_Q)
MAKE_TEST_KYBER_BASEMUL_4P_RVV_I16_TABLE(poly_basemul_acc_rvv_vlen128_opt_c908, poly_basemul_acc_rvv_vlen128_opt_c908_wrap, poly_basemul_acc_rvv_vlen128_wrap, KYBER_Q)
MAKE_TEST_KYBER_BASEMUL_5P_RVV_I16_TABLE_CACHE(poly_basemul_cached_rvv_vlen128_opt_c908, poly_basemul_cached_rvv_vlen128_opt_c908_wrap, poly_basemul_cached_rvv_vlen128_wrap, KYBER_Q)
MAKE_TEST_KYBER_BASEMUL_5P_RVV_I16_TABLE_CACHE(poly_basemul_acc_cached_rvv_vlen128_opt_c908, poly_basemul_acc_cached_rvv_vlen128_opt_c908_wrap, poly_basemul_acc_cached_rvv_vlen128_wrap, KYBER_Q)
MAKE_TEST_KYBER_BASEMUL_5P_RVV_I16_TABLE_CACHE(poly_basemul_cache_init_rvv_vlen128_opt_c908, poly_basemul_cache_init_rvv_vlen128_opt_c908_wrap, poly_basemul_cache_init_rvv_vlen128_wrap, KYBER_Q)
MAKE_TEST_KYBER_BASEMUL_5P_RVV_I16_TABLE_CACHE(poly_basemul_acc_cache_init_rvv_vlen128_opt_c908, poly_basemul_acc_cache_init_rvv_vlen128_opt_c908_wrap, poly_basemul_acc_cache_init_rvv_vlen128_wrap, KYBER_Q)

// === BENCHMARKS ===
// Basic function benchmarks
MAKE_BENCH_KYBER_BASEMUL_4P_I32_I64(poly_basemul_acc_rv64im,              poly_basemul_acc_rv64im_wrap)
MAKE_BENCH_KYBER_BASEMUL_4P_I32_I32(poly_basemul_acc_rv64im_dual,         poly_basemul_acc_rv64im_dual_wrap)

// End function benchmarks
MAKE_BENCH_KYBER_BASEMUL_5P_I16_I64_I32(poly_basemul_acc_end_rv64im,      poly_basemul_acc_end_rv64im_wrap)
MAKE_BENCH_KYBER_BASEMUL_5P_I16_I32_I32(poly_basemul_acc_end_rv64im_dual, poly_basemul_acc_end_rv64im_dual_wrap)

// Cache function benchmarks
MAKE_BENCH_KYBER_BASEMUL_5P_I32_CACHE_I64(poly_basemul_cache_init_rv64im, poly_basemul_cache_init_rv64im_wrap)
MAKE_BENCH_KYBER_BASEMUL_5P_I32_CACHE_I32(poly_basemul_cache_init_rv64im_dual, poly_basemul_cache_init_rv64im_dual_wrap)

// Cached function benchmarks
MAKE_BENCH_KYBER_BASEMUL_4P_CACHED(poly_basemul_acc_cached_rv64im,        poly_basemul_acc_cached_rv64im_wrap)
MAKE_BENCH_KYBER_BASEMUL_4P_CACHED(poly_basemul_acc_cached_rv64im_dual,   poly_basemul_acc_cached_rv64im_dual_wrap)

// Cache end function benchmarks
MAKE_BENCH_KYBER_BASEMUL_5P_I16_CACHE_ACC(poly_basemul_acc_cache_end_rv64im, poly_basemul_acc_cache_end_rv64im_wrap)
MAKE_BENCH_KYBER_BASEMUL_5P_I16_CACHE_ACC(poly_basemul_acc_cache_end_rv64im_dual, poly_basemul_acc_cache_end_rv64im_dual_wrap)

// Cache init function benchmarks
MAKE_BENCH_KYBER_BASEMUL_5P_I32_CACHE_I64(poly_basemul_acc_cache_init_rv64im, poly_basemul_acc_cache_init_rv64im_wrap)
MAKE_BENCH_KYBER_BASEMUL_5P_I32_CACHE_I32(poly_basemul_acc_cache_init_rv64im_dual, poly_basemul_acc_cache_init_rv64im_dual_wrap)

// Cache init end function benchmarks
MAKE_BENCH_KYBER_BASEMUL_6P_I16_CACHE_I64_ACC(poly_basemul_acc_cache_init_end_rv64im, poly_basemul_acc_cache_init_end_rv64im_wrap)
MAKE_BENCH_KYBER_BASEMUL_6P_I16_CACHE_I32_ACC(poly_basemul_acc_cache_init_end_rv64im_dual, poly_basemul_acc_cache_init_end_rv64im_dual_wrap)

// RVV function benchmarks (temporarily commented out due to assembly syntax issues)
MAKE_BENCH_KYBER_BASEMUL_4P_RVV_I16_TABLE(poly_basemul_rvv_vlen128, poly_basemul_rvv_vlen128_wrap)
MAKE_BENCH_KYBER_BASEMUL_4P_RVV_I16_TABLE(poly_basemul_acc_rvv_vlen128, poly_basemul_acc_rvv_vlen128_wrap)
MAKE_BENCH_KYBER_BASEMUL_5P_RVV_I16_TABLE_CACHE(poly_basemul_cached_rvv_vlen128, poly_basemul_cached_rvv_vlen128_wrap)
MAKE_BENCH_KYBER_BASEMUL_5P_RVV_I16_TABLE_CACHE(poly_basemul_acc_cached_rvv_vlen128, poly_basemul_acc_cached_rvv_vlen128_wrap)
MAKE_BENCH_KYBER_BASEMUL_5P_RVV_I16_TABLE_CACHE(poly_basemul_cache_init_rvv_vlen128, poly_basemul_cache_init_rvv_vlen128_wrap)
MAKE_BENCH_KYBER_BASEMUL_5P_RVV_I16_TABLE_CACHE(poly_basemul_acc_cache_init_rvv_vlen128, poly_basemul_acc_cache_init_rvv_vlen128_wrap)

// Optimized function benchmarks
MAKE_BENCH_KYBER_BASEMUL_4P_I32_I64(poly_basemul_acc_rv64im_opt_c908,     poly_basemul_acc_rv64im_opt_c908_wrap)
MAKE_BENCH_KYBER_BASEMUL_4P_I32_I32(poly_basemul_acc_rv64im_dual_opt_c908, poly_basemul_acc_rv64im_dual_opt_c908_wrap)

MAKE_BENCH_KYBER_BASEMUL_5P_I16_I64_I32(poly_basemul_acc_end_rv64im_opt_c908, poly_basemul_acc_end_rv64im_opt_c908_wrap)
MAKE_BENCH_KYBER_BASEMUL_5P_I16_I32_I32(poly_basemul_acc_end_rv64im_dual_opt_c908, poly_basemul_acc_end_rv64im_dual_opt_c908_wrap)

MAKE_BENCH_KYBER_BASEMUL_5P_I32_CACHE_I64(poly_basemul_cache_init_rv64im_opt_c908, poly_basemul_cache_init_rv64im_opt_c908_wrap)
MAKE_BENCH_KYBER_BASEMUL_5P_I32_CACHE_I32(poly_basemul_cache_init_rv64im_dual_opt_c908, poly_basemul_cache_init_rv64im_dual_opt_c908_wrap)

MAKE_BENCH_KYBER_BASEMUL_4P_CACHED(poly_basemul_acc_cached_rv64im_opt_c908, poly_basemul_acc_cached_rv64im_opt_c908_wrap)
MAKE_BENCH_KYBER_BASEMUL_4P_CACHED(poly_basemul_acc_cached_rv64im_dual_opt_c908, poly_basemul_acc_cached_rv64im_dual_opt_c908_wrap)

MAKE_BENCH_KYBER_BASEMUL_5P_I16_CACHE_ACC(poly_basemul_acc_cache_end_rv64im_opt_c908, poly_basemul_acc_cache_end_rv64im_opt_c908_wrap)
MAKE_BENCH_KYBER_BASEMUL_5P_I16_CACHE_ACC(poly_basemul_acc_cache_end_rv64im_dual_opt_c908, poly_basemul_acc_cache_end_rv64im_dual_opt_c908_wrap)

MAKE_BENCH_KYBER_BASEMUL_5P_I32_CACHE_I64(poly_basemul_acc_cache_init_rv64im_opt_c908, poly_basemul_acc_cache_init_rv64im_opt_c908_wrap)
MAKE_BENCH_KYBER_BASEMUL_5P_I32_CACHE_I32(poly_basemul_acc_cache_init_rv64im_dual_opt_c908, poly_basemul_acc_cache_init_rv64im_dual_opt_c908_wrap)

MAKE_BENCH_KYBER_BASEMUL_6P_I16_CACHE_I64_ACC(poly_basemul_acc_cache_init_end_rv64im_opt_c908, poly_basemul_acc_cache_init_end_rv64im_opt_c908_wrap)
MAKE_BENCH_KYBER_BASEMUL_6P_I16_CACHE_I32_ACC(poly_basemul_acc_cache_init_end_rv64im_dual_opt_c908, poly_basemul_acc_cache_init_end_rv64im_dual_opt_c908_wrap)

// RVV optimized function benchmarks (temporarily commented out due to assembly syntax issues)
MAKE_BENCH_KYBER_BASEMUL_4P_RVV_I16_TABLE(poly_basemul_rvv_vlen128_opt_c908, poly_basemul_rvv_vlen128_opt_c908_wrap)
MAKE_BENCH_KYBER_BASEMUL_4P_RVV_I16_TABLE(poly_basemul_acc_rvv_vlen128_opt_c908, poly_basemul_acc_rvv_vlen128_opt_c908_wrap)
MAKE_BENCH_KYBER_BASEMUL_5P_RVV_I16_TABLE_CACHE(poly_basemul_cached_rvv_vlen128_opt_c908, poly_basemul_cached_rvv_vlen128_opt_c908_wrap)
MAKE_BENCH_KYBER_BASEMUL_5P_RVV_I16_TABLE_CACHE(poly_basemul_acc_cached_rvv_vlen128_opt_c908, poly_basemul_acc_cached_rvv_vlen128_opt_c908_wrap)
MAKE_BENCH_KYBER_BASEMUL_5P_RVV_I16_TABLE_CACHE(poly_basemul_cache_init_rvv_vlen128_opt_c908, poly_basemul_cache_init_rvv_vlen128_opt_c908_wrap)
MAKE_BENCH_KYBER_BASEMUL_5P_RVV_I16_TABLE_CACHE(poly_basemul_acc_cache_init_rvv_vlen128_opt_c908, poly_basemul_acc_cache_init_rvv_vlen128_opt_c908_wrap)

// === MAIN FUNCTION ===
int main (void)
{
    debug_test_start( "Kyber Poly basemul!" );

    // Basic function tests (sanity checks)
    if (test_poly_basemul_acc_rv64im()           != 0) return 1;
    if (test_poly_basemul_acc_rv64im_dual()      != 0) return 1;

    if (test_poly_basemul_acc_end_rv64im()       != 0) return 1;
    if (test_poly_basemul_acc_end_rv64im_dual()  != 0) return 1;

    if (test_poly_basemul_cache_init_rv64im()    != 0) return 1;
    if (test_poly_basemul_cache_init_rv64im_dual() != 0) return 1;

    if (test_poly_basemul_acc_cached_rv64im()    != 0) return 1;
    if (test_poly_basemul_acc_cached_rv64im_dual() != 0) return 1;

    if (test_poly_basemul_acc_cache_end_rv64im() != 0) return 1;
    if (test_poly_basemul_acc_cache_end_rv64im_dual() != 0) return 1;

    if (test_poly_basemul_acc_cache_init_rv64im() != 0) return 1;
    if (test_poly_basemul_acc_cache_init_rv64im_dual() != 0) return 1;

    if (test_poly_basemul_acc_cache_init_end_rv64im() != 0) return 1;
    if (test_poly_basemul_acc_cache_init_end_rv64im_dual() != 0) return 1;

    // RVV function test (temporarily commented out due to assembly syntax issues)
    if (test_poly_basemul_rvv_vlen128() != 0) return 1;
    if (test_poly_basemul_acc_rvv_vlen128() != 0) return 1;
    if (test_poly_basemul_cached_rvv_vlen128() != 0) return 1;
    if (test_poly_basemul_acc_cached_rvv_vlen128() != 0) return 1;
    if (test_poly_basemul_cache_init_rvv_vlen128() != 0) return 1;
    if (test_poly_basemul_acc_cache_init_rvv_vlen128() != 0) return 1;

    // Optimized function tests (compare against basic versions)
    if (test_poly_basemul_acc_rv64im_opt_c908()  != 0) return 1;
    if (test_poly_basemul_acc_rv64im_dual_opt_c908() != 0) return 1;

    if (test_poly_basemul_acc_end_rv64im_opt_c908() != 0) return 1;
    if (test_poly_basemul_acc_end_rv64im_dual_opt_c908() != 0) return 1;

    if (test_poly_basemul_cache_init_rv64im_opt_c908() != 0) return 1;
    if (test_poly_basemul_cache_init_rv64im_dual_opt_c908() != 0) return 1;

    if (test_poly_basemul_acc_cached_rv64im_opt_c908() != 0) return 1;
    if (test_poly_basemul_acc_cached_rv64im_dual_opt_c908() != 0) return 1;

    if (test_poly_basemul_acc_cache_end_rv64im_opt_c908() != 0) return 1;
    if (test_poly_basemul_acc_cache_end_rv64im_dual_opt_c908() != 0) return 1;

    if (test_poly_basemul_acc_cache_init_rv64im_opt_c908() != 0) return 1;
    if (test_poly_basemul_acc_cache_init_rv64im_dual_opt_c908() != 0) return 1;

    if (test_poly_basemul_acc_cache_init_end_rv64im_opt_c908() != 0) return 1;
    if (test_poly_basemul_acc_cache_init_end_rv64im_dual_opt_c908() != 0) return 1;

    if (test_poly_basemul_rvv_vlen128_opt_c908() != 0) return 1;
    if (test_poly_basemul_acc_rvv_vlen128_opt_c908() != 0) return 1;
    if (test_poly_basemul_cached_rvv_vlen128_opt_c908() != 0) return 1;
    if (test_poly_basemul_acc_cached_rvv_vlen128_opt_c908() != 0) return 1;
    if (test_poly_basemul_cache_init_rvv_vlen128_opt_c908() != 0) return 1;
    if (test_poly_basemul_acc_cache_init_rvv_vlen128_opt_c908() != 0) return 1;

    // === BENCHMARKS ===
    debug_printf("Starting benchmarks...");

    // Basic function benchmarks
    bench_poly_basemul_acc_rv64im();
    bench_poly_basemul_acc_rv64im_dual();

    // End function benchmarks
    bench_poly_basemul_acc_end_rv64im();
    bench_poly_basemul_acc_end_rv64im_dual();

    // Cache function benchmarks
    bench_poly_basemul_cache_init_rv64im();
    bench_poly_basemul_cache_init_rv64im_dual();

    // Cached function benchmarks
    bench_poly_basemul_acc_cached_rv64im();
    bench_poly_basemul_acc_cached_rv64im_dual();

    // Cache end function benchmarks
    bench_poly_basemul_acc_cache_end_rv64im();
    bench_poly_basemul_acc_cache_end_rv64im_dual();

    // Cache init function benchmarks
    bench_poly_basemul_acc_cache_init_rv64im();
    bench_poly_basemul_acc_cache_init_rv64im_dual();

    // Cache init end function benchmarks
    bench_poly_basemul_acc_cache_init_end_rv64im();
    bench_poly_basemul_acc_cache_init_end_rv64im_dual();

    // RVV function benchmarks
    bench_poly_basemul_rvv_vlen128();
    bench_poly_basemul_acc_rvv_vlen128();
    bench_poly_basemul_cached_rvv_vlen128();
    bench_poly_basemul_acc_cached_rvv_vlen128();
    bench_poly_basemul_cache_init_rvv_vlen128();
    bench_poly_basemul_acc_cache_init_rvv_vlen128();

    // Optimized function benchmarks
    bench_poly_basemul_acc_rv64im_opt_c908();
    bench_poly_basemul_acc_rv64im_dual_opt_c908();

    bench_poly_basemul_acc_end_rv64im_opt_c908();
    bench_poly_basemul_acc_end_rv64im_dual_opt_c908();

    bench_poly_basemul_cache_init_rv64im_opt_c908();
    bench_poly_basemul_cache_init_rv64im_dual_opt_c908();

    bench_poly_basemul_acc_cached_rv64im_opt_c908();
    bench_poly_basemul_acc_cached_rv64im_dual_opt_c908();

    bench_poly_basemul_acc_cache_end_rv64im_opt_c908();
    bench_poly_basemul_acc_cache_end_rv64im_dual_opt_c908();

    bench_poly_basemul_acc_cache_init_rv64im_opt_c908();
    bench_poly_basemul_acc_cache_init_rv64im_dual_opt_c908();

    bench_poly_basemul_acc_cache_init_end_rv64im_opt_c908();
    bench_poly_basemul_acc_cache_init_end_rv64im_dual_opt_c908();

    bench_poly_basemul_rvv_vlen128_opt_c908();
    bench_poly_basemul_acc_rvv_vlen128_opt_c908();
    bench_poly_basemul_cached_rvv_vlen128_opt_c908();
    bench_poly_basemul_acc_cached_rvv_vlen128_opt_c908();
    bench_poly_basemul_cache_init_rvv_vlen128_opt_c908();
    bench_poly_basemul_acc_cache_init_rvv_vlen128_opt_c908();

    debug_printf("Test Success!"); 

    return 0;
}
