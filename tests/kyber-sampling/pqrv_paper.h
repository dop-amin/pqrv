#ifndef PQRV_PAPER_H
#define PQRV_PAPER_H
#include <stdint.h>

/*
 * Kyber sampling kernels (RVV VLEN=128): centered-binomial-distribution (CBD)
 * and rejection sampling. All read a shared constant table (masks / shuffle
 * indices) as their `table` argument (a2). The table layout below matches the
 * offsets baked into the assembly (see the #define _..._ offsets at the top of
 * each kyber_*_rvv_vlen128.s):
 *
 *   _MASK_45674567        0
 *   _MASK_01230123        8
 *   _MASK_01014545       16
 *   _MASK_23236767       24
 *   _MASK_10325476       32
 *   _REJ_UNIFORM_IDX8    40
 *   _REJ_UNIFORM_MASK_01 48
 *   _CBD2_MASK_E8_01     56
 *   _CBD2_IDX8_LOW       64
 *   _CBD2_IDX8_HIGH      72
 *   _CBD3_MASK_E8_0122   80
 *   _CBD3_IDX16_HIGH     88
 *   _CBD3_MASK_E16_1100  96
 *   _CBD3_IDX16_LOW     104
 *
 * Values copied verbatim from the Kyber RVV constants table (consts_vlen128 /
 * the ntt-kyber test's `zetas[]`). Correctness is checked by naive-vs-opt
 * equivalence: identical random input to the naive and _opt_c908 kernels must
 * yield identical output (SLOTHY only reschedules).
 */

#define I16(l, h) (((h) << 8) | (l))

static const int16_t kyber_sampling_consts[] __attribute__((aligned(16))) = {
    /* _MASK_45674567        (0)   */ 4, 5, 6, 7, 4, 5, 6, 7,
    /* _MASK_01230123        (8)   */ 0, 1, 2, 3, 0, 1, 2, 3,
    /* _MASK_01014545        (16)  */ 0, 1, 0, 1, 4, 5, 4, 5,
    /* _MASK_23236767        (24)  */ 2, 3, 2, 3, 6, 7, 6, 7,
    /* _MASK_10325476        (32)  */ 1, 0, 3, 2, 5, 4, 7, 6,
    /* _REJ_UNIFORM_IDX8     (40)  */ I16(0,1), I16(1,2), I16(3,4), I16(4,5), I16(6,7), I16(7,8), I16(9,10), I16(10,11),
    /* _REJ_UNIFORM_MASK_01  (48)  */ 0, 1, 0, 1, 0, 1, 0, 1,
    /* _CBD2_MASK_E8_01      (56)  */ I16(0,1), I16(0,1), I16(0,1), I16(0,1), I16(0,1), I16(0,1), I16(0,1), I16(0,1),
    /* _CBD2_IDX8_LOW        (64)  */ I16(0,0), I16(1,1), I16(2,2), I16(3,3), I16(4,4), I16(5,5), I16(6,6), I16(7,7),
    /* _CBD2_IDX8_HIGH       (72)  */ I16(8,8), I16(9,9), I16(10,10), I16(11,11), I16(12,12), I16(13,13), I16(14,14), I16(15,15),
    /* _CBD3_MASK_E8_0122    (80)  */ I16(0,1), I16(2,2), I16(3,4), I16(5,5), I16(6,7), I16(8,8), I16(9,10), I16(11,11),
    /* _CBD3_IDX16_HIGH      (88)  */ 4, 5, 4, 5, 6, 7, 6, 7,
    /* _CBD3_MASK_E16_1100   (96)  */ 1, 1, 0, 0, 1, 1, 0, 0,
    /* _CBD3_IDX16_LOW       (104) */ 0, 1, 0, 1, 2, 3, 2, 3,
    /* padding                     */ 0, 0, 0, 0, 0, 0, 0, 0,
};

/* ===== Assembly kernels (naive + SLOTHY-optimized) ===== */
extern void cbd2_rvv_vlen128(int16_t *r, const uint8_t *buf, const int16_t *table);
extern void cbd2_rvv_vlen128_opt_c908(int16_t *r, const uint8_t *buf, const int16_t *table);
extern void cbd3_rvv_vlen128(int16_t *r, const uint8_t *buf, const int16_t *table);
extern void cbd3_rvv_vlen128_opt_c908(int16_t *r, const uint8_t *buf, const int16_t *table);
extern void rej_uniform_rvv_vlen128(int16_t *r, const uint8_t *buf, const int16_t *table,
                                    uint32_t *ctr_p, uint32_t *pos_p);
extern void rej_uniform_rvv_vlen128_opt_c908(int16_t *r, const uint8_t *buf, const int16_t *table,
                                             uint32_t *ctr_p, uint32_t *pos_p);

/* ===== Wrappers binding the shared constant table ===== */
void cbd2_rvv_vlen128_wrap(int16_t *r, const uint8_t *buf) { cbd2_rvv_vlen128(r, buf, kyber_sampling_consts); }
void cbd2_rvv_vlen128_opt_c908_wrap(int16_t *r, const uint8_t *buf) { cbd2_rvv_vlen128_opt_c908(r, buf, kyber_sampling_consts); }
void cbd3_rvv_vlen128_wrap(int16_t *r, const uint8_t *buf) { cbd3_rvv_vlen128(r, buf, kyber_sampling_consts); }
void cbd3_rvv_vlen128_opt_c908_wrap(int16_t *r, const uint8_t *buf) { cbd3_rvv_vlen128_opt_c908(r, buf, kyber_sampling_consts); }

void rej_uniform_rvv_vlen128_wrap(int16_t *r, const uint8_t *buf, uint32_t *ctr_p, uint32_t *pos_p)
{ rej_uniform_rvv_vlen128(r, buf, kyber_sampling_consts, ctr_p, pos_p); }
void rej_uniform_rvv_vlen128_opt_c908_wrap(int16_t *r, const uint8_t *buf, uint32_t *ctr_p, uint32_t *pos_p)
{ rej_uniform_rvv_vlen128_opt_c908(r, buf, kyber_sampling_consts, ctr_p, pos_p); }

#endif // PQRV_PAPER_H
