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
 * from PQRV's own test harness (test/test_kyber_ntt_rvv.c).
 */

#include "kyber-ref.h"

#define KYBER_QINV (-3327) /* q^-1 mod 2^16 */

const int16_t zetas_ref[128] = {
    -1044, -758,  -359,  -1517, 1493,  1422,  287,   202,   -171,  622,
    1577,  182,   962,   -1202, -1474, 1468,  573,   -1325, 264,   383,
    -829,  1458,  -1602, -130,  -681,  1017,  732,   608,   -1542, 411,
    -205,  -1571, 1223,  652,   -552,  1015,  -1293, 1491,  -282,  -1544,
    516,   -8,    -320,  -666,  -1618, -1162, 126,   1469,  -853,  -90,
    -271,  830,   107,   -1421, -247,  -951,  -398,  961,   -1508, -725,
    448,   -1065, 677,   -1275, -1103, 430,   555,   843,   -1251, 871,
    1550,  105,   422,   587,   177,   -235,  -291,  -460,  1574,  1653,
    -246,  778,   1159,  -147,  -777,  1483,  -602,  1119,  -1590, 644,
    -872,  349,   418,   329,   -156,  -75,   817,   1097,  603,   610,
    1322,  -1285, -1465, 384,   -1215, -136,  1218,  -1335, -874,  220,
    -1187, -1659, -1185, -1530, -1278, 794,   -1510, -854,  -870,  478,
    -108,  -308,  996,   991,   958,   -1460, 1522,  1628};

int16_t montgomery_reduce_ref(int32_t a)
{
    int16_t t;

    t = (int16_t)((int16_t)a * KYBER_QINV);
    t = (int16_t)((a - (int32_t)t * KYBER_Q) >> 16);
    return t;
}

int16_t barrett_reduce_ref(int16_t a)
{
    int16_t t;
    const int16_t v = ((1 << 26) + KYBER_Q / 2) / KYBER_Q;

    t = (int16_t)(((int32_t)v * a + (1 << 25)) >> 26);
    t = (int16_t)(t * KYBER_Q);
    return (int16_t)(a - t);
}

static int16_t fqmul(int16_t a, int16_t b)
{
    return montgomery_reduce_ref((int32_t)a * b);
}

void ntt_ref(int16_t r[KYBER_N])
{
    unsigned int len, start, j, k;
    int16_t t, zeta;

    j = 0;
    k = 1;
    for (len = 128; len >= 2; len >>= 1) {
        for (start = 0; start < 256; start = j + len) {
            zeta = zetas_ref[k++];
            for (j = start; j < start + len; j++) {
                t = fqmul(zeta, r[j + len]);
                r[j + len] = (int16_t)(r[j] - t);
                r[j] = (int16_t)(r[j] + t);
            }
        }
    }
}

void intt_ref(int16_t r[KYBER_N])
{
    unsigned int start, len, j, k;
    int16_t t, zeta;
    const int16_t f = 1441; /* mont^2/128 */

    j = 0;
    k = 127;
    for (len = 2; len <= 128; len <<= 1) {
        for (start = 0; start < 256; start = j + len) {
            zeta = zetas_ref[k--];
            for (j = start; j < start + len; j++) {
                t = r[j];
                r[j] = barrett_reduce_ref((int16_t)(t + r[j + len]));
                r[j + len] = (int16_t)(r[j + len] - t);
                r[j + len] = fqmul(zeta, r[j + len]);
            }
        }
    }

    for (j = 0; j < 256; j++)
        r[j] = fqmul(r[j], f);
}

int poly_equal_mod_q(const int16_t *a, const int16_t *b, unsigned n)
{
    unsigned i;

    for (i = 0; i < n; i++) {
        if (((a[i] + KYBER_Q * 10) % KYBER_Q) != ((b[i] + KYBER_Q * 10) % KYBER_Q))
            return 0;
    }
    return 1;
}
