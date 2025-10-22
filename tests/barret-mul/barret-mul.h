#ifndef BARRET_MUL_H
#define BARRET_MUL_H
#include <stdint.h>
#include <stdio.h>

extern uint64_t barret_mul(uint64_t a, uint64_t b, uint64_t n, uint64_t k, uint64_t C);
__attribute__((optimize("O0"))) void empty_dummy() {}
#endif