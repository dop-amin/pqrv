# SLOTHY Optimization Results — PQRV on XuanTie C908

Measured on the K230 board (XuanTie C908, RVV 1.0, VLEN=128) with the hardware performance
counters, read from user mode (`csrr`). **Cycles**: C908 raw event `0x2D` (U-mode cycles) per
call. **Instr.**: minimum over all windows per call. **Clean**: windows that reached that minimum.
**Faster by** = (naive cycles / optimized cycles − 1) × 100, against the baseline of the same
variant (dual-issue against dual-issue); `—` marks a baseline. Scalar kernels from the
`cross-rv64im` build, vector kernels from `cross-rvv`. Raw data: `bench_results/*.csv`.

## Summary

| Category             | Kernels |    Median |  Best |  Worst |
| -------------------- | ------: | --------: | ----: | -----: |
| Scalar, single-issue |      18 | **+111%** | +196% |   +24% |
| Scalar, dual-issue   |      18 |  **+31%** |  +51% |    +7% |
| RVV (VLEN=128)       |      22 |   **+3%** |  +58% | ⚠️ −2% |

## Results by kernel group

### Dilithium — NTT / INTT

**Scalar (`cross-rv64im`)**

| Function                                       | Cycles | Instr. |  IPC |    Clean | Faster by |
| ---------------------------------------------- | -----: | -----: | ---: | -------: | --------: |
| `ntt_8l_rv64im`                                |  17337 |   7666 | 0.44 | 892/1000 |         — |
| `ntt_8l_rv64im_opt`                            |   5854 |   7664 | 1.31 | 963/1000 | **+196%** |
| `ntt_8l_dual_rv64im`                           |   8250 |   7762 | 0.94 | 949/1000 |         — |
| `ntt_8l_dual_rv64im_opt`                       |   6459 |   7762 | 1.20 | 959/1000 |      +28% |
| `intt_dilithium_8l_plant_rv64im`               |  14987 |   8209 | 0.55 | 974/1000 |         — |
| `intt_dilithium_8l_plant_rv64im_opt_c908`      |   6879 |   8207 | 1.19 | 989/1000 | **+118%** |
| `intt_dilithium_8l_plant_rv64im_dual`          |   8477 |   8290 | 0.98 | 984/1000 |         — |
| `intt_dilithium_8l_plant_rv64im_dual_opt_c908` |   6730 |   8290 | 1.23 | 989/1000 |      +26% |

**Vector, VLEN=128 (`cross-rvv`)**

| Function                                | Cycles | Instr. |  IPC |    Clean | Faster by |
| --------------------------------------- | -----: | -----: | ---: | -------: | --------: |
| `ntt_rvv_vlen128`                       |   3318 |   2430 | 0.73 | 995/1000 |         — |
| `ntt_rvv_vlen128_opt_c908`              |   3274 |   2430 | 0.74 | 995/1000 |       +1% |
| `intt_rvv_vlen128`                      |   3515 |   2677 | 0.76 | 993/1000 |         — |
| `intt_rvv_vlen128_opt_c908`             |   3446 |   2677 | 0.78 | 993/1000 |       +2% |
| `ntt_rvv_vlen128_barret_mul`            |   4218 |   2199 | 0.52 | 993/1000 |         — |
| `ntt_rvv_vlen128_barret_mul_opt_c908`   |   2673 |   2197 | 0.82 | 995/1000 |      +58% |
| `intt_rvv_vlen128_barret_mul`           |   4894 |   2584 | 0.53 | 990/1000 |         — |
| `intt_rvv_vlen128_barret_mul_opt_c908`  |   3346 |   2582 | 0.77 | 993/1000 |      +46% |
| `ntt2normal_order_rvv_vlen128`          |   1032 |    320 | 0.31 | 999/1000 |         — |
| `ntt2normal_order_rvv_vlen128_opt_c908` |    992 |    319 | 0.32 | 999/1000 |       +4% |
| `normal2ntt_order_rvv_vlen128`          |   1030 |    320 | 0.31 | 999/1000 |         — |
| `normal2ntt_order_rvv_vlen128_opt_c908` |   1031 |    320 | 0.31 | 999/1000 |    ⚠️ −0% |

### Dilithium — Poly Basemul & Reduce

**Scalar (`cross-rv64im`)**

| Function                                       | Cycles | Instr. |  IPC |    Clean | Faster by |
| ---------------------------------------------- | -----: | -----: | ---: | -------: | --------: |
| `poly_basemul_8l_acc_rv64im`                   |   1695 |   1701 | 1.00 | 995/1000 |         — |
| `poly_basemul_8l_acc_rv64im_opt_c908`          |   1151 |   1701 | 1.48 | 997/1000 |      +47% |
| `poly_basemul_8l_acc_end_rv64im`               |   3367 |   2992 | 0.89 | 994/1000 |         — |
| `poly_basemul_8l_acc_end_rv64im_opt_c908`      |   2408 |   2991 | 1.24 | 996/1000 |      +40% |
| `poly_basemul_8l_init_rv64im`                  |   1119 |   1189 | 1.06 | 998/1000 |         — |
| `poly_basemul_8l_init_rv64im_opt_c908`         |    895 |   1189 | 1.33 | 999/1000 |      +25% |
| `poly_basemul_8l_rv64im`                       |   2805 |   2351 | 0.84 | 996/1000 |         — |
| `poly_basemul_8l_rv64im_opt_c908`              |   2036 |   2350 | 1.15 | 997/1000 |      +38% |
| `poly_basemul_8l_acc_rv64im_dual`              |   1375 |   1701 | 1.24 | 997/1000 |         — |
| `poly_basemul_8l_acc_rv64im_opt_c908_dual`     |   1151 |   1701 | 1.48 | 998/1000 |      +19% |
| `poly_basemul_8l_acc_end_rv64im_dual`          |   2950 |   2801 | 0.95 | 995/1000 |         — |
| `poly_basemul_8l_acc_end_rv64im_opt_c908_dual` |   2215 |   2801 | 1.26 | 997/1000 |      +33% |
| `poly_basemul_8l_init_rv64im_dual`             |    863 |   1189 | 1.38 | 999/1000 |         — |
| `poly_basemul_8l_init_rv64im_opt_c908_dual`    |    799 |   1189 | 1.49 | 997/1000 |       +8% |
| `poly_basemul_8l_rv64im_dual`                  |   2414 |   2223 | 0.92 | 996/1000 |         — |
| `poly_basemul_8l_rv64im_opt_c908_dual`         |   2259 |   2222 | 0.98 | 997/1000 |       +7% |
| `poly_reduce_rv64im`                           |   1629 |   1700 | 1.04 | 998/1000 |         — |
| `poly_reduce_rv64im_opt_c908`                  |   1311 |   1700 | 1.30 | 998/1000 |      +24% |
| `poly_reduce_rv64im_dual`                      |   1629 |   1700 | 1.04 | 996/1000 |         — |
| `poly_reduce_rv64im_dual_opt_c908`             |   1375 |   1700 | 1.24 | 998/1000 |      +18% |

**Vector, VLEN=128 (`cross-rvv`)**

| Function                           | Cycles | Instr. |  IPC |     Clean | Faster by |
| ---------------------------------- | -----: | -----: | ---: | --------: | --------: |
| `poly_reduce_rvv_vlen128`          |    475 |     79 | 0.17 | 1000/1000 |         — |
| `poly_reduce_rvv_vlen128_opt_c908` |    476 |     78 | 0.16 | 1000/1000 |    ⚠️ −0% |

### Kyber — NTT / INTT

**Scalar (`cross-rv64im`)**

| Function                          | Cycles | Instr. |  IPC |    Clean | Faster by |
| --------------------------------- | -----: | -----: | ---: | -------: | --------: |
| `ntt_kyber_rv64im`                |  15001 |   6770 | 0.45 | 909/1000 |         — |
| `ntt_kyber_rv64im_opt_c908`       |   5310 |   6768 | 1.27 | 985/1000 | **+183%** |
| `ntt_kyber_dual_rv64im`           |   6570 |   6866 | 1.05 | 990/1000 |         — |
| `ntt_kyber_dual_rv64im_opt_c908`  |   5434 |   6866 | 1.26 | 990/1000 |      +21% |
| `intt_kyber_rv64im`               |  13435 |   7313 | 0.54 | 976/1000 |         — |
| `intt_kyber_rv64im_opt_c908`      |   6106 |   7311 | 1.20 | 988/1000 | **+120%** |
| `intt_kyber_dual_rv64im`          |   6956 |   7394 | 1.06 | 989/1000 |         — |
| `intt_kyber_dual_rv64im_opt_c908` |   5545 |   7394 | 1.33 | 989/1000 |      +25% |

**Vector, VLEN=128 (`cross-rvv`)**

| Function                                      | Cycles | Instr. |  IPC |    Clean | Faster by |
| --------------------------------------------- | -----: | -----: | ---: | -------: | --------: |
| `ntt_kyber_rvv_vlen128`                       |   1569 |    750 | 0.48 | 998/1000 |         — |
| `ntt_kyber_rvv_vlen128_opt_c908`              |   1542 |    750 | 0.49 | 998/1000 |       +2% |
| `intt_kyber_rvv_vlen128`                      |   1811 |    955 | 0.53 | 997/1000 |         — |
| `intt_kyber_rvv_vlen128_opt_c908`             |   1696 |    955 | 0.56 | 997/1000 |       +7% |
| `kyber_normal2ntt_order_rvv_vlen128`          |    692 |    255 | 0.37 | 999/1000 |         — |
| `kyber_normal2ntt_order_rvv_vlen128_opt_c908` |    708 |    254 | 0.36 | 998/1000 |    ⚠️ −2% |
| `kyber_ntt2normal_order_rvv_vlen128`          |    706 |    255 | 0.36 | 999/1000 |         — |
| `kyber_ntt2normal_order_rvv_vlen128_opt_c908` |    710 |    254 | 0.36 | 999/1000 |    ⚠️ −1% |

### Kyber — Poly Basemul

**Scalar (`cross-rv64im`)**

| Function                                               | Cycles | Instr. |  IPC |    Clean | Faster by |
| ------------------------------------------------------ | -----: | -----: | ---: | -------: | --------: |
| `poly_basemul_acc_rv64im`                              |   4960 |   3111 | 0.63 | 969/1000 |         — |
| `poly_basemul_acc_rv64im_opt_c908`                     |   2399 |   3110 | 1.30 | 985/1000 | **+107%** |
| `poly_basemul_acc_rv64im_dual`                         |   2624 |   2919 | 1.11 | 984/1000 |         — |
| `poly_basemul_acc_rv64im_dual_opt_c908`                |   1737 |   2918 | 1.68 | 988/1000 |      +51% |
| `poly_basemul_acc_end_rv64im`                          |   8296 |   4208 | 0.51 | 957/1000 |         — |
| `poly_basemul_acc_end_rv64im_opt_c908`                 |   4001 |   4207 | 1.05 | 994/1000 | **+107%** |
| `poly_basemul_acc_end_rv64im_dual`                     |   3778 |   4043 | 1.07 | 993/1000 |         — |
| `poly_basemul_acc_end_rv64im_dual_opt_c908`            |   2947 |   4042 | 1.37 | 993/1000 |      +28% |
| `poly_basemul_cache_init_rv64im`                       |   4705 |   2792 | 0.59 | 993/1000 |         — |
| `poly_basemul_cache_init_rv64im_opt_c908`              |   2208 |   2791 | 1.26 | 997/1000 | **+113%** |
| `poly_basemul_cache_init_rv64im_dual`                  |   2497 |   2568 | 1.03 | 995/1000 |         — |
| `poly_basemul_cache_init_rv64im_dual_opt_c908`         |   1769 |   2567 | 1.45 | 997/1000 |      +41% |
| `poly_basemul_acc_cached_rv64im`                       |   3424 |   2599 | 0.76 | 993/1000 |         — |
| `poly_basemul_acc_cached_rv64im_opt_c908`              |   1637 |   2598 | 1.59 | 998/1000 | **+109%** |
| `poly_basemul_acc_cached_rv64im_dual`                  |   2112 |   2407 | 1.14 | 997/1000 |         — |
| `poly_basemul_acc_cached_rv64im_dual_opt_c908`         |   1441 |   2406 | 1.67 | 998/1000 |      +47% |
| `poly_basemul_acc_cache_end_rv64im`                    |   6888 |   3696 | 0.54 | 987/1000 |         — |
| `poly_basemul_acc_cache_end_rv64im_opt_c908`           |   2670 |   3695 | 1.38 | 996/1000 | **+158%** |
| `poly_basemul_acc_cache_end_rv64im_dual`               |   3266 |   3531 | 1.08 | 994/1000 |         — |
| `poly_basemul_acc_cache_end_rv64im_dual_opt_c908`      |   2380 |   3530 | 1.48 | 996/1000 |      +37% |
| `poly_basemul_acc_cache_init_rv64im`                   |   4961 |   3304 | 0.67 | 991/1000 |         — |
| `poly_basemul_acc_cache_init_rv64im_opt_c908`          |   2401 |   3303 | 1.38 | 995/1000 | **+107%** |
| `poly_basemul_acc_cache_init_rv64im_dual`              |   2657 |   3080 | 1.16 | 996/1000 |         — |
| `poly_basemul_acc_cache_init_rv64im_dual_opt_c908`     |   1954 |   3079 | 1.58 | 996/1000 |      +36% |
| `poly_basemul_acc_cache_init_end_rv64im`               |   8360 |   4401 | 0.53 | 985/1000 |         — |
| `poly_basemul_acc_cache_init_end_rv64im_opt_c908`      |   3755 |   4400 | 1.17 | 994/1000 | **+123%** |
| `poly_basemul_acc_cache_init_end_rv64im_dual`          |   4161 |   4489 | 1.08 | 994/1000 |         — |
| `poly_basemul_acc_cache_init_end_rv64im_dual_opt_c908` |   3072 |   4488 | 1.46 | 994/1000 |      +35% |

**Vector, VLEN=128 (`cross-rvv`)**

| Function                                           | Cycles | Instr. |  IPC |     Clean | Faster by |
| -------------------------------------------------- | -----: | -----: | ---: | --------: | --------: |
| `poly_basemul_rvv_vlen128`                         |    676 |    688 | 1.02 |  998/1000 |         — |
| `poly_basemul_rvv_vlen128_opt_c908`                |    687 |    688 | 1.00 |  999/1000 |    ⚠️ −2% |
| `poly_basemul_acc_rvv_vlen128`                     |    772 |    784 | 1.02 |  998/1000 |         — |
| `poly_basemul_acc_rvv_vlen128_opt_c908`            |    702 |    784 | 1.12 |  999/1000 |      +10% |
| `poly_basemul_cached_rvv_vlen128`                  |    597 |    607 | 1.02 |  999/1000 |         — |
| `poly_basemul_cached_rvv_vlen128_opt_c908`         |    580 |    607 | 1.05 |  999/1000 |       +3% |
| `poly_basemul_acc_cached_rvv_vlen128`              |    695 |    707 | 1.02 |  999/1000 |         — |
| `poly_basemul_acc_cached_rvv_vlen128_opt_c908`     |    631 |    707 | 1.12 |  999/1000 |      +10% |
| `poly_basemul_cache_init_rvv_vlen128`              |    714 |    725 | 1.02 |  998/1000 |         — |
| `poly_basemul_cache_init_rvv_vlen128_opt_c908`     |    692 |    725 | 1.05 |  999/1000 |       +3% |
| `poly_basemul_acc_cache_init_rvv_vlen128`          |    810 |    821 | 1.01 |  998/1000 |         — |
| `poly_basemul_acc_cache_init_rvv_vlen128_opt_c908` |    778 |    821 | 1.06 | 1000/1000 |       +4% |

### Kyber — Poly Reduce / Domain Conversion

**Scalar (`cross-rv64im`)**

| Function                                 | Cycles | Instr. |  IPC |    Clean | Faster by |
| ---------------------------------------- | -----: | -----: | ---: | -------: | --------: |
| `poly_plantard_rdc_rv64im`               |   3284 |   1643 | 0.50 | 979/1000 |         — |
| `poly_plantard_rdc_rv64im_opt_c908`      |   1385 |   1642 | 1.19 | 991/1000 | **+137%** |
| `poly_plantard_rdc_rv64im_dual`          |   1552 |   1622 | 1.05 | 991/1000 |         — |
| `poly_plantard_rdc_rv64im_dual_opt_c908` |   1045 |   1621 | 1.55 | 995/1000 |      +49% |
| `poly_toplant_rv64im`                    |   3286 |   1645 | 0.50 | 980/1000 |         — |
| `poly_toplant_rv64im_opt_c908`           |   1300 |   1644 | 1.26 | 992/1000 | **+153%** |
| `poly_toplant_rv64im_dual`               |   1554 |   1624 | 1.05 | 989/1000 |         — |
| `poly_toplant_rv64im_dual_opt_c908`      |   1152 |   1623 | 1.41 | 992/1000 |      +35% |

**Vector, VLEN=128 (`cross-rvv`)**

| Function                           | Cycles | Instr. |  IPC |    Clean | Faster by |
| ---------------------------------- | -----: | -----: | ---: | -------: | --------: |
| `poly_reduce_rvv_vlen128`          |    250 |     40 | 0.16 | 999/1000 |         — |
| `poly_reduce_rvv_vlen128_opt_c908` |    250 |     40 | 0.16 | 998/1000 |       +0% |
| `poly_tomont_rvv_vlen128`          |    236 |     40 | 0.17 | 998/1000 |         — |
| `poly_tomont_rvv_vlen128_opt_c908` |    228 |     40 | 0.18 | 998/1000 |       +4% |

### Kyber — Sampling

**Vector, VLEN=128 (`cross-rvv`)**

| Function                           | Cycles | Instr. |  IPC |    Clean | Faster by |
| ---------------------------------- | -----: | -----: | ---: | -------: | --------: |
| `cbd2_rvv_vlen128`                 |    472 |    238 | 0.50 | 996/1000 |         — |
| `cbd2_rvv_vlen128_opt_c908`        |    424 |    238 | 0.56 | 997/1000 |      +11% |
| `cbd3_rvv_vlen128`                 |   1048 |    583 | 0.56 | 993/1000 |         — |
| `cbd3_rvv_vlen128_opt_c908`        |    942 |    582 | 0.62 | 994/1000 |      +11% |
| `rej_uniform_rvv_vlen128`          |    865 |    581 | 0.67 | 996/1000 |         — |
| `rej_uniform_rvv_vlen128_opt_c908` |    746 |    539 | 0.72 | 997/1000 |      +16% |

## Regressions

| Kernel                                        | Naive cycles | Optimized cycles | Extra cycles |
| --------------------------------------------- | -----------: | ---------------: | -----------: |
| `kyber_normal2ntt_order_rvv_vlen128_opt_c908` |          692 |              708 |      **+2%** |
| `poly_basemul_rvv_vlen128_opt_c908`           |          676 |              687 |      **+2%** |
| `kyber_ntt2normal_order_rvv_vlen128_opt_c908` |          706 |              710 |      **+1%** |
| `poly_reduce_rvv_vlen128_opt_c908`            |          475 |              476 |      **+0%** |
| `normal2ntt_order_rvv_vlen128_opt_c908`       |         1030 |             1031 |      **+0%** |
