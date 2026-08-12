/*
 * Copyright (c) 2026 Justus Bergermann
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

/*
 * Plain-C algorithmic reference for the Kyber NTT / inverse NTT, transcribed
 * from PQRV's own test harness (test/test_kyber_ntt_rvv.c) so that the
 * assembly kernels can be checked against a real algorithm and not only
 * against each other.
 */

#ifndef KYBER_REF_H
#define KYBER_REF_H

#include <stdint.h>

#include "kyber.h"

/* Reference (natural-order) twiddle factors, Montgomery domain. */
extern const int16_t zetas_ref[128];

int16_t montgomery_reduce_ref(int32_t a);
int16_t barrett_reduce_ref(int16_t a);

/* In-place forward / inverse NTT on 256 coefficients, natural order. */
void ntt_ref(int16_t r[KYBER_N]);
void intt_ref(int16_t r[KYBER_N]);

/* Compare two polynomials modulo KYBER_Q (representatives may differ).
 * Returns 1 if equal, 0 otherwise. */
int poly_equal_mod_q(const int16_t *a, const int16_t *b, unsigned n);

#endif /* KYBER_REF_H */
