# Test name - needs to match the directory name
TESTS += kyber-poly-reduce

# All further variables must be prefixed with the capitalized test name

# Platforms this test should run on (matching the directory name in envs/)
KYBER_POLY_REDUCE_PLATFORMS += cross-rv64im
KYBER_POLY_REDUCE_PLATFORMS += cross-rvv

# C sources required for this test
KYBER_POLY_REDUCE_SOURCES += main.c

# Assembly sources required for this test (shared kyber_basemul asm group)
KYBER_POLY_REDUCE_ASM_DIR = ../../asm/manual/kyber_poly_basemul

# --- RV64IM: Plantard reduction (naive + dual + optimized) ---
KYBER_POLY_REDUCE_ASMS += $(KYBER_POLY_REDUCE_ASM_DIR)/kyber_poly_plantard_rdc_rv64im.s
KYBER_POLY_REDUCE_ASMS += $(KYBER_POLY_REDUCE_ASM_DIR)/kyber_poly_plantard_rdc_rv64im_dual.s
KYBER_POLY_REDUCE_ASMS += $(KYBER_POLY_REDUCE_ASM_DIR)/kyber_poly_plantard_rdc_rv64im_opt_c908.s
KYBER_POLY_REDUCE_ASMS += $(KYBER_POLY_REDUCE_ASM_DIR)/kyber_poly_plantard_rdc_rv64im_dual_opt_c908.s

# --- RV64IM: to-Plantard conversion (naive + dual + optimized) ---
KYBER_POLY_REDUCE_ASMS += $(KYBER_POLY_REDUCE_ASM_DIR)/kyber_poly_toplant_rv64im.s
KYBER_POLY_REDUCE_ASMS += $(KYBER_POLY_REDUCE_ASM_DIR)/kyber_poly_toplant_rv64im_dual.s
KYBER_POLY_REDUCE_ASMS += $(KYBER_POLY_REDUCE_ASM_DIR)/kyber_poly_toplant_rv64im_opt_c908.s
KYBER_POLY_REDUCE_ASMS += $(KYBER_POLY_REDUCE_ASM_DIR)/kyber_poly_toplant_rv64im_dual_opt_c908.s

# --- RVV VLEN=128: Barrett reduction (naive + optimized) ---
KYBER_POLY_REDUCE_ASMS += $(KYBER_POLY_REDUCE_ASM_DIR)/kyber_poly_reduce_rvv_vlen128.s
KYBER_POLY_REDUCE_ASMS += $(KYBER_POLY_REDUCE_ASM_DIR)/kyber_poly_reduce_rvv_vlen128_opt_c908.s

# --- RVV VLEN=128: to-Montgomery conversion (naive + optimized) ---
KYBER_POLY_REDUCE_ASMS += $(KYBER_POLY_REDUCE_ASM_DIR)/kyber_poly_tomont_rvv_vlen128.s
KYBER_POLY_REDUCE_ASMS += $(KYBER_POLY_REDUCE_ASM_DIR)/kyber_poly_tomont_rvv_vlen128_opt_c908.s
