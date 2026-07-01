# Test name - needs to match the directory name
TESTS += kyber-sampling

# All further variables must be prefixed with the capitalized test name

# Platforms this test should run on (matching the directory name in envs/)
KYBER_SAMPLING_PLATFORMS += cross-rvv

# C sources required for this test
KYBER_SAMPLING_SOURCES += main.c

# Assembly sources required for this test
KYBER_SAMPLING_ASM_DIR = ../../asm/manual/kyber_sampling

# --- naive kernels ---
KYBER_SAMPLING_ASMS += $(KYBER_SAMPLING_ASM_DIR)/kyber_cbd2_rvv_vlen128.s
KYBER_SAMPLING_ASMS += $(KYBER_SAMPLING_ASM_DIR)/kyber_cbd3_rvv_vlen128.s
KYBER_SAMPLING_ASMS += $(KYBER_SAMPLING_ASM_DIR)/kyber_rej_uniform_rvv_vlen128.s

# --- SLOTHY-optimized kernels ---
KYBER_SAMPLING_ASMS += $(KYBER_SAMPLING_ASM_DIR)/kyber_cbd2_rvv_vlen128_opt_c908.s
KYBER_SAMPLING_ASMS += $(KYBER_SAMPLING_ASM_DIR)/kyber_cbd3_rvv_vlen128_opt_c908.s
KYBER_SAMPLING_ASMS += $(KYBER_SAMPLING_ASM_DIR)/kyber_rej_uniform_rvv_vlen128_opt_c908.s
