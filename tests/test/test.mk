# Test name - needs to match the directory name
TESTS += test

# All further variables must be prefixed with the capitalized test name

# Platforms this test should run on (matching the directory name in envs/)
TEST_PLATFORMS += cross-rvv
TEST_PLATFORMS += cross-rv64im

# C sources required for this test
TEST_SOURCES += main.c
TEST_SOURCES += perf.c

# Assembly sources required for this test
TEST_ASM_DIR = ../../asm/manual/
TEST_ASMS += $(TEST_ASM_DIR)/test.s


