# Run every *.elf in this directory and store its output in <name>.csv.
# Copy this file next to the .elf files on the board and run `make`.
#
#   make            disable IRQs, run all benchmarks, re-enable IRQs
#   make foo.csv    run a single benchmark (IRQs untouched)
#   make clean      remove all generated .csv files

ELFS := $(wildcard *.elf)
CSVS := $(ELFS:.elf=.csv)

# Benchmarks must never run concurrently, even with -j.
.NOTPARALLEL:
.PHONY: all run clean FORCE

# enable_irq.sh runs even if a benchmark fails; make's exit status is kept.
all:
	@chmod +x disable_irq.sh enable_irq.sh
	./disable_irq.sh
	@$(MAKE) --no-print-directory run; status=$$?; \
	./enable_irq.sh; \
	exit $$status

run: $(CSVS)

%.csv: %.elf FORCE
	@echo "### $<"
	@chmod +x $<
	./$< | tee $@

FORCE:

clean:
	rm -f $(CSVS)
