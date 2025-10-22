# Test name - needs to match the directory name
TESTS += barret-mul

# All further variables must be prefixed with the capitalized test name

# Platforms this test should run on (matching the directory name in envs/)
BARRET_MUL_PLATFORMS += cross-rvv
BARRET_MUL_PLATFORMS += cross-rv64im

# C sources required for this test
BARRET_MUL_SOURCES += main.c
BARRET_MUL_SOURCES += barret-mul-ref.c

# Assembly sources required for this test
BARRET_MUL_ASM_DIR = ../../asm/manual/
BARRET_MUL_ASMS += $(BARRET_MUL_ASM_DIR)/barret_mul/barret_mul_scalar.s


