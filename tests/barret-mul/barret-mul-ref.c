#include "barret-mul-ref.h"

uint64_t barret_mul_ref(uint64_t a, uint64_t b, uint64_t n, uint64_t k, uint64_t C) {
    uint64_t z = a*b;
    uint64_t t = ((unsigned __int128)a * C) >> k;  // = multiply high unsigned
    uint64_t result = z-n*t;
    return result;
}

void print_u128_hex(unsigned __int128 x) {
    uint64_t hi = (uint64_t)(x >> 64);
    uint64_t lo = (uint64_t)x;
    if (hi)
        printf("temp = 0x%lx%016lx \n", hi, lo);
    else
        printf("temp = 0x%lx \n", lo);
}

uint64_t calc_const(uint64_t b, uint32_t k, uint64_t n) {
    unsigned __int128 tmp = (unsigned __int128)b << k;
    uint64_t C = (uint64_t)(tmp / n);
    return C;
}