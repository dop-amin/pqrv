#ifndef BARRET_MUL_H
#define BARRET_MUL_H
#include <stdint.h>
#include <stdio.h>

extern uint64_t barret_mul_scalar(uint64_t a, uint64_t b, uint64_t n, uint64_t k, uint64_t c);
extern void barret_mul_vect_naive(void* a, void* b, uint64_t n, void* c, void* result);
extern void barret_mul_vect_opt(void* a, void* b, uint64_t n, void* c, void* result);
extern void barret_mul_vect_bench();
extern void mont_mul_vect_bench();
__attribute__((optimize("O0"))) void empty_dummy() {}
#endif