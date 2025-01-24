#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <hal.h>

#define WARMUP_ITERATIONS 100
#define ITER_PER_TEST     100
#define TEST_COUNT        100

#define UBENCH_PATTERN_REPEATS 1

extern void empty_func(void*,void*,void*,void*,void*);
extern void asm_func(void*,void*,void*,void*,void*);

static int cmp_uint64_t(const void *a, const void *b)
{
    return (int)((*((const uint64_t *)a)) - (*((const uint64_t *)b)));
}

unsigned int measure(void func(void*,void*,void*,void*,void*), size_t pattern_repeats)
{
    uint64_t t0, t1;
    uint64_t cycles[TEST_COUNT];
    unsigned char foo[1000],foo1[1000],foo2[1000],foo3[1000],foo4[1000];
    for( unsigned cnt=0; cnt < WARMUP_ITERATIONS; cnt++ )
        func(foo,foo1,foo2,foo3,foo4);
    for( unsigned cnt=0; cnt < TEST_COUNT; cnt++ )
    {
        t0 = get_cyclecounter();
        for( unsigned cntp=0; cntp < ITER_PER_TEST; cntp++ )
        {
            func(foo,foo1,foo2,foo3,foo4);
        }
        t1 = get_cyclecounter();
        cycles[cnt] = (t1 - t0) / ITER_PER_TEST;
    }

    /* Report median */
    qsort( cycles, TEST_COUNT, sizeof(uint64_t), cmp_uint64_t );
    return (unsigned int)((cycles[TEST_COUNT >> 1]) /
                          pattern_repeats );
}

int main(void)
{
    debug_test_start( "Easy Bench" );
    int empty = measure(empty_func, UBENCH_PATTERN_REPEATS);
    int c = measure(asm_func, UBENCH_PATTERN_REPEATS);
    debug_printf("Cycles: %d\n", c);
    debug_printf("Cycles empty: %d\n", empty);
    debug_printf("Cycles final: %d\n", c - empty);
    debug_printf("ALL GOOD!\n");

    return 0;
}