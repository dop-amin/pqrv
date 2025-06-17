# Test name - needs to match the directory name
TESTS += poly-basemul

# All further variables must be prefixed with the capitalized test name

# Platforms this test should run on (matching the directory name in envs/)
POLY_BASEMUL_PLATFORMS += cross-rvv
POLY_BASEMUL_PLATFORMS += cross-rv64im

# C sources required for this test
POLY_BASEMUL_SOURCES += main.c

# Assembly sources required for this test
POLY_BASEMUL_ASM_DIR = ../../asm/manual/poly_basemul
POLY_BASEMUL_ASMS += $(POLY_BASEMUL_ASM_DIR)/poly_basemul_8l_acc_rv64im.s
POLY_BASEMUL_ASMS += $(POLY_BASEMUL_ASM_DIR)/poly_basemul_8l_acc_rv64im_opt_c908.s
POLY_BASEMUL_ASMS += $(POLY_BASEMUL_ASM_DIR)/poly_basemul_8l_acc_dual_rv64im.s

