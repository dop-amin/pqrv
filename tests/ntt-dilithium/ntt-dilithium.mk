# Test name - needs to match the directory name
TESTS += ntt-dilithium

# All further variables must be prefixed with the capitalized test name

# Platforms this test should run on (matching the directory name in envs/)
NTT_DILITHIUM_PLATFORMS += cross-rvv
NTT_DILITHIUM_PLATFORMS += cross-rv64im

# C sources required for this test
NTT_DILITHIUM_SOURCES += main.c

# Assembly sources required for this test
NTT_DILITHIUM_ASM_DIR = ../../asm/manual/ntt_dilithium
NTT_DILITHIUM_ASMS += $(NTT_DILITHIUM_ASM_DIR)/ntt_8l_singleissue_plant_rv64im.S
NTT_DILITHIUM_ASMS += $(NTT_DILITHIUM_ASM_DIR)/ntt_8l_dualissue_plant_rv64im.S


