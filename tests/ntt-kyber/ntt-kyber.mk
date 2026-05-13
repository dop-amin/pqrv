# Test name - needs to match the directory name
TESTS += ntt-kyber

# All further variables must be prefixed with the capitalized test name

# Platforms this test should run on (matching the directory name in envs/)
NTT_KYBER_PLATFORMS += cross-rvv

# C sources required for this test
NTT_KYBER_SOURCES += main.c

# Assembly sources required for this test
NTT_KYBER_ASM_DIR = ../../asm/manual/ntt_kyber
NTT_KYBER_ASMS += $(NTT_KYBER_ASM_DIR)/ntt_kyber_rvv_vlen128_unfolded.s
NTT_KYBER_ASMS += $(NTT_KYBER_ASM_DIR)/ntt_kyber_rvv_vlen128_unfolded_opt_c908.s
