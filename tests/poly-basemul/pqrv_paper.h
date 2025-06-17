#ifndef PQRV_PAPER_H
#define PQRV_PAPER_H
#include "dilithium.h"
#include <stdint.h>
int32_t a[256];
int32_t b[256];

// single issue poly_basemul
extern void poly_basemul_8l_acc_rv64im(int32_t r[256], const int32_t a[256], const int32_t b[256]);

// optimized single issue poly_basemul
extern void poly_basemul_8l_acc_rv64im_opt_c908(int32_t r[256], const int32_t a[256], const int32_t b[256]);

// original dual issue poly_basemul
extern void poly_basemul_8l_acc_dual_rv64im(int32_t r[256], const int32_t a[256], const int32_t b[256]);

// wrapper single issue poly_basemul
void poly_basemul_8l_acc_rv64im_wrap(int32_t *r) {
    poly_basemul_8l_acc_rv64im(r, a, b);
}

// wrapper optimized single issue poly_basemul
void poly_basemul_8l_acc_rv64im_opt_wrap(int32_t *r) {
    poly_basemul_8l_acc_rv64im_opt_c908(r, a, b);
}

// wrapper original dual issue poly_basemul
void poly_basemul_8l_acc_dual_rv64im_wrap(int32_t *r) {
    poly_basemul_8l_acc_dual_rv64im(r, a, b);
}
#endif