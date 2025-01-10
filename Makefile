# Tests
include tests/helloworld/helloworld.mk
include tests/ntt-dilithium/ntt-dilithium.mk
include tests/profiling/profiling.mk
include tests/uarch-testing/uarch-testing.mk

testname = $(shell echo $(1) | tr '[a-z]' '[A-Z]' | tr '-' '_' | tr '/' '_')
testdir = $(addprefix $(2),tests/$(firstword $(subst /, ,$1))/)
testsources = $(addprefix $(2),$(addprefix $(call testdir,$1,),$($(call testname,$(1))_SOURCES)))
testasms = $(addprefix $(2),$(addprefix $(call testdir,$1,),$($(call testname,$(1))_ASMS)))
testplatforms = $(addsuffix _$(1),$($(call testname,$(1))_PLATFORMS))
testcflags = $($(call testname,$(1))_CFLAGS)
elfname = $(addsuffix -test.elf,$(subst /,-,$1))


platformtests := $(foreach test,$(TESTS), $(call testplatforms,$(test)))
platforms =  $(sort $(foreach test,$(platformtests),$(firstword $(subst _, ,$(test)))))

builds          := $(addprefix build-, $(platformtests))
runs            := $(addprefix run-, $(platformtests))
runsplatform    := $(addprefix runall-, $(platforms))
cleans          := $(addprefix clean-, $(platformtests))
run-remotes     := $(addprefix run-remote-, $(platformtests))

.PHONY: all
all: ${builds}

platform = $(firstword $(subst _, ,$*))
test = $(lastword $(subst _, ,$*))

.PHONY: ${builds}
${builds}: build-%:
	make -j$(shell nproc) -C envs/$(platform) build CFLAGS_EXTRA='$(call testcflags,$(test))' SOURCES='$(call testsources,$(test),../../)' ASMS='$(call testasms,$(test),../../)' TARGET=$(call elfname,$(test)) TESTDIR=$(call testdir,$(test),../../)

.PHONY: ${runs}
${runs}: run-%:
	make -C envs/$(platform) run CFLAGS_EXTRA='$(call testcflags,$(test))' SOURCES='$(call testsources,$(test),../../)' ASMS='$(call testasms,$(test),../../)' TARGET=$(call elfname,$(test)) TESTDIR=$(call testdir,$(test),../../)

.PHONY: run
run: ${runs}

.PHONY:${runsplatform}
${runsplatform}:
	make $(filter run-$(subst runall-,,$@)_%, $(runs))

.PHONY: ${cleans}
${cleans}: clean-%:
	make -C envs/$(platform) clean

.PHONY: run-remote
run-remote: ${run-remotes}

.PHONY: ${run-remotes}
${run-remotes}: run-remote-%:
	make -C envs/$(platform) run-remote CFLAGS_EXTRA='$(call testcflags,$(test))' SOURCES='$(call testsources,$(test),../../)' ASMS='$(call testasms,$(test),../../)' TARGET=$(call elfname,$(test)) TESTDIR=$(call testdir,$(test),../../)

.PHONY: run-remote
run-remote: ${run-remotes}
