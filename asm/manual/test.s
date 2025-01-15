.globl test
test:
    addi x24, x20, 256
    srai x17, x17, 32
    mulh x21, x21, x16
    addi x19, x17, 256
    sub x15, x23, x21
    mulh x25, x19, x16
    ret