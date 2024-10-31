/*
 * Copyright (c) 2022 Arm Limited
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
#include "cpucycles.h"
/* Dependency on standard library:
 * - rand(), srand()
 * - time()
 * - printf()
 * - fflush()
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#define FILENO stderr

void rand_init( unsigned long seed )
{
    ((void) seed);
    srand(time(NULL));
}

uint8_t get_random_byte()
{
    return( rand() );
}

/* Debugging stubs */

void debug_test_start( const char *testname )
{
    fprintf( FILENO, "%s ... ", testname );
    fflush( FILENO );
}

void debug_printf(const char * format, ... )
{
    va_list argp;
    va_start( argp, format );
    vfprintf( FILENO, format, argp );
    va_end( argp );
}

void debug_test_ok()   { printf( "Ok\n"    ); }
void debug_test_fail() { printf( "FAIL!\n" ); }


#if !defined(EXTERNAL_CYCLES) && !defined(PERF_CYCLES) && !defined(PMU_CYCLES) && !defined(NO_CYCLES)
#define NO_CYCLES
#endif

#if !defined(NO_CYCLES)
uint64_t cpucycles_overhead(void)
{
    uint64_t t0, t1, overhead = -1LL;
    unsigned int i;

    for (i = 0; i < 100000; i++) {
        t0 = cpucycles();
        __asm__ volatile("");
        t1 = cpucycles();
        if (t1 - t0 < overhead)
            overhead = t1 - t0;
    }

    return overhead;
}

void enable_cyclecounter() {
    return;
}
void disable_cyclecounter() {
    return;
}

uint64_t get_cyclecounter() {
    return cpucycles();
}

#else /* NO_CYCLES */

void enable_cyclecounter() {
    return;
}
void disable_cyclecounter() {
    return;
}
uint64_t get_cyclecounter() {
    return(0);
}

#endif /* NO_CYCLES */
