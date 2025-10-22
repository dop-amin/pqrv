#include "barret-mul-ref.h"
#include <stdlib.h>
#include <time.h>

#define NTEST 10000

int test_ref() {
    uint64_t n = rand();
    uint64_t a = rand();
    uint64_t b = rand() % n;
    int64_t C = calc_const(b, K, n);
    int64_t result = barret_mul(a, b, n, K, C);
    //printf("%lu * %lu mod %lu = %lu\n", a, b, n, barret_mul(a, b, n, K, C));
    if(result != (a*b % n)) {
        return 1;
    } else{
        return 0;
    }
}
int main () {
    srand(time(NULL));
    for(int i=0; i < NTEST; i++) {
        if(test_ref() != 0) {
            printf("ERROR");
        }
    }
}