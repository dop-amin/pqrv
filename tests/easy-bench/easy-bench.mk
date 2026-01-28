# Test name - needs to match the directory name
TESTS += easy-bench

# All further variables must be prefixed with the capitalized test name

# Platforms this test should run on (matching the directory name in envs/)
EASY_BENCH_PLATFORMS += cross-rv64im

# C sources required for this test
EASY_BENCH_SOURCES += main.c

# Assembly sources required for this test
EASY_BENCH_ASMS += asm.s