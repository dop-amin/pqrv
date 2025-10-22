#include <stdint.h>
#include <stdio.h>
#include <math.h>

#define A 179
#define B 666  // must be smaller than N
#define N 3329
#define K 64

uint64_t barret_mul_scalar_ref(uint64_t a, uint64_t b, uint64_t n, uint64_t k, uint64_t c);
void barret_mul_vect_ref(uint64_t* a, uint64_t* b, uint64_t n, uint64_t* c, uint32_t len, uint64_t* result);
uint64_t calc_const(uint64_t b, uint32_t k, uint64_t n);
void calc_const_vect(uint64_t* b, uint32_t k, uint64_t n, uint64_t* c, uint32_t len);
