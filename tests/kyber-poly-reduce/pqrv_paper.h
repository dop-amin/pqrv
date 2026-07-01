#ifndef PQRV_PAPER_H
#define PQRV_PAPER_H
#include <stdint.h>

/*
 * Kyber polynomial reduction / domain-conversion helpers.
 *
 * All functions share the same signature: they operate in-place on a
 * polynomial of 256 signed 16-bit coefficients:
 *
 *     void f(int16_t a[256]);
 *
 * a0: pointer to the coefficient array (input and output, in-place).
 *
 * The SLOTHY-optimized variants (`_opt_c908`) are semantically identical to
 * the corresponding naive versions (SLOTHY only reschedules instructions),
 * so correctness is checked by feeding identical random input to the naive
 * and optimized routines and comparing the results.
 */

#define KYBER_N 256

/* ===== RV64IM: Plantard reduction ===== */
extern void poly_plantard_rdc_rv64im(int16_t a[KYBER_N]);
extern void poly_plantard_rdc_rv64im_dual(int16_t a[KYBER_N]);
extern void poly_plantard_rdc_rv64im_opt_c908(int16_t a[KYBER_N]);
extern void poly_plantard_rdc_rv64im_dual_opt_c908(int16_t a[KYBER_N]);

/* ===== RV64IM: to-Plantard domain conversion ===== */
extern void poly_toplant_rv64im(int16_t a[KYBER_N]);
extern void poly_toplant_rv64im_dual(int16_t a[KYBER_N]);
extern void poly_toplant_rv64im_opt_c908(int16_t a[KYBER_N]);
extern void poly_toplant_rv64im_dual_opt_c908(int16_t a[KYBER_N]);

/* ===== RVV VLEN=128: Barrett reduction ===== */
extern void poly_reduce_rvv_vlen128(int16_t a[KYBER_N]);
extern void poly_reduce_rvv_vlen128_opt_c908(int16_t a[KYBER_N]);

/* ===== RVV VLEN=128: to-Montgomery domain conversion ===== */
extern void poly_tomont_rvv_vlen128(int16_t a[KYBER_N]);
extern void poly_tomont_rvv_vlen128_opt_c908(int16_t a[KYBER_N]);

/* ===== Wrapper functions ===== */

/* Plantard reduction */
void poly_plantard_rdc_rv64im_wrap(int16_t *a) { poly_plantard_rdc_rv64im(a); }
void poly_plantard_rdc_rv64im_dual_wrap(int16_t *a) { poly_plantard_rdc_rv64im_dual(a); }
void poly_plantard_rdc_rv64im_opt_c908_wrap(int16_t *a) { poly_plantard_rdc_rv64im_opt_c908(a); }
void poly_plantard_rdc_rv64im_dual_opt_c908_wrap(int16_t *a) { poly_plantard_rdc_rv64im_dual_opt_c908(a); }

/* to-Plantard */
void poly_toplant_rv64im_wrap(int16_t *a) { poly_toplant_rv64im(a); }
void poly_toplant_rv64im_dual_wrap(int16_t *a) { poly_toplant_rv64im_dual(a); }
void poly_toplant_rv64im_opt_c908_wrap(int16_t *a) { poly_toplant_rv64im_opt_c908(a); }
void poly_toplant_rv64im_dual_opt_c908_wrap(int16_t *a) { poly_toplant_rv64im_dual_opt_c908(a); }

#ifdef VECTOR128
/* Barrett reduction (RVV) */
void poly_reduce_rvv_vlen128_wrap(int16_t *a) { poly_reduce_rvv_vlen128(a); }
void poly_reduce_rvv_vlen128_opt_c908_wrap(int16_t *a) { poly_reduce_rvv_vlen128_opt_c908(a); }

/* to-Montgomery (RVV) */
void poly_tomont_rvv_vlen128_wrap(int16_t *a) { poly_tomont_rvv_vlen128(a); }
void poly_tomont_rvv_vlen128_opt_c908_wrap(int16_t *a) { poly_tomont_rvv_vlen128_opt_c908(a); }
#endif /* VECTOR128 */

#endif // PQRV_PAPER_H
