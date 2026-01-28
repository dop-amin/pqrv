# Test name - needs to match the directory name
TESTS += kyber-poly-basemul

# All further variables must be prefixed with the capitalized test name

# Platforms this test should run on (matching the directory name in envs/)
KYBER_POLY_BASEMUL_PLATFORMS += cross-rv64im
KYBER_POLY_BASEMUL_PLATFORMS += cross-rvv

# C sources required for this test
KYBER_POLY_BASEMUL_SOURCES += main.c

# Assembly sources required for this test
KYBER_POLY_BASEMUL_ASM_DIR = ../../asm/manual/kyber_poly_basemul
KYBER_POLY_BASEMUL_ASMS += $(KYBER_POLY_BASEMUL_ASM_DIR)/kyber_poly_basemul_acc_cache_end_rv64im_dual.s
KYBER_POLY_BASEMUL_ASMS += $(KYBER_POLY_BASEMUL_ASM_DIR)/kyber_poly_basemul_acc_cache_end_rv64im.s
KYBER_POLY_BASEMUL_ASMS += $(KYBER_POLY_BASEMUL_ASM_DIR)/kyber_poly_basemul_acc_end_rv64im_dual.s
KYBER_POLY_BASEMUL_ASMS += $(KYBER_POLY_BASEMUL_ASM_DIR)/kyber_poly_basemul_acc_cache_init_rv64im.s
KYBER_POLY_BASEMUL_ASMS += $(KYBER_POLY_BASEMUL_ASM_DIR)/kyber_poly_basemul_acc_cached_rv64im_dual.s
KYBER_POLY_BASEMUL_ASMS += $(KYBER_POLY_BASEMUL_ASM_DIR)/kyber_poly_basemul_cache_init_rv64im.s
KYBER_POLY_BASEMUL_ASMS += $(KYBER_POLY_BASEMUL_ASM_DIR)/kyber_poly_basemul_acc_rv64im.s
KYBER_POLY_BASEMUL_ASMS += $(KYBER_POLY_BASEMUL_ASM_DIR)/kyber_poly_basemul_acc_cache_init_end_rv64im_dual.s
KYBER_POLY_BASEMUL_ASMS += $(KYBER_POLY_BASEMUL_ASM_DIR)/kyber_poly_basemul_acc_rv64im_dual.s
KYBER_POLY_BASEMUL_ASMS += $(KYBER_POLY_BASEMUL_ASM_DIR)/kyber_poly_basemul_acc_cache_init_rv64im_dual.s
KYBER_POLY_BASEMUL_ASMS += $(KYBER_POLY_BASEMUL_ASM_DIR)/kyber_poly_basemul_cache_init_rv64im_dual.s
KYBER_POLY_BASEMUL_ASMS += $(KYBER_POLY_BASEMUL_ASM_DIR)/kyber_poly_basemul_acc_end_rv64im.s
KYBER_POLY_BASEMUL_ASMS += $(KYBER_POLY_BASEMUL_ASM_DIR)/kyber_poly_basemul_acc_cache_init_end_rv64im.s
KYBER_POLY_BASEMUL_ASMS += $(KYBER_POLY_BASEMUL_ASM_DIR)/kyber_poly_basemul_acc_cached_rv64im.s

# Optimized assembly sources
KYBER_POLY_BASEMUL_ASMS += $(KYBER_POLY_BASEMUL_ASM_DIR)/kyber_poly_basemul_acc_rv64im_opt_c908.s
KYBER_POLY_BASEMUL_ASMS += $(KYBER_POLY_BASEMUL_ASM_DIR)/kyber_poly_basemul_acc_rv64im_dual_opt_c908.s
KYBER_POLY_BASEMUL_ASMS += $(KYBER_POLY_BASEMUL_ASM_DIR)/kyber_poly_basemul_acc_end_rv64im_opt_c908.s
KYBER_POLY_BASEMUL_ASMS += $(KYBER_POLY_BASEMUL_ASM_DIR)/kyber_poly_basemul_acc_end_rv64im_dual_opt_c908.s
KYBER_POLY_BASEMUL_ASMS += $(KYBER_POLY_BASEMUL_ASM_DIR)/kyber_poly_basemul_cache_init_rv64im_opt_c908.s
KYBER_POLY_BASEMUL_ASMS += $(KYBER_POLY_BASEMUL_ASM_DIR)/kyber_poly_basemul_cache_init_rv64im_dual_opt_c908.s
KYBER_POLY_BASEMUL_ASMS += $(KYBER_POLY_BASEMUL_ASM_DIR)/kyber_poly_basemul_acc_cached_rv64im_opt_c908.s
KYBER_POLY_BASEMUL_ASMS += $(KYBER_POLY_BASEMUL_ASM_DIR)/kyber_poly_basemul_acc_cached_rv64im_dual_opt_c908.s
KYBER_POLY_BASEMUL_ASMS += $(KYBER_POLY_BASEMUL_ASM_DIR)/kyber_poly_basemul_acc_cache_end_rv64im_opt_c908.s
KYBER_POLY_BASEMUL_ASMS += $(KYBER_POLY_BASEMUL_ASM_DIR)/kyber_poly_basemul_acc_cache_end_rv64im_dual_opt_c908.s
KYBER_POLY_BASEMUL_ASMS += $(KYBER_POLY_BASEMUL_ASM_DIR)/kyber_poly_basemul_acc_cache_init_rv64im_opt_c908.s
KYBER_POLY_BASEMUL_ASMS += $(KYBER_POLY_BASEMUL_ASM_DIR)/kyber_poly_basemul_acc_cache_init_rv64im_dual_opt_c908.s
KYBER_POLY_BASEMUL_ASMS += $(KYBER_POLY_BASEMUL_ASM_DIR)/kyber_poly_basemul_acc_cache_init_end_rv64im_opt_c908.s
KYBER_POLY_BASEMUL_ASMS += $(KYBER_POLY_BASEMUL_ASM_DIR)/kyber_poly_basemul_acc_cache_init_end_rv64im_dual_opt_c908.s

# RVV assembly sources (temporarily commented out due to assembly syntax issues)
KYBER_POLY_BASEMUL_ASMS += $(KYBER_POLY_BASEMUL_ASM_DIR)/kyber_poly_basemul_rvv_vlen128.s
KYBER_POLY_BASEMUL_ASMS += $(KYBER_POLY_BASEMUL_ASM_DIR)/kyber_poly_basemul_acc_rvv_vlen128.s
KYBER_POLY_BASEMUL_ASMS += $(KYBER_POLY_BASEMUL_ASM_DIR)/kyber_poly_basemul_cached_rvv_vlen128.s
KYBER_POLY_BASEMUL_ASMS += $(KYBER_POLY_BASEMUL_ASM_DIR)/kyber_poly_basemul_acc_cached_rvv_vlen128.s
KYBER_POLY_BASEMUL_ASMS += $(KYBER_POLY_BASEMUL_ASM_DIR)/kyber_poly_basemul_cache_init_rvv_vlen128.s
KYBER_POLY_BASEMUL_ASMS += $(KYBER_POLY_BASEMUL_ASM_DIR)/kyber_poly_basemul_acc_cache_init_rvv_vlen128.s

# RVV optimized assembly sources
KYBER_POLY_BASEMUL_ASMS += $(KYBER_POLY_BASEMUL_ASM_DIR)/kyber_poly_basemul_rvv_vlen128_opt_c908.s
KYBER_POLY_BASEMUL_ASMS += $(KYBER_POLY_BASEMUL_ASM_DIR)/kyber_poly_basemul_acc_rvv_vlen128_opt_c908.s
KYBER_POLY_BASEMUL_ASMS += $(KYBER_POLY_BASEMUL_ASM_DIR)/kyber_poly_basemul_cached_rvv_vlen128_opt_c908.s
KYBER_POLY_BASEMUL_ASMS += $(KYBER_POLY_BASEMUL_ASM_DIR)/kyber_poly_basemul_acc_cached_rvv_vlen128_opt_c908.s
KYBER_POLY_BASEMUL_ASMS += $(KYBER_POLY_BASEMUL_ASM_DIR)/kyber_poly_basemul_cache_init_rvv_vlen128_opt_c908.s
KYBER_POLY_BASEMUL_ASMS += $(KYBER_POLY_BASEMUL_ASM_DIR)/kyber_poly_basemul_acc_cache_init_rvv_vlen128_opt_c908.s
