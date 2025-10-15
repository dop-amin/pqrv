#ifndef BARRET_MUL_H
#define BARRET_MUL_H
#include <stdint.h>
#include <stdio.h>

extern uint32_t barret_mul(uint32_t a, uint32_t b, uint32_t n, uint32_t k, uint64_t C);
__attribute__((optimize("O0"))) void empty_dummy() {}
#endif