# emulation (qemu or empty for native)
EMU = qemu-riscv64
CC = riscv64-unknown-linux-gnu-gcc
SSH_DEST = root@k230-0
SSH_PATH = /home
SSH_DEST2 = justus.bergermann@bpi-f3-0
SSH_PATH2 = /home/justus.bergerman/uarch_benchmarks

SSH_ID = /home/juber/.ssh/id_ed25519
SSH_CONF = /home/juber/.ssh/config
include ../common/common.mk