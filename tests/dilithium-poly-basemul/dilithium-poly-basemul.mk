# Test name - needs to match the directory name
TESTS += dilithium-poly-basemul

# All further variables must be prefixed with the capitalized test name

# Platforms this test should run on (matching the directory name in envs/)
DILITHIUM_POLY_BASEMUL_PLATFORMS += cross-rvv
DILITHIUM_POLY_BASEMUL_PLATFORMS += cross-rv64im

# C sources required for this test
DILITHIUM_POLY_BASEMUL_SOURCES += main.c

# Assembly sources required for this test
DILITHIUM_POLY_BASEMUL_ASM_DIR = ../../asm/manual/dilithium_poly_basemul
DILITHIUM_POLY_BASEMUL_ASMS += $(DILITHIUM_POLY_BASEMUL_ASM_DIR)/dilithium_poly_basemul_8l_acc_end_rv64im_dual_opt_c908.s
DILITHIUM_POLY_BASEMUL_ASMS += $(DILITHIUM_POLY_BASEMUL_ASM_DIR)/dilithium_poly_basemul_8l_acc_end_rv64im_dual.s
DILITHIUM_POLY_BASEMUL_ASMS += $(DILITHIUM_POLY_BASEMUL_ASM_DIR)/dilithium_poly_basemul_8l_acc_end_rv64im_opt_c908.s
DILITHIUM_POLY_BASEMUL_ASMS += $(DILITHIUM_POLY_BASEMUL_ASM_DIR)/dilithium_poly_basemul_8l_acc_end_rv64im.s
DILITHIUM_POLY_BASEMUL_ASMS += $(DILITHIUM_POLY_BASEMUL_ASM_DIR)/dilithium_poly_basemul_8l_acc_rv64im_dual_opt_c908.s
DILITHIUM_POLY_BASEMUL_ASMS += $(DILITHIUM_POLY_BASEMUL_ASM_DIR)/dilithium_poly_basemul_8l_acc_rv64im_dual.s
DILITHIUM_POLY_BASEMUL_ASMS += $(DILITHIUM_POLY_BASEMUL_ASM_DIR)/dilithium_poly_basemul_8l_acc_rv64im_opt_c908.s
DILITHIUM_POLY_BASEMUL_ASMS += $(DILITHIUM_POLY_BASEMUL_ASM_DIR)/dilithium_poly_basemul_8l_acc_rv64im.s
DILITHIUM_POLY_BASEMUL_ASMS += $(DILITHIUM_POLY_BASEMUL_ASM_DIR)/dilithium_poly_basemul_8l_init_rv64im_dual_opt_c908.s
DILITHIUM_POLY_BASEMUL_ASMS += $(DILITHIUM_POLY_BASEMUL_ASM_DIR)/dilithium_poly_basemul_8l_init_rv64im_dual.s
DILITHIUM_POLY_BASEMUL_ASMS += $(DILITHIUM_POLY_BASEMUL_ASM_DIR)/dilithium_poly_basemul_8l_init_rv64im_opt_c908.s
DILITHIUM_POLY_BASEMUL_ASMS += $(DILITHIUM_POLY_BASEMUL_ASM_DIR)/dilithium_poly_basemul_8l_init_rv64im.s
DILITHIUM_POLY_BASEMUL_ASMS += $(DILITHIUM_POLY_BASEMUL_ASM_DIR)/dilithium_poly_basemul_8l_rv64im_dual_opt_c908.s
DILITHIUM_POLY_BASEMUL_ASMS += $(DILITHIUM_POLY_BASEMUL_ASM_DIR)/dilithium_poly_basemul_8l_rv64im_dual.s
DILITHIUM_POLY_BASEMUL_ASMS += $(DILITHIUM_POLY_BASEMUL_ASM_DIR)/dilithium_poly_basemul_8l_rv64im_opt_c908.s
DILITHIUM_POLY_BASEMUL_ASMS += $(DILITHIUM_POLY_BASEMUL_ASM_DIR)/dilithium_poly_basemul_8l_rv64im.s
DILITHIUM_POLY_BASEMUL_ASMS += $(DILITHIUM_POLY_BASEMUL_ASM_DIR)/dilithium_poly_reduce_rv64im_dual_opt_c908.s
DILITHIUM_POLY_BASEMUL_ASMS += $(DILITHIUM_POLY_BASEMUL_ASM_DIR)/dilithium_poly_reduce_rv64im_dual.s
DILITHIUM_POLY_BASEMUL_ASMS += $(DILITHIUM_POLY_BASEMUL_ASM_DIR)/dilithium_poly_reduce_rv64im_opt_c908.s
DILITHIUM_POLY_BASEMUL_ASMS += $(DILITHIUM_POLY_BASEMUL_ASM_DIR)/dilithium_poly_reduce_rv64im.s

