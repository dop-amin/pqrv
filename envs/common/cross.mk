# emulation (qemu or empty for native)
EMU = qemu-riscv64
CC = riscv64-unknown-linux-gnu-gcc
REMOTE_USER = remote_user_here
_USER = local_user_here
SSH_DEST = root@k230-0
SSH_PATH = /home
SSH_DEST2 = $(REMOTE_USER)@bpi-f3-0
SSH_PATH2 = /home/$(REMOTE_USER)/uarch_benchmarks

SSH_ID = /home/$(_USER)/.ssh/id_ed25519
SSH_CONF = /home/$(_USER)/.ssh/config
include ../common/common.mk