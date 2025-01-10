# Test name - needs to match the directory name
TESTS += uarch-testing

# All further variables must be prefixed with the capitalized test name

# Platforms this test should run on (matching the directory name in envs/)
UARCH_TESTING_PLATFORMS += cross-rvv
UARCH_TESTING_PLATFORMS += cross-rv64im

# C sources required for this test
UARCH_TESTING_SOURCES += main.c

# Assembly sources required for this test
UARCH_TESTING_ASM_DIR = ../../asm/manual/
UARCH_TESTING_ASMS += $(UARCH_TESTING_ASM_DIR)/riscv_test.s
UARCH_TESTING_ASMS += $(UARCH_TESTING_ASM_DIR)/riscv_test_opt_c908.s

