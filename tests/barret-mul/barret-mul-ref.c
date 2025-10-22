#include "barret-mul-ref.h"

void print_u128_hex(unsigned __int128 x) {
    uint64_t hi = (uint64_t)(x >> 64);
    uint64_t lo = (uint64_t)x;
    if (hi)
        printf("temp = 0x%lx%016lx \n", hi, lo);
    else
        printf("temp = 0x%lx \n", lo);
}

uint64_t barret_mul_scalar_ref(uint64_t a, uint64_t b, uint64_t n, uint64_t k, uint64_t c) {
    uint64_t z = a*b;
    uint64_t t = ((unsigned __int128)a * c) >> k;  // = multiply high unsigned
    uint64_t result = z-n*t;
    return result;
}

void barret_mul_vect_ref(uint64_t* a, uint64_t* b, uint64_t n, uint64_t* c, uint32_t len, uint64_t* result) {
    for(unsigned int i = 0; i < len; i++) {
        result[i] = barret_mul_scalar_ref(a[i], b[i], n, K, c[i]);
    }
}
uint64_t calc_const(uint64_t b, uint32_t k, uint64_t n) {
    unsigned __int128 tmp = (unsigned __int128)b << k;
    uint64_t c = (uint64_t)(tmp / n);
    return c;
}

void calc_const_vect(uint64_t* b, uint32_t k, uint64_t n, uint64_t* c, uint32_t len) {
    for(unsigned int i = 0; i < len; i++) {
        c[i] = calc_const(b[i], k, n);
    }
}