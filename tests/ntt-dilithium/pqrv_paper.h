#ifndef PQRV_PAPER_H
#define PQRV_PAPER_H
#include "dilithium.h"
#include <stdint.h>
int64_t zetas[DILITHIUM_N * 2];

// ASM funcs
extern void ntt_8l_rv64im(int32_t *r, const int64_t *zetas);
extern void ntt_8l_dual_rv64im(int32_t *r, const int64_t *zetas);

// opt
extern void ntt_8l_rv64im_opt_c908(int32_t *r, const int64_t *zetas);

// Wrappers
void ntt_8l_rv64im_wrap(int32_t *r)
{
    ntt_8l_rv64im(r, zetas);
}

void ntt_8l_dual_rv64im_wrap(int32_t *r)
{
    ntt_8l_dual_rv64im(r, zetas);
}

// opt
void ntt_8l_rv64im_opt_wrap(int32_t *r) {
    ntt_8l_rv64im_opt_c908(r, zetas);
}
#endif