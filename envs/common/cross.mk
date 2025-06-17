# emulation (qemu or empty for native)
EMU = qemu-riscv64
CC = riscv64-unknown-linux-gnu-gcc
SSH_DEST = root@k230-0
SSH_PATH = /home/
SSH_ID = /home/$USER/.ssh/id_ed25519
SSH_CONF = /home/$USER/.ssh/config
include ../common/common.mk