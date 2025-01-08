# emulation (qemu or empty for native)
EMU = qemu-riscv64
CC = riscv64-unknown-linux-gnu-gcc
SSH_DEST = localadmin@k230-0
SSH_PATH = /home/localadmin
SSH_ID = /home/juber/.ssh/id_ed25519
SSH_CONF = /home/juber/.ssh/config
include ../common/common.mk