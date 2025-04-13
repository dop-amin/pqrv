#include <hal.h>
#include "profiling.h"
#include "prefix_ubenchs.h"
#include <stdlib.h>
#include <string.h>

unsigned int measure(ubench_t func)
{
    uint32_t data0[2048] __attribute__((aligned(16)));
    uint32_t data1[2048] __attribute__((aligned(16)));
    uint32_t data2[2048] __attribute__((aligned(16)));
    uint32_t data3[2048] __attribute__((aligned(16)));
    uint32_t data4[2048] __attribute__((aligned(16)));
    for( unsigned cnt=0; cnt < WARMUP_ITERATIONS; cnt++ )
        func ( data0 + 1024, data1 + 1024,
               data2 + 1024, data3 + 1024,
               data4 + 1024 );
    init_perf_events();
    start_counting_events();
    for( unsigned cnt=0; cnt < TEST_COUNT; cnt++ )
    {
        for( unsigned cntp=0; cntp < ITER_PER_TEST; cntp++ )
        {

            func( data0 + 1024, data1 + 1024,
                  data2 + 1024, data3 + 1024,
                  data4 + 1024 );

        }
    }
    stop_and_read_events();
    calc_average(ITER_PER_TEST, UBENCH_PATTERN_REPEATS);
    cleanup_perf_events();
    return get_total_cycles();
}

void print_line_with_star( unsigned len, unsigned pos )
{

}

void profile_full()
{
    const unsigned maxchars = 80;
    char tmp[maxchars + 1];

    debug_printf( "===== Stepwise profiling =======\n");
    for( unsigned int i=0; i < num_ubenchs_prefix; i++ )
    {
        int median = measure(ubenchs_prefix[i]);
        debug_printf("%i", median);
        median = median % maxchars;
        memset( tmp, '.', maxchars );
        tmp[median] = '*';
        tmp[maxchars] = '\0';
        debug_printf( "[%3u]: %60s %s\n", i, ubench_prefix_instructions[i], tmp );
    }
}
