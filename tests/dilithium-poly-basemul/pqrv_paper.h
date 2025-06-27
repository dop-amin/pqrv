#ifndef PQRV_PAPER_H
#define PQRV_PAPER_H
#include "dilithium.h"
#include <stdint.h>
int32_t a[256];
int32_t b[256];
int64_t dbl[256];

// single issue poly_basemul
extern void poly_basemul_8l_acc_rv64im(int32_t r[256], const int32_t a[256], const int32_t b[256]);
extern void poly_basemul_8l_acc_end_rv64im(int32_t r[256], const int32_t a[256], const int32_t b[256], int64_t r_double[256]);
extern void poly_basemul_8l_init_rv64im(int32_t r[256], const int32_t a[256], const int32_t b[256]);
extern void poly_basemul_8l_rv64im(int32_t r[256], const int32_t a[256], const int32_t b[256]);

// dual issue poly_basemul
extern void poly_basemul_8l_acc_rv64im_dual(int32_t r[256], const int32_t a[256], const int32_t b[256]);
extern void poly_basemul_8l_acc_end_rv64im_dual(int32_t r[256], const int32_t a[256], const int32_t b[256]);
extern void poly_basemul_8l_init_rv64im_dual(int32_t r[256], const int32_t a[256], const int32_t b[256]);
extern void poly_basemul_8l_rv64im_dual(int32_t r[256], const int32_t a[256], const int32_t b[256]);

// optimized single issue poly_basemul
extern void poly_basemul_8l_acc_rv64im_opt_c908(int32_t r[256], const int32_t a[256], const int32_t b[256]);
extern void poly_basemul_8l_acc_end_rv64im_opt_c908(int32_t r[256], const int32_t a[256], const int32_t b[256]);
extern void poly_basemul_8l_init_rv64im_opt_c908(int32_t r[256], const int32_t a[256], const int32_t b[256]);
extern void poly_basemul_8l_rv64im_opt_c908(int32_t r[256], const int32_t a[256], const int32_t b[256]);

// optimized dual issue poly_basemul
extern void poly_basemul_8l_acc_rv64im_opt_c908_dual(int32_t r[256], const int32_t a[256], const int32_t b[256]);
extern void poly_basemul_8l_acc_end_rv64im_opt_c908_dual(int32_t r[256], const int32_t a[256], const int32_t b[256]);
extern void poly_basemul_8l_init_rv64im_opt_c908_dual(int32_t r[256], const int32_t a[256], const int32_t b[256]);
extern void poly_basemul_8l_rv64im_opt_c908_dual(int32_t r[256], const int32_t a[256], const int32_t b[256]);

// ===== Wrapper functions =====

// single issue wrappers
void poly_basemul_8l_acc_rv64im_wrap(int32_t *r) {
    poly_basemul_8l_acc_rv64im(r, a, b);
}
void poly_basemul_8l_acc_end_rv64im_wrap(int32_t *r) {
    poly_basemul_8l_acc_end_rv64im(r, a, b, dbl);
}
void poly_basemul_8l_init_rv64im_wrap(int32_t *r) {
    poly_basemul_8l_init_rv64im(r, a, b);
}
void poly_basemul_8l_rv64im_wrap(int32_t *r) {
    poly_basemul_8l_rv64im(r, a, b);
}

// dual issue wrappers
void poly_basemul_8l_acc_rv64im_dual_wrap(int32_t *r) {
    poly_basemul_8l_acc_rv64im_dual(r, a, b);
}
void poly_basemul_8l_acc_end_rv64im_dual_wrap(int32_t *r) {
    poly_basemul_8l_acc_end_rv64im_dual(r, a, b);
}
void poly_basemul_8l_init_rv64im_dual_wrap(int32_t *r) {
    poly_basemul_8l_init_rv64im_dual(r, a, b);
}
void poly_basemul_8l_rv64im_dual_wrap(int32_t *r) {
    poly_basemul_8l_rv64im_dual(r, a, b);
}

// optimized single issue wrappers
void poly_basemul_8l_acc_rv64im_opt_c908_wrap(int32_t *r) {
    poly_basemul_8l_acc_rv64im_opt_c908(r, a, b);
}
void poly_basemul_8l_acc_end_rv64im_opt_c908_wrap(int32_t *r) {
    poly_basemul_8l_acc_end_rv64im_opt_c908(r, a, b);
}
void poly_basemul_8l_init_rv64im_opt_c908_wrap(int32_t *r) {
    poly_basemul_8l_init_rv64im_opt_c908(r, a, b);
}
void poly_basemul_8l_rv64im_opt_c908_wrap(int32_t *r) {
    poly_basemul_8l_rv64im_opt_c908(r, a, b);
}

// optimized dual issue wrappers
void poly_basemul_8l_acc_rv64im_opt_c908_dual_wrap(int32_t *r) {
    poly_basemul_8l_acc_rv64im_opt_c908_dual(r, a, b);
}
void poly_basemul_8l_acc_end_rv64im_opt_c908_dual_wrap(int32_t *r) {
    poly_basemul_8l_acc_end_rv64im_opt_c908_dual(r, a, b);
}
void poly_basemul_8l_init_rv64im_opt_c908_dual_wrap(int32_t *r) {
    poly_basemul_8l_init_rv64im_opt_c908_dual(r, a, b);
}
void poly_basemul_8l_rv64im_opt_c908_dual_wrap(int32_t *r) {
    poly_basemul_8l_rv64im_opt_c908_dual(r, a, b);
}

#endif // PQRV_PAPER_H