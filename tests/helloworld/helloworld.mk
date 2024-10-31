# Test name - needs to match the directory name
TESTS += helloworld

# All further variables must be prefixed with the capitalized test name

# Platforms this test should run on (matching the directory name in envs/)
HELLOWORLD_PLATFORMS += cross-rvv

# C sources required for this test
HELLOWORLD_SOURCES += main.c

# Assembly sources required for this test
HELLOWORLD_ASMS += rvv_test.s

