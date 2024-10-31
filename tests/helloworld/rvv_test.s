        .text
        .global rvv_test
        .global _rvv_test
rvv_test:
_rvv_test:
        vmv.v.v v0, v28
        ret