#ifndef PQRV_PAPER_H
#define PQRV_PAPER_H
#include "dilithium.h"
#include <stdint.h>

// single issue poly_basemul
extern void poly_basemul_8l_acc_rv64im(int64_t r[256], const int32_t a[256], const int32_t b[256]);
extern void poly_basemul_8l_acc_end_rv64im(int32_t r[256], const int32_t a[256], const int32_t b[256], int64_t r_double[256]);
extern void poly_basemul_8l_init_rv64im(int64_t r[256], const int32_t a[256], const int32_t b[256]);
extern void poly_basemul_8l_rv64im(int32_t r[256], const int32_t a[256], const int32_t b[256]);

// dual issue poly_basemul
extern void poly_basemul_8l_acc_rv64im_dual(int64_t r[256], const int32_t a[256], const int32_t b[256]);
extern void poly_basemul_8l_acc_end_rv64im_dual(int32_t r[256], const int32_t a[256], const int32_t b[256], int64_t r_double[256]);
extern void poly_basemul_8l_init_rv64im_dual(int64_t r[256], const int32_t a[256], const int32_t b[256]);
extern void poly_basemul_8l_rv64im_dual(int32_t r[256], const int32_t a[256], const int32_t b[256]);

// optimized single issue poly_basemul
extern void poly_basemul_8l_acc_rv64im_opt_c908(int64_t r[256], const int32_t a[256], const int32_t b[256]);
extern void poly_basemul_8l_acc_end_rv64im_opt_c908(int32_t r[256], const int32_t a[256], const int32_t b[256], int64_t r_double[256]);
extern void poly_basemul_8l_init_rv64im_opt_c908(int64_t r[256], const int32_t a[256], const int32_t b[256]);
extern void poly_basemul_8l_rv64im_opt_c908(int32_t r[256], const int32_t a[256], const int32_t b[256]);

// optimized dual issue poly_basemul
extern void poly_basemul_8l_acc_rv64im_dual_opt_c908(int64_t r[256], const int32_t a[256], const int32_t b[256]);
extern void poly_basemul_8l_acc_end_rv64im_dual_opt_c908(int32_t r[256], const int32_t a[256], const int32_t b[256], int64_t r_double[256]);
extern void poly_basemul_8l_init_rv64im_dual_opt_c908(int64_t r[256], const int32_t a[256], const int32_t b[256]);
extern void poly_basemul_8l_rv64im_dual_opt_c908(int32_t r[256], const int32_t a[256], const int32_t b[256]);

// ===== Wrapper functions =====

// single issue wrappers - 3 parameters (int64_t r, int32_t a, int32_t b)
void poly_basemul_8l_acc_rv64im_wrap(int64_t *r, const int32_t *a, const int32_t *b) {
    poly_basemul_8l_acc_rv64im(r, a, b);
}
void poly_basemul_8l_init_rv64im_wrap(int64_t *r, const int32_t *a, const int32_t *b) {
    poly_basemul_8l_init_rv64im(r, a, b);
}

// single issue wrappers - 3 parameters (int32_t r, int32_t a, int32_t b)
void poly_basemul_8l_rv64im_wrap(int32_t *r, const int32_t *a, const int32_t *b) {
    poly_basemul_8l_rv64im(r, a, b);
}

// single issue wrappers - 4 parameters (int32_t r, int32_t a, int32_t b, int64_t r_double)
void poly_basemul_8l_acc_end_rv64im_wrap(int32_t *r, const int32_t *a, const int32_t *b, int64_t *r_double) {
    poly_basemul_8l_acc_end_rv64im(r, a, b, r_double);
}

// dual issue wrappers - 3 parameters (int64_t r, int32_t a, int32_t b)
void poly_basemul_8l_acc_rv64im_dual_wrap(int64_t *r, const int32_t *a, const int32_t *b) {
    poly_basemul_8l_acc_rv64im_dual(r, a, b);
}
void poly_basemul_8l_init_rv64im_dual_wrap(int64_t *r, const int32_t *a, const int32_t *b) {
    poly_basemul_8l_init_rv64im_dual(r, a, b);
}

// dual issue wrappers - 3 parameters (int32_t r, int32_t a, int32_t b)
void poly_basemul_8l_rv64im_dual_wrap(int32_t *r, const int32_t *a, const int32_t *b) {
    poly_basemul_8l_rv64im_dual(r, a, b);
}

// dual issue wrappers - 4 parameters (int32_t r, int32_t a, int32_t b, int64_t r_double)
void poly_basemul_8l_acc_end_rv64im_dual_wrap(int32_t *r, const int32_t *a, const int32_t *b, int64_t *r_double) {
    poly_basemul_8l_acc_end_rv64im_dual(r, a, b, r_double);
}

// optimized single issue wrappers - 3 parameters (int64_t r, int32_t a, int32_t b)
void poly_basemul_8l_acc_rv64im_opt_c908_wrap(int64_t *r, const int32_t *a, const int32_t *b) {
    poly_basemul_8l_acc_rv64im_opt_c908(r, a, b);
}
void poly_basemul_8l_init_rv64im_opt_c908_wrap(int64_t *r, const int32_t *a, const int32_t *b) {
    poly_basemul_8l_init_rv64im_opt_c908(r, a, b);
}

// optimized single issue wrappers - 3 parameters (int32_t r, int32_t a, int32_t b)
void poly_basemul_8l_rv64im_opt_c908_wrap(int32_t *r, const int32_t *a, const int32_t *b) {
    poly_basemul_8l_rv64im_opt_c908(r, a, b);
}

// optimized single issue wrappers - 4 parameters (int32_t r, int32_t a, int32_t b, int64_t r_double)
void poly_basemul_8l_acc_end_rv64im_opt_c908_wrap(int32_t *r, const int32_t *a, const int32_t *b, int64_t *r_double) {
    poly_basemul_8l_acc_end_rv64im_opt_c908(r, a, b, r_double);
}

// optimized dual issue wrappers - 3 parameters (int64_t r, int32_t a, int32_t b)
void poly_basemul_8l_acc_rv64im_opt_c908_dual_wrap(int64_t *r, const int32_t *a, const int32_t *b) {
    poly_basemul_8l_acc_rv64im_dual_opt_c908(r, a, b);
}
void poly_basemul_8l_init_rv64im_opt_c908_dual_wrap(int64_t *r, const int32_t *a, const int32_t *b) {
    poly_basemul_8l_init_rv64im_dual_opt_c908(r, a, b);
}

// optimized dual issue wrappers - 3 parameters (int32_t r, int32_t a, int32_t b)
void poly_basemul_8l_rv64im_opt_c908_dual_wrap(int32_t *r, const int32_t *a, const int32_t *b) {
    poly_basemul_8l_rv64im_dual_opt_c908(r, a, b);
}

// optimized dual issue wrappers - 4 parameters (int32_t r, int32_t a, int32_t b, int64_t r_double)
void poly_basemul_8l_acc_end_rv64im_opt_c908_dual_wrap(int32_t *r, const int32_t *a, const int32_t *b, int64_t *r_double) {
    poly_basemul_8l_acc_end_rv64im_dual_opt_c908(r, a, b, r_double);
}

#endif // PQRV_PAPER_H