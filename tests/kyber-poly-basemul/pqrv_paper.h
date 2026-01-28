#ifndef PQRV_PAPER_H
#define PQRV_PAPER_H
#include <stdint.h>

// Function signatures based on assembly analysis:
// a0: result array (int32_t or int16_t depending on function)
// a1: input array a (int16_t)
// a2: input array b (int16_t)  
// a3: cache/zeta array (varies by function)
// a4: additional parameter for some functions

// Basic accumulation functions - 4 parameters
// poly_basemul_acc_rv64im(int32_t r[256], int16_t a[256], int16_t b[256], int64_t zeta[128])
extern void poly_basemul_acc_rv64im(int32_t r[256], const int16_t a[256], const int16_t b[256], const int64_t zeta[128]);
extern void poly_basemul_acc_rv64im_dual(int32_t r[256], const int16_t a[256], const int16_t b[256], const int32_t zeta[128]);

// End accumulation functions - 5 parameters  
// poly_basemul_acc_end_rv64im(int16_t r[256], int16_t a[256], int16_t b[256], int64_t zeta[128], int32_t acc[256])
extern void poly_basemul_acc_end_rv64im(int16_t r[256], const int16_t a[256], const int16_t b[256], const int64_t zeta[128], const int32_t acc[256]);
extern void poly_basemul_acc_end_rv64im_dual(int16_t r[256], const int16_t a[256], const int16_t b[256], const int32_t zeta[128], const int32_t acc[256]);

// Cache initialization functions - 5 parameters
// poly_basemul_cache_init_rv64im(int32_t r[256], int16_t a[256], int16_t b[256], int16_t cache[256], int64_t zeta[128])
extern void poly_basemul_cache_init_rv64im(int32_t r[256], const int16_t a[256], const int16_t b[256], int16_t cache[256], const int64_t zeta[128]);
extern void poly_basemul_cache_init_rv64im_dual(int32_t r[256], const int16_t a[256], const int16_t b[256], int16_t cache[256], const int32_t zeta[128]);

// Cached accumulation functions - 4 parameters
// poly_basemul_acc_cached_rv64im(int32_t r[256], int16_t a[256], int16_t b[256], int16_t cache[256])
extern void poly_basemul_acc_cached_rv64im(int32_t r[256], const int16_t a[256], const int16_t b[256], const int16_t cache[256]);
extern void poly_basemul_acc_cached_rv64im_dual(int32_t r[256], const int16_t a[256], const int16_t b[256], const int16_t cache[256]);

// Cache end functions - 5 parameters
// poly_basemul_acc_cache_end_rv64im(int16_t r[256], int16_t a[256], int16_t b[256], int16_t cache[256], int32_t acc[256])
extern void poly_basemul_acc_cache_end_rv64im(int16_t r[256], const int16_t a[256], const int16_t b[256], const int16_t cache[256], const int32_t acc[256]);
extern void poly_basemul_acc_cache_end_rv64im_dual(int16_t r[256], const int16_t a[256], const int16_t b[256], const int16_t cache[256], const int32_t acc[256]);

// Cache init functions - 5 parameters
// poly_basemul_acc_cache_init_rv64im(int32_t r[256], int16_t a[256], int16_t b[256], int16_t cache[256], int64_t zeta[128])
extern void poly_basemul_acc_cache_init_rv64im(int32_t r[256], const int16_t a[256], const int16_t b[256], int16_t cache[256], const int64_t zeta[128]);
extern void poly_basemul_acc_cache_init_rv64im_dual(int32_t r[256], const int16_t a[256], const int16_t b[256], int16_t cache[256], const int32_t zeta[128]);

// Cache init end functions - 6 parameters
// poly_basemul_acc_cache_init_end_rv64im(int16_t r[256], int16_t a[256], int16_t b[256], int16_t cache[256], int64_t zeta[128], int32_t acc[256])
extern void poly_basemul_acc_cache_init_end_rv64im(int16_t r[256], const int16_t a[256], const int16_t b[256], int16_t cache[256], const int64_t zeta[128], const int32_t acc[256]);
extern void poly_basemul_acc_cache_init_end_rv64im_dual(int16_t r[256], const int16_t a[256], const int16_t b[256], int16_t cache[256], const int32_t zeta[128], const int32_t acc[256]);

// ===== OPTIMIZED FUNCTIONS =====

// Basic optimized functions - 4 parameters
extern void poly_basemul_acc_rv64im_opt_c908(int32_t r[256], const int16_t a[256], const int16_t b[256], const int64_t zeta[128]);
extern void poly_basemul_acc_rv64im_dual_opt_c908(int32_t r[256], const int16_t a[256], const int16_t b[256], const int32_t zeta[128]);

// End optimized functions - 5 parameters  
extern void poly_basemul_acc_end_rv64im_opt_c908(int16_t r[256], const int16_t a[256], const int16_t b[256], const int64_t zeta[128], const int32_t acc[256]);
extern void poly_basemul_acc_end_rv64im_dual_opt_c908(int16_t r[256], const int16_t a[256], const int16_t b[256], const int32_t zeta[128], const int32_t acc[256]);

// Cache optimized functions - 5 parameters
extern void poly_basemul_cache_init_rv64im_opt_c908(int32_t r[256], const int16_t a[256], const int16_t b[256], int16_t cache[256], const int64_t zeta[128]);
extern void poly_basemul_cache_init_rv64im_dual_opt_c908(int32_t r[256], const int16_t a[256], const int16_t b[256], int16_t cache[256], const int32_t zeta[128]);

// Cached optimized functions - 4 parameters
extern void poly_basemul_acc_cached_rv64im_opt_c908(int32_t r[256], const int16_t a[256], const int16_t b[256], const int16_t cache[256]);
extern void poly_basemul_acc_cached_rv64im_dual_opt_c908(int32_t r[256], const int16_t a[256], const int16_t b[256], const int16_t cache[256]);

// Cache end optimized functions - 5 parameters
extern void poly_basemul_acc_cache_end_rv64im_opt_c908(int16_t r[256], const int16_t a[256], const int16_t b[256], const int16_t cache[256], const int32_t acc[256]);
extern void poly_basemul_acc_cache_end_rv64im_dual_opt_c908(int16_t r[256], const int16_t a[256], const int16_t b[256], const int16_t cache[256], const int32_t acc[256]);

// Cache init optimized functions - 5 parameters
extern void poly_basemul_acc_cache_init_rv64im_opt_c908(int32_t r[256], const int16_t a[256], const int16_t b[256], int16_t cache[256], const int64_t zeta[128]);
extern void poly_basemul_acc_cache_init_rv64im_dual_opt_c908(int32_t r[256], const int16_t a[256], const int16_t b[256], int16_t cache[256], const int32_t zeta[128]);

// Cache init end optimized functions - 6 parameters
extern void poly_basemul_acc_cache_init_end_rv64im_opt_c908(int16_t r[256], const int16_t a[256], const int16_t b[256], int16_t cache[256], const int64_t zeta[128], const int32_t acc[256]);
extern void poly_basemul_acc_cache_init_end_rv64im_dual_opt_c908(int16_t r[256], const int16_t a[256], const int16_t b[256], int16_t cache[256], const int32_t zeta[128], const int32_t acc[256]);

// ===== RVV FUNCTIONS =====

// RVV functions - 4 parameters (int16_t r, int16_t a, int16_t b, int16_t table)
extern void poly_basemul_rvv_vlen128(int16_t r[256], const int16_t a[256], const int16_t b[256], const int16_t table[256]);
extern void poly_basemul_acc_rvv_vlen128(int16_t r[256], const int16_t a[256], const int16_t b[256], const int16_t table[256]);

// RVV functions - 5 parameters (int16_t r, int16_t a, int16_t b, int16_t table, int16_t b_cache)
extern void poly_basemul_cached_rvv_vlen128(int16_t r[256], const int16_t a[256], const int16_t b[256], const int16_t table[256], int16_t b_cache[256]);
extern void poly_basemul_acc_cached_rvv_vlen128(int16_t r[256], const int16_t a[256], const int16_t b[256], const int16_t table[256], int16_t b_cache[256]);
extern void poly_basemul_cache_init_rvv_vlen128(int16_t r[256], const int16_t a[256], const int16_t b[256], const int16_t table[256], int16_t b_cache[256]);
extern void poly_basemul_acc_cache_init_rvv_vlen128(int16_t r[256], const int16_t a[256], const int16_t b[256], const int16_t table[256], int16_t b_cache[256]);

// ===== RVV OPTIMIZED FUNCTIONS =====

// RVV optimized functions - 4 parameters (int16_t r, int16_t a, int16_t b, int16_t table)
extern void poly_basemul_rvv_vlen128_opt_c908(int16_t r[256], const int16_t a[256], const int16_t b[256], const int16_t table[256]);
extern void poly_basemul_acc_rvv_vlen128_opt_c908(int16_t r[256], const int16_t a[256], const int16_t b[256], const int16_t table[256]);

// RVV optimized functions - 5 parameters (int16_t r, int16_t a, int16_t b, int16_t table, int16_t b_cache)
extern void poly_basemul_cached_rvv_vlen128_opt_c908(int16_t r[256], const int16_t a[256], const int16_t b[256], const int16_t table[256], int16_t b_cache[256]);
extern void poly_basemul_acc_cached_rvv_vlen128_opt_c908(int16_t r[256], const int16_t a[256], const int16_t b[256], const int16_t table[256], int16_t b_cache[256]);
extern void poly_basemul_cache_init_rvv_vlen128_opt_c908(int16_t r[256], const int16_t a[256], const int16_t b[256], const int16_t table[256], int16_t b_cache[256]);
extern void poly_basemul_acc_cache_init_rvv_vlen128_opt_c908(int16_t r[256], const int16_t a[256], const int16_t b[256], const int16_t table[256], int16_t b_cache[256]);

// ===== Wrapper functions =====

// 4-parameter wrappers (int32_t r, int16_t a, int16_t b, int64_t zeta)
void poly_basemul_acc_rv64im_wrap(int32_t *r, const int16_t *a, const int16_t *b, const int64_t *zeta) {
    poly_basemul_acc_rv64im(r, a, b, zeta);
}

// 4-parameter wrappers (int32_t r, int16_t a, int16_t b, int32_t zeta)
void poly_basemul_acc_rv64im_dual_wrap(int32_t *r, const int16_t *a, const int16_t *b, const int32_t *zeta) {
    poly_basemul_acc_rv64im_dual(r, a, b, zeta);
}

// 5-parameter wrappers (int16_t r, int16_t a, int16_t b, int64_t zeta, int32_t acc)
void poly_basemul_acc_end_rv64im_wrap(int16_t *r, const int16_t *a, const int16_t *b, const int64_t *zeta, const int32_t *acc) {
    poly_basemul_acc_end_rv64im(r, a, b, zeta, acc);
}

void poly_basemul_acc_end_rv64im_dual_wrap(int16_t *r, const int16_t *a, const int16_t *b, const int32_t *zeta, const int32_t *acc) {
    poly_basemul_acc_end_rv64im_dual(r, a, b, zeta, acc);
}

// 5-parameter wrappers (int32_t r, int16_t a, int16_t b, int16_t cache, int64_t zeta)
void poly_basemul_cache_init_rv64im_wrap(int32_t *r, const int16_t *a, const int16_t *b, int16_t *cache, const int64_t *zeta) {
    poly_basemul_cache_init_rv64im(r, a, b, cache, zeta);
}

void poly_basemul_cache_init_rv64im_dual_wrap(int32_t *r, const int16_t *a, const int16_t *b, int16_t *cache, const int32_t *zeta) {
    poly_basemul_cache_init_rv64im_dual(r, a, b, cache, zeta);
}

// 4-parameter wrappers (int32_t r, int16_t a, int16_t b, int16_t cache)
void poly_basemul_acc_cached_rv64im_wrap(int32_t *r, const int16_t *a, const int16_t *b, const int16_t *cache) {
    poly_basemul_acc_cached_rv64im(r, a, b, cache);
}

void poly_basemul_acc_cached_rv64im_dual_wrap(int32_t *r, const int16_t *a, const int16_t *b, const int16_t *cache) {
    poly_basemul_acc_cached_rv64im_dual(r, a, b, cache);
}

// 5-parameter wrappers (int16_t r, int16_t a, int16_t b, int16_t cache, int32_t acc)
void poly_basemul_acc_cache_end_rv64im_wrap(int16_t *r, const int16_t *a, const int16_t *b, const int16_t *cache, const int32_t *acc) {
    poly_basemul_acc_cache_end_rv64im(r, a, b, cache, acc);
}

void poly_basemul_acc_cache_end_rv64im_dual_wrap(int16_t *r, const int16_t *a, const int16_t *b, const int16_t *cache, const int32_t *acc) {
    poly_basemul_acc_cache_end_rv64im_dual(r, a, b, cache, acc);
}

// 5-parameter wrappers (int32_t r, int16_t a, int16_t b, int16_t cache, int64_t zeta)
void poly_basemul_acc_cache_init_rv64im_wrap(int32_t *r, const int16_t *a, const int16_t *b, int16_t *cache, const int64_t *zeta) {
    poly_basemul_acc_cache_init_rv64im(r, a, b, cache, zeta);
}

void poly_basemul_acc_cache_init_rv64im_dual_wrap(int32_t *r, const int16_t *a, const int16_t *b, int16_t *cache, const int32_t *zeta) {
    poly_basemul_acc_cache_init_rv64im_dual(r, a, b, cache, zeta);
}

// 6-parameter wrappers (int16_t r, int16_t a, int16_t b, int16_t cache, int64_t zeta, int32_t acc)
void poly_basemul_acc_cache_init_end_rv64im_wrap(int16_t *r, const int16_t *a, const int16_t *b, int16_t *cache, const int64_t *zeta, const int32_t *acc) {
    poly_basemul_acc_cache_init_end_rv64im(r, a, b, cache, zeta, acc);
}

void poly_basemul_acc_cache_init_end_rv64im_dual_wrap(int16_t *r, const int16_t *a, const int16_t *b, int16_t *cache, const int32_t *zeta, const int32_t *acc) {
    poly_basemul_acc_cache_init_end_rv64im_dual(r, a, b, cache, zeta, acc);
}

// ===== OPTIMIZED WRAPPER FUNCTIONS =====

// Basic optimized wrappers
void poly_basemul_acc_rv64im_opt_c908_wrap(int32_t *r, const int16_t *a, const int16_t *b, const int64_t *zeta) {
    poly_basemul_acc_rv64im_opt_c908(r, a, b, zeta);
}

void poly_basemul_acc_rv64im_dual_opt_c908_wrap(int32_t *r, const int16_t *a, const int16_t *b, const int32_t *zeta) {
    poly_basemul_acc_rv64im_dual_opt_c908(r, a, b, zeta);
}

// End optimized wrappers
void poly_basemul_acc_end_rv64im_opt_c908_wrap(int16_t *r, const int16_t *a, const int16_t *b, const int64_t *zeta, const int32_t *acc) {
    poly_basemul_acc_end_rv64im_opt_c908(r, a, b, zeta, acc);
}

void poly_basemul_acc_end_rv64im_dual_opt_c908_wrap(int16_t *r, const int16_t *a, const int16_t *b, const int32_t *zeta, const int32_t *acc) {
    poly_basemul_acc_end_rv64im_dual_opt_c908(r, a, b, zeta, acc);
}

// Cache optimized wrappers
void poly_basemul_cache_init_rv64im_opt_c908_wrap(int32_t *r, const int16_t *a, const int16_t *b, int16_t *cache, const int64_t *zeta) {
    poly_basemul_cache_init_rv64im_opt_c908(r, a, b, cache, zeta);
}

void poly_basemul_cache_init_rv64im_dual_opt_c908_wrap(int32_t *r, const int16_t *a, const int16_t *b, int16_t *cache, const int32_t *zeta) {
    poly_basemul_cache_init_rv64im_dual_opt_c908(r, a, b, cache, zeta);
}

// Cached optimized wrappers
void poly_basemul_acc_cached_rv64im_opt_c908_wrap(int32_t *r, const int16_t *a, const int16_t *b, const int16_t *cache) {
    poly_basemul_acc_cached_rv64im_opt_c908(r, a, b, cache);
}

void poly_basemul_acc_cached_rv64im_dual_opt_c908_wrap(int32_t *r, const int16_t *a, const int16_t *b, const int16_t *cache) {
    poly_basemul_acc_cached_rv64im_dual_opt_c908(r, a, b, cache);
}

// Cache end optimized wrappers
void poly_basemul_acc_cache_end_rv64im_opt_c908_wrap(int16_t *r, const int16_t *a, const int16_t *b, const int16_t *cache, const int32_t *acc) {
    poly_basemul_acc_cache_end_rv64im_opt_c908(r, a, b, cache, acc);
}

void poly_basemul_acc_cache_end_rv64im_dual_opt_c908_wrap(int16_t *r, const int16_t *a, const int16_t *b, const int16_t *cache, const int32_t *acc) {
    poly_basemul_acc_cache_end_rv64im_dual_opt_c908(r, a, b, cache, acc);
}

// Cache init optimized wrappers
void poly_basemul_acc_cache_init_rv64im_opt_c908_wrap(int32_t *r, const int16_t *a, const int16_t *b, int16_t *cache, const int64_t *zeta) {
    poly_basemul_acc_cache_init_rv64im_opt_c908(r, a, b, cache, zeta);
}

void poly_basemul_acc_cache_init_rv64im_dual_opt_c908_wrap(int32_t *r, const int16_t *a, const int16_t *b, int16_t *cache, const int32_t *zeta) {
    poly_basemul_acc_cache_init_rv64im_dual_opt_c908(r, a, b, cache, zeta);
}

// Cache init end optimized wrappers
void poly_basemul_acc_cache_init_end_rv64im_opt_c908_wrap(int16_t *r, const int16_t *a, const int16_t *b, int16_t *cache, const int64_t *zeta, const int32_t *acc) {
    poly_basemul_acc_cache_init_end_rv64im_opt_c908(r, a, b, cache, zeta, acc);
}

void poly_basemul_acc_cache_init_end_rv64im_dual_opt_c908_wrap(int16_t *r, const int16_t *a, const int16_t *b, int16_t *cache, const int32_t *zeta, const int32_t *acc) {
    poly_basemul_acc_cache_init_end_rv64im_dual_opt_c908(r, a, b, cache, zeta, acc);
}

// ===== RVV WRAPPER FUNCTIONS =====

// RVV wrappers - 4 parameters (int16_t r, int16_t a, int16_t b, int16_t table)
void poly_basemul_rvv_vlen128_wrap(int16_t *r, const int16_t *a, const int16_t *b, const int16_t *table) {
    poly_basemul_rvv_vlen128(r, a, b, table);
}

void poly_basemul_acc_rvv_vlen128_wrap(int16_t *r, const int16_t *a, const int16_t *b, const int16_t *table) {
    poly_basemul_acc_rvv_vlen128(r, a, b, table);
}

// RVV wrappers - 5 parameters (int16_t r, int16_t a, int16_t b, int16_t table, int16_t b_cache)
void poly_basemul_cached_rvv_vlen128_wrap(int16_t *r, const int16_t *a, const int16_t *b, const int16_t *table, int16_t *b_cache) {
    poly_basemul_cached_rvv_vlen128(r, a, b, table, b_cache);
}

void poly_basemul_acc_cached_rvv_vlen128_wrap(int16_t *r, const int16_t *a, const int16_t *b, const int16_t *table, int16_t *b_cache) {
    poly_basemul_acc_cached_rvv_vlen128(r, a, b, table, b_cache);
}

void poly_basemul_cache_init_rvv_vlen128_wrap(int16_t *r, const int16_t *a, const int16_t *b, const int16_t *table, int16_t *b_cache) {
    poly_basemul_cache_init_rvv_vlen128(r, a, b, table, b_cache);
}

void poly_basemul_acc_cache_init_rvv_vlen128_wrap(int16_t *r, const int16_t *a, const int16_t *b, const int16_t *table, int16_t *b_cache) {
    poly_basemul_acc_cache_init_rvv_vlen128(r, a, b, table, b_cache);
}

// ===== RVV OPTIMIZED WRAPPER FUNCTIONS =====

// RVV optimized wrappers - 4 parameters (int16_t r, int16_t a, int16_t b, int16_t table)
void poly_basemul_rvv_vlen128_opt_c908_wrap(int16_t *r, const int16_t *a, const int16_t *b, const int16_t *table) {
    poly_basemul_rvv_vlen128_opt_c908(r, a, b, table);
}

void poly_basemul_acc_rvv_vlen128_opt_c908_wrap(int16_t *r, const int16_t *a, const int16_t *b, const int16_t *table) {
    poly_basemul_acc_rvv_vlen128_opt_c908(r, a, b, table);
}

// RVV optimized wrappers - 5 parameters (int16_t r, int16_t a, int16_t b, int16_t table, int16_t b_cache)
void poly_basemul_cached_rvv_vlen128_opt_c908_wrap(int16_t *r, const int16_t *a, const int16_t *b, const int16_t *table, int16_t *b_cache) {
    poly_basemul_cached_rvv_vlen128_opt_c908(r, a, b, table, b_cache);
}

void poly_basemul_acc_cached_rvv_vlen128_opt_c908_wrap(int16_t *r, const int16_t *a, const int16_t *b, const int16_t *table, int16_t *b_cache) {
    poly_basemul_acc_cached_rvv_vlen128_opt_c908(r, a, b, table, b_cache);
}

void poly_basemul_cache_init_rvv_vlen128_opt_c908_wrap(int16_t *r, const int16_t *a, const int16_t *b, const int16_t *table, int16_t *b_cache) {
    poly_basemul_cache_init_rvv_vlen128_opt_c908(r, a, b, table, b_cache);
}

void poly_basemul_acc_cache_init_rvv_vlen128_opt_c908_wrap(int16_t *r, const int16_t *a, const int16_t *b, const int16_t *table, int16_t *b_cache) {
    poly_basemul_acc_cache_init_rvv_vlen128_opt_c908(r, a, b, table, b_cache);
}

#endif // PQRV_PAPER_H
