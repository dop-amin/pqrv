LD=$(CC)

COMMON_INC=../common/inc/
ENV_INC=./inc/
TEST_COMMON=../../tests/common/
SRC_DIR=./src
BUILD_DIR=./build/$(TARGET)


CFLAGS += -fpic -Wall -Wextra -Werror -Wshadow -Wno-unused-parameter -fomit-frame-pointer -static
CFLAGS += $(CFLAGS_EXTRA)

CFLAGS+= -Ofast \
	-g \
	-ox \
	-I$(COMMON_INC) \
	-I$(ENV_INC) \
	-I$(SRC_DIR) \
	-I$(TESTDIR) \
	-I$(TEST_COMMON) \


LDFLAGS = -static

CYCLES?=NO # PMU / PERF

ifeq ($(CYCLES),PMU)
	CFLAGS += -DPMU_CYCLES
endif

ifeq ($(CYCLES),PERF)
	CFLAGS += -DPERF_CYCLES
endif

ifeq ($(CYCLES),NO)
	CFLAGS += -DNO_CYCLES
endif

all: $(TARGET)

HAL_SOURCES = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/*/*.c)
OBJECTS_HAL = $(patsubst %.c, $(BUILD_DIR)/%.c.o, $(abspath $(HAL_SOURCES)))
TEST_COMMON_SOURCES = $(wildcard $(TEST_COMMON)/*.c)
OBJECTS_TEST_COMMON = $(patsubst %.c, $(BUILD_DIR)/%.c.o, $(abspath $(TEST_COMMON_SOURCES)))
OBJECTS_SOURCES=$(patsubst %.c, $(BUILD_DIR)/%.c.o, $(abspath $(SOURCES)))
OBJECTS_C = $(OBJECTS_SOURCES) $(OBJECTS_HAL) $(OBJECTS_TEST_COMMON)
OBJECTS_ASM = $(patsubst %.s, $(BUILD_DIR)/%.s.o, $(abspath $(ASMS)))

OBJECTS = $(OBJECTS_C) $(OBJECTS_ASM)

# Compilation
$(OBJECTS_C): $(BUILD_DIR)/%.o: %
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJECTS_ASM): $(BUILD_DIR)/%.o: %
	mkdir -p $(@D)
	$(CC) -x assembler-with-cpp $(CFLAGS) -c -o $@ $<

# Linking
$(TARGET): $(OBJECTS)
	mkdir -p $(@D)
	$(LD) $(LDFLAGS) $(OBJECTS) -o $(TARGET)

.PHONY: build
build: $(TARGET)

# Running
.PHONY: run
run: $(TARGET)
	$(EMU) ./$(TARGET)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
	rm -f *.elf

# Running
# Platform name, derived from the env directory (e.g. envs/cross-rvv -> cross-rvv),
# so results from different platforms no longer overwrite each other.
PLATFORM := $(notdir $(CURDIR))

# Fail fast instead of hanging on an unreachable board; never prompt interactively.
SSH_OPTS = -o BatchMode=yes -o ConnectTimeout=30 -o ControlMaster=auto -o ControlPath=/tmp/pqrv-cm-%r@%h:%p -o ControlPersist=120
SSH_RETRIES     ?= 3
SSH_RETRY_DELAY ?= 3

.PHONY: run-remote
run-remote: $(TARGET)
	mkdir -p ../../bench_results
	@n=0; \
	while :; do \
	  n=$$((n+1)); \
	  if scp -O $(SSH_OPTS) -i $(SSH_ID) -F $(SSH_CONF) ./$(TARGET) $(SSH_DEST):$(SSH_PATH); then break; fi; \
	  if [ $$n -ge $(SSH_RETRIES) ]; then echo "[FAIL] $(PLATFORM)-$(TARGET): scp failed $$n times" >&2; exit 1; fi; \
	  echo "[retry scp $$n/$(SSH_RETRIES)] $(TARGET)-$(PLATFORM) in $(SSH_RETRY_DELAY)s" >&2; \
	  sleep $(SSH_RETRY_DELAY); \
	done
	@out=../../bench_results/$(TARGET)-$(PLATFORM).csv; tmp=$$out.tmp; n=0; \
	while :; do \
	  n=$$((n+1)); \
	  if ssh $(SSH_OPTS) -i $(SSH_ID) -F $(SSH_CONF) $(SSH_DEST) '$(SSH_PATH)/$(TARGET)' > $$tmp && [ -s $$tmp ]; then \
	    mv $$tmp $$out; exit 0; \
	  fi; \
	  rm -f $$tmp; \
	  if [ $$n -ge $(SSH_RETRIES) ]; then echo "[FAIL] $(TARGET)-$(PLATFORM): ssh failed $$n times" >&2; exit 1; fi; \
	  echo "[retry $$n/$(SSH_RETRIES)] $(TARGET)-$(PLATFORM) in $(SSH_RETRY_DELAY)s" >&2; \
	  sleep $(SSH_RETRY_DELAY); \
	done
	#scp -i $(SSH_ID) -F $(SSH_CONF) ./$(TARGET) $(SSH_DEST2):$(SSH_PATH2)
	#ssh -i $(SSH_ID) -F $(SSH_CONF) $(SSH_DEST2) '$(SSH_PATH2)/$(TARGET)'