#ifndef PQRV_PAPER_H
#define PQRV_PAPER_H
#include "kyber.h"
#include <stdint.h>

/* Zetas / shuffle-mask table consumed by the Kyber RVV NTT.
 * The forward NTT reads at offsets up to a few hundred bytes; we allocate
 * 1024 int16_t to comfortably cover the entire table layout encoded in the
 * assembly's KYBER_NTT_RVV_VLEN128_CONSTS_H header.
 * The buffer is zero-initialized as a global, which is sufficient to make
 * the (vector-)NTT produce a deterministic, repeatable output. Both the
 * baseline and the optimized assembly read from the same table, so their
 * outputs must agree. */
int16_t zetas[1024];

/* ASM functions: forward NTT for Kyber on RVV vlen=128.
 * Signature: void ntt_rvv_vlen128(int16_t r[256], const int16_t zetas[]); */
extern void ntt_rvv_vlen128(int16_t *r, const int16_t *zetas);
extern void ntt_rvv_vlen128_opt_c908(int16_t *r, const int16_t *zetas);

/* Wrappers that bind the zetas argument */
void ntt_kyber_rvv_vlen128_wrap(int16_t *r)
{
    ntt_rvv_vlen128(r, zetas);
}

void ntt_kyber_rvv_vlen128_opt_c908_wrap(int16_t *r)
{
    ntt_rvv_vlen128_opt_c908(r, zetas);
}

#endif
