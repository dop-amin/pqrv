# 0 "ntt_dilithium_rvv_vlen128_barret_mul.s"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/nix/store/dcvg6lshngpfr8lyn4dzgj2hkp6l6m1v-riscv64-unknown-linux-gnu-gcc-13.3.0/riscv64-unknown-linux-gnu/sys-include/stdc-predef.h" 1 3 4
# 0 "<command-line>" 2
# 1 "ntt_dilithium_rvv_vlen128_barret_mul.s"
# 79 "ntt_dilithium_rvv_vlen128_barret_mul.s"
.macro barret_mul dst, a, b, barret_const
    vmul.vx \dst, \a, \b
    vmulhu.vx v16, \a, \barret_const
    vnmsac.vx \dst, x4, v16
.endm

.macro ct_butterfly a, b, root, barret_const
    barret_mul v17, \b, \root, \barret_const
    vsub.vv \b, \a, v17
    vadd.vv \a, \a, v17
.endm

.macro barret_mul_v vdst, va, vb, vbarret_const
    vmul.vv \vdst, \va, \vb
    vmulhu.vv v16, \va, \vbarret_const
    vnmsac.vx \vdst, x4, v16
.endm

.macro ct_butterfly_v va, vb, vroot, vbarret_const
    barret_mul_v v17, \vb, \vroot, \vbarret_const
    vsub.vv \vb, \va, v17
    vadd.vv \va, \va, v17
.endm

.macro transpose4 data, ptr
    vsseg4e32.v \data, (\ptr)
    vl4re32.v \data, (\ptr)



.endm

.macro load_roots_1234 x5, x7, x8, x9, x1, x11, x12, x13, x14, x15, x16, x17, x18, x19, x20, x21, x22, x23, x24, x25, x26, x27, x28, x29, x30, x31



    lw \ x7, (0*8+4)(\ x5)
    lw \ x8, (1*8+4)(\ x5)
    lw \ x9, (2*8+4)(\ x5)
    lw \ x1, (3*8+4)(\ x5)
    lw \ x11, (4*8+4)(\ x5)
    lw \ x12, (5*8)(\ x5)
    lw \ x13, (5*8+4)(\ x5)
    lw \ x14, (6*8)(\ x5)
    lw \ x15, (6*8+4)(\ x5)
    lw \ x16, (7*8)(\ x5)
    lw \ x17, (7*8+4)(\ x5)
    lw \ x18, (8*8)(\ x5)
    lw \ x19, (8*8+4)(\ x5)
    lw \ x20, (9*8)(\ x5)
    lw \ x21, (9*8+4)(\ x5)
    lw \ x22, (10*8)(\ x5)
    lw \ x23, (10*8+4)(\ x5)
    lw \ x24, (11*8)(\ x5)
    lw \ x25, (11*8+4)(\ x5)
    lw \ x26, (12*8)(\ x5)
    lw \ x27, (12*8+4)(\ x5)
    lw \ x28, (13*8)(\ x5)
    lw \ x29, (13*8+4)(\ x5)
    lw \ x30, (14*8)(\ x5)
    lw \ x31, (14*8+4)(\ x5)
.endm


.macro load_roots_5678 xroot1, xbarretc_1, xroot2, xbarretc_2, xroot3, xbarretc_3, vroot1, vbarretc_1, vroot2, vbarretc_2, vroot3, vbarretc_3, x5


    lw \xroot1, (0*8)(\ x5)
    lw \xbarretc_1, (0*8+4)(\ x5)
    lw \xroot2, (1*8)(\ x5)
    lw \xbarretc_2, (1*8+4)(\ x5)
    lw \xroot3, (2*8)(\ x5)
    lw \xbarretc_3, (2*8+4)(\ x5)
    addi \ x5, \ x5, 3*8
    vle32.v \vroot1, (\ x5)
    addi \ x5, \ x5, 16
    vle32.v \vbarretc_1, (\ x5)
    addi \ x5, \ x5, 16
    vle32.v \vroot2, (\ x5)
    addi \ x5, \ x5, 16
    vle32.v \vbarretc_2, (\ x5)
    addi \ x5, \ x5, 16
    vle32.v \vroot3, (\ x5)
    addi \ x5, \ x5, 16
    vle32.v \vbarretc_3, (\ x5)
    addi \ x5, \ x5, 16
.endm

.macro load16 x10, stride
    .irp r, v0,v1,v2,v3,v4,v5,v6,v7, v8,v9,v10,v11,v12,v13,v14

        vle32.v \r, (\ x10)
        addi \ x10, \ x10, \stride
    .endr
    vle32.v v15, (\ x10)
.endm

.macro store16 x10, stride
    .irp r, v0,v1,v2,v3,v4,v5,v6,v7, v8,v9,v10,v11,v12,v13,v14

        vse32.v \r, (\ x10)
        addi \ x10, \ x10, \stride
    .endr
    vse32.v v15, (\ x10)
.endm

.macro push_stack
    addi sp, sp, -8*15
    sd s0, 0*8(sp)
    sd s1, 1*8(sp)
    sd s2, 2*8(sp)
    sd s3, 3*8(sp)
    sd s4, 4*8(sp)
    sd s5, 5*8(sp)
    sd s6, 6*8(sp)
    sd s7, 7*8(sp)
    sd s8, 8*8(sp)
    sd s9, 9*8(sp)
    sd s10, 10*8(sp)
    sd s11, 11*8(sp)
    sd gp, 12*8(sp)
    sd tp, 13*8(sp)
    sd ra, 14*8(sp)
.endm

.macro pop_stack
    ld s0, 0*8(sp)
    ld s1, 1*8(sp)
    ld s2, 2*8(sp)
    ld s3, 3*8(sp)
    ld s4, 4*8(sp)
    ld s5, 5*8(sp)
    ld s6, 6*8(sp)
    ld s7, 7*8(sp)
    ld s8, 8*8(sp)
    ld s9, 9*8(sp)
    ld s10, 10*8(sp)
    ld s11, 11*8(sp)
    ld gp, 12*8(sp)
    ld tp, 13*8(sp)
    ld ra, 14*8(sp)
    addi sp, sp, 8*15
.endm

.data

.p2align 4
roots:
# 1 "ntt_dilithium_1234_5678_twiddles_barret_mul.s" 1
# 27 "ntt_dilithium_1234_5678_twiddles_barret_mul.s"
roots_l0123:
.word 4808194
.word 2464201481
.word 3765607
.word 1929875198
.word 3761513
.word 1927777021
.word 5178923
.word 2654200252
.word 5496691
.word 2817056488
.word 5234739
.word 2682805976
.word 5178987
.word 2654233052
.word 7778734
.word 3986604501
.word 3542485
.word 1815525077
.word 2682288
.word 1374673747
.word 2129892
.word 1091570561
.word 3764867
.word 1929495947
.word 7375178
.word 3779781879
.word 557458
.word 285697463
.word 7159240
.word 3669113561
.word 0
.word 0
roots_l4567:
.word 5010068
.word 2567661992
.word 3602218
.word 1846138265
.word 3182878
.word 1631226336
.word 3073009
.word 1277625
.word 5744944
.word 3852015
.word 1574918427
.word 654783359
.word 2944286257
.word 1974159335
.word 1753
.word 5720892
.word 2660408
.word 8321269
.word 898413
.word 2931959596
.word 1363460238
.word 4264653921
.word 6444997
.word 6924527
.word 6600190
.word 2772600
.word 3303063718
.word 3548823048
.word 3382600197
.word 1420958686
.word 4317364
.word 2212650896
.word 2740543
.word 1404529459
.word 4793971
.word 2456912187
.word 4183372
.word 5157610
.word 5258977
.word 8106357
.word 2143979939
.word 2643277330
.word 2695227961
.word 4154511429
.word 1182243
.word 636927
.word 4423672
.word 5095502
.word 605900043
.word 326425360
.word 2267133792
.word 2611446954
.word 87208
.word 4415111
.word 6084020
.word 4663471
.word 44694137
.word 2262746275
.word 3118062852
.word 2390030882
.word 2663378
.word 1364982364
.word 5269599
.word 2700671741
.word 2101410
.word 1076973524
.word 2508980
.word 2028118
.word 1937570
.word 4564692
.word 1285853323
.word 1039411342
.word 993005454
.word 2339406602
.word 8352605
.word 1009365
.word 6400920
.word 4423473
.word 4280713634
.word 517299994
.word 3280474237
.word 2267031804
.word 822541
.word 5926272
.word 1596822
.word 4620952
.word 421552614
.word 3037216934
.word 818371958
.word 2368239876
.word 6705802
.word 3436726392
.word 3704823
.word 1898723372
.word 1159875
.word 594436433
.word 2811291
.word 5396636
.word 7270901
.word 4158088
.word 1440787840
.word 2765778258
.word 3726339872
.word 2131021878
.word 6695264
.word 2678278
.word 4829411
.word 8129971
.word 3431325663
.word 1372618620
.word 2475075203
.word 4166613614
.word 4969849
.word 4611469
.word 635956
.word 5925040
.word 2547049738
.word 2363379834
.word 325927722
.word 3036585534
.word 4855975
.word 2488689264
.word 394148
.word 202001019
.word 928749
.word 475984260
.word 1528066
.word 482649
.word 1148858
.word 5418153
.word 783134478
.word 247357819
.word 588790216
.word 2776805729
.word 4234153
.word 2192938
.word 2387513
.word 8111961
.word 2170005223
.word 1123881663
.word 1223601433
.word 4157383481
.word 6607829
.word 6653329
.word 4768667
.word 5199961
.word 3386515188
.word 3409833957
.word 2443943877
.word 2664982236
.word 7946292
.word 4072478048
.word 1095468
.word 561427818
.word 4874037
.word 2497946047
.word 7814814
.word 169688
.word 2462444
.word 5046034
.word 4005095517
.word 86965173
.word 1262003603
.word 2586094583
.word 3747250
.word 1239911
.word 3195676
.word 1254190
.word 1920467227
.word 635454918
.word 1637785316
.word 642772911
.word 2296099
.word 4541938
.word 2642980
.word 8368000
.word 1176751719
.word 2327745167
.word 1354528380
.word 4288603578
.word 676590
.word 346752664
.word 2071829
.word 1061813248
.word 4361428
.word 2235233715
.word 4213992
.word 4892034
.word 1987814
.word 5183169
.word 2159672702
.word 2507169517
.word 1018755525
.word 2656376329
.word 2998219
.word 8291116
.word 7025525
.word 7070156
.word 1536588520
.word 4249200495
.word 3600584567
.word 3623457973
.word 141835
.word 2513018
.word 613238
.word 6161950
.word 72690498
.word 1287922800
.word 314284737
.word 3158002010
.word 7044481
.word 3610299525
.word 3241972
.word 1661512036
.word 2156050
.word 1104976547
.word 1736313
.word 235407
.word 5130263
.word 3258457
.word 889861155
.word 120646188
.word 2629261981
.word 1669960606
.word 7921677
.word 4040196
.word 2039144
.word 7561656
.word 4059862850
.word 2070602178
.word 1045062172
.word 3875351933
.word 6458423
.word 4908348
.word 6500539
.word 6201452
.word 3309944549
.word 2515530449
.word 3331529017
.word 3178246802
.word 5152541
.word 2640679466
.word 3415069
.word 1750224323
.word 1759347
.word 901666090
.word 5801164
.word 1787943
.word 5989328
.word 6125690
.word 2973099031
.word 916321552
.word 3069533161
.word 3139418744
.word 6757063
.word 6006015
.word 586241
.word 527981
.word 3462997677
.word 3078085256
.word 300448763
.word 270590488
.word 2105286
.word 6346610
.word 7200804
.word 5637006
.word 1078959975
.word 3252640339
.word 3690415129
.word 2888967985
.word 1714295
.word 878576921
.word 7562881
.word 3875979746
.word 4805951
.word 2463051943
.word 3482206
.word 4197502
.word 7080401
.word 6018354
.word 1784632064
.word 2151221570
.word 3628708540
.word 3084408998
.word 6903432
.word 2491325
.word 507927
.word 7655613
.word 3538011852
.word 1276805128
.word 260312805
.word 3923504936
.word 1994046
.word 6987258
.word 7192532
.word 6545891
.word 1021949428
.word 3580972713
.word 3686175726
.word 3354771937
.word 2453983
.word 1257667337
.word 3756790
.word 1925356481
.word 6444618
.word 3302869481
.word 7062739
.word 2461387
.word 3035980
.word 621164
.word 3619656758
.word 1261461890
.word 1555941048
.word 318346816
.word 5346675
.word 2647994
.word 5767564
.word 749577
.word 2740173224
.word 1357098057
.word 2955879016
.word 384158533
.word 8041997
.word 3009748
.word 4148469
.word 4357667
.word 4121526901
.word 1542497137
.word 2126092136
.word 2233306201
.word 1460718
.word 748618600
.word 6663429
.word 3415010212
.word 4430364
.word 2270563444
.word 3901472
.word 7153756
.word 2925816
.word 3374250
.word 1999506068
.word 3666303009
.word 1499481951
.word 1729304568
.word 3980599
.word 6764887
.word 1665318
.word 1163598
.word 2040058690
.word 3467007480
.word 853476187
.word 596344473
.word 2569011
.word 1723229
.word 2028038
.word 5011144
.word 1316619236
.word 883155599
.word 1039370342
.word 2568213443
.word 7737789
.word 3965620171
.word 5483103
.word 2810092632
.word 3192354
.word 1636082790
.word 1356448
.word 5604662
.word 2683270
.word 5601629
.word 695180180
.word 2872391672
.word 1375177022
.word 2870837258
.word 3994671
.word 7009900
.word 3363542
.word 545376
.word 2047270596
.word 3592576747
.word 1723816713
.word 279505433
.word 8368538
.word 3020393
.word 214880
.word 7609976
.word 4288879303
.word 1547952704
.word 110126092
.word 3900115954
.word 4795319
.word 2457603038
.word 556856
.word 285388938
.word 3870317
.word 1983539117
.word 4912752
.word 2312838
.word 7727142
.word 7921254
.word 2517787501
.word 1185330464
.word 3960163579
.word 4059646062
.word 3105558
.word 508145
.word 860144
.word 140244
.word 1591599803
.word 260424530
.word 440824168
.word 71875110
.word 7277073
.word 7826699
.word 3430436
.word 6866265
.word 3729503024
.word 4011186584
.word 1758099917
.word 3518963749
.word 2815639
.word 1443016191
.word 2917338
.word 1495136972
.word 1853806
.word 950076368
.word 348812
.word 8052569
.word 1011223
.word 6026202
.word 178766299
.word 4126945056
.word 518252220
.word 3088431102
.word 6195333
.word 2358373
.word 5365997
.word 2926054
.word 3175110812
.word 1208667171
.word 2750075757
.word 1499603926
.word 3123762
.word 6187330
.word 6663603
.word 7987710
.word 1600929361
.word 3171009271
.word 3415099387
.word 4093704791
.word 2283733
.word 1170414139
.word 3345963
.word 1714807468
.word 1858416
.word 952438995
.word 4561790
.word 6458164
.word 6143691
.word 1744507
.word 2337919326
.word 3309811812
.word 3148644265
.word 894060583
.word 8077412
.word 4405932
.word 1900052
.word 1054478
.word 4139677104
.word 2258042034
.word 973777462
.word 540420426
.word 3531229
.word 4606686
.word 7598542
.word 7648983
.word 1809756372
.word 2360928545
.word 3894256024
.word 3920107058
# 228 "ntt_dilithium_rvv_vlen128_barret_mul.s" 2
.text
    .global ntt_dilithium_1234_5678
    .global _ntt_dilithium_1234_5678
.p2align 4

.globl ntt_rvv_vlen128_barret_mul
ntt_rvv_vlen128_barret_mul:
_ntt_rvv_vlen128_barret_mul:
    push_stack
    vsetivli zero, 4, e32, m1

    li x4, 8380417
    vmv.v.x v19, x4

    .equ L_STRIDE, 64
    .equ S_STRIDE, 16

    la x5, roots

    load_roots_1234 x5, x7, x8, x9, x1, x11, x12, x13, x14, x15, x16, x17, x18, x19, x20, x21, x22, x23, x24, x25, x26, x27, x28, x29, x30, x31



    li x3, 4

    .p2align 2
layer1234_start:



    load16 x10, L_STRIDE
    addi x10, x10, -15*L_STRIDE

    lw x6, 0*8(x5)



    ct_butterfly v0, v8, x6, x7
    ct_butterfly v1, v9, x6, x7
    ct_butterfly v2, v10, x6, x7
    ct_butterfly v3, v11, x6, x7
    ct_butterfly v4, v12, x6, x7
    ct_butterfly v5, v13, x6, x7
    ct_butterfly v6, v14, x6, x7
    ct_butterfly v7, v15, x6, x7

    lw x6, 1*8(x5)


    ct_butterfly v0, v4, x6, x8
    ct_butterfly v1, v5, x6, x8
    ct_butterfly v2, v6, x6, x8
    ct_butterfly v3, v7, x6, x8

    lw x6, 2*8(x5)

    ct_butterfly v8, v12, x6, x9
    ct_butterfly v9, v13, x6, x9
    ct_butterfly v10, v14, x6, x9
    ct_butterfly v11, v15, x6, x9

    lw x6, 3*8(x5)


    ct_butterfly v0, v2, x6, x1
    ct_butterfly v1, v3, x6, x1

    lw x6, 4*8(x5)

    ct_butterfly v4, v6, x6, x11
    ct_butterfly v5, v7, x6, x11
    ct_butterfly v8, v10, x12, x13
    ct_butterfly v9, v11, x12, x13
    ct_butterfly v12, v14, x12, x15
    ct_butterfly v13, v15, x14, x15


    ct_butterfly v0, v1, x16, x17
    ct_butterfly v2, v3, x18, x19
    ct_butterfly v4, v5, x20, x21
    ct_butterfly v6, v7, x22, x23
    ct_butterfly v8, v9, x24, x25
    ct_butterfly v10, v11, x26, x27
    ct_butterfly v12, v13, x28, x29
    ct_butterfly v14, v15, x30, x31


    store16 x10, L_STRIDE
    addi x10, x10, -15*L_STRIDE

    addi x10, x10, S_STRIDE
layer1234_end:
    addi x3, x3, -1
    bnez x3, layer1234_start
# 343 "ntt_dilithium_rvv_vlen128_barret_mul.s"
    addi x10, x10, -4*S_STRIDE

    li x3, 16
    addi x5, x5, 15*8

    .equ L_STRIDE, 16
    .equ S_STRIDE, 64

    .p2align 2
layer5678_start:
    vle32.v v0, (x10)
    addi x10, x10, L_STRIDE
    vle32.v v1, (x10)
    addi x10, x10, L_STRIDE
    vle32.v v2, (x10)
    addi x10, x10, L_STRIDE
    vle32.v v3, (x10)

    addi x10, x10, -3*L_STRIDE

    load_roots_5678 x7, x8, x9, x1, x11, x12, v19, v20, v21, v22, v23, v24, x5




    ct_butterfly v0, v2, x7, x8
    ct_butterfly v1, v3, x7, x8
    ct_butterfly v0, v1, x9, x1
    ct_butterfly v2, v3, x11, x12

    addi sp, sp, -64
    transpose4 v0, sp
    addi sp, sp, 64


    ct_butterfly_v v0, v2, v19, v20
    ct_butterfly_v v1, v3, v19, v20
    ct_butterfly_v v0, v1, v21, v22
    ct_butterfly_v v2, v3, v23, v24


    vsseg4e32.v v0, (x10)
# 398 "ntt_dilithium_rvv_vlen128_barret_mul.s"
    addi x10, x10, S_STRIDE
layer5678_end:
    addi x3, x3, -1
    bnez x3, layer5678_start

    pop_stack
    ret
