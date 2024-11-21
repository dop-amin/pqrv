/*
 * Author: Hanno Becker <hannobecker@posteo.de>
 */

/*
 * Some external references to auto-generated assembly.
 */
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "profiling.h"
#include <hal.h>

int main( void )
{
    debug_printf( "=========== uArch profiling ===============\n" );

    debug_printf( "- Enable cycle counter ..." );
    enable_cyclecounter();
    debug_printf( "ok\n" );

    profile_full();

    debug_printf( "- Disable cycle counter ..." );
    disable_cyclecounter();
    debug_printf( "ok\n" );

    debug_printf( "\nDone!\n" );
    return(0);
}
