# Measuring instruction counts on the K230 C908

Record of an investigation into why `perf` reported implausible instruction
counts for SLOTHY-optimized kernels, what the cause turned out to be, and how
the benchmark harness was changed. Dated 2026-08-26.

---

## 1. The problem

`bench_results/ntt-dilithium-test.elf-cross-rv64im.csv` reported the
SLOTHY-optimized Dilithium NTT executing **117 fewer instructions** per call
than the naive version:

```
ntt_8l_rv64im,      17789, 7843, 0.44
ntt_8l_rv64im_opt,   6027, 7726, 1.28
```

This contradicts what SLOTHY does. SLOTHY is an instruction scheduler and
register allocator: it reorders instructions and reassigns registers, but does
no common-subexpression elimination, strength reduction, or dead-code
elimination. Per-iteration instruction count is invariant by construction. The
only mechanisms that can change the count at all are:

- software pipelining, which peels one iteration into a preamble/postamble and
  therefore executes one loop-back branch fewer;
- `unroll`, which changes counter/branch instructions per unit of work;
- `unsafe_address_offset_fixup`, which can fold pointer increments into
  immediate offsets.

None of these can account for 117 instructions. So either the measurement was
wrong, or an assumption about the code was.

---

## 2. What we checked

### 2.1 Hand count of the assembly

Counting `ntt_dilithium_8l_singleissue_plant_rv64im.s` by hand, with macros
expanded, surfaced two errors in the original manual tally:

- **Loop 1 runs 16 times, not 15.** `addi gp, x0, 15` with a bottom test
  (`addi gp, gp, -1; bge gp, zero`) gives pre-body `gp` values 15…0, i.e. 16
  trips. This also matches the algorithm: 16 groups of 16 coefficients for
  layers 1–4. Worth +233 instructions.
- **Loop 2's body is 243 instructions, not 234.** The nine twiddle loads
  (`ld t0..ra`) at the top of the body were missed. They are hoisted into the
  function preamble for loop 1, but loop 2 must reload them each iteration
  because `a1` advances. Worth +144 instructions.

Corrected naive total: 7663, not 7286. The apparent saving was therefore even
larger than first thought, not smaller.

### 2.2 Exact instruction-stream count

Rather than trust a hand count, both variants were assembled with the real
toolchain and their executed instruction streams traced:

```
riscv64-unknown-elf-as -march=rv64im_zicsr -mno-relax -o k.o k.s
qemu-riscv64 -one-insn-per-tb -d exec -D k.trace ./k
```

filtering the trace to the function's PC range. This also revealed that
`li a6, q32` is a pseudo-instruction expanding to three real instructions
(`lui` / `addiw` / `slli`), which the hand count had treated as one.

| kernel | naive | opt | Δ |
| --- | --- | --- | --- |
| `ntt_8l_rv64im` | 7665 | 7663 | −2 |
| `ntt_8l_dual_rv64im` | 7761 | 7761 | 0 |
| `intt_dilithium_8l_plant_rv64im` | 8208 | 8206 | −2 |
| `intt_…_dual` | 8289 | 8289 | 0 |

The −2 is exactly one loop-back branch per software-pipelined loop: SLOTHY
moves the counter decrement into the preamble, so 16 `addi` but only 15
branches execute. The dual-issue variants show **0** because SLOTHY did not
software-pipeline them at all — in
`ntt_dilithium_8l_dualissue_plant_rv64im_opt_c908.s` the loop label sits
immediately after the counter initialisation, with no preamble or postamble.

That is a useful control: it confirms instruction count is preserved except for
the peeled branch.

### 2.3 Ruling out the obvious explanations

- **Different source files.** The `.s` files under `asm/manual/ntt_dilithium/`
  were diffed against the SLOTHY `examples/` originals — byte-identical.
- **Harness overhead.** The deployed ELF was disassembled. GCC had **inlined**
  `ntt_8l_rv64im_wrap`, so the measured inner loop is exactly five
  instructions plus the call:
  `mv a1,s2 ; mv a0,s1 ; addiw s0,s0,-1 ; jal ntt_8l_rv64im ; bnez s0`.
  Nowhere near 117.
- **`exclude_kernel`.** `struct perf_event_attr attr[]` in
  `tests/common/perf.c` is a zero-initialised global with only `.type`,
  `.config` and `.disabled` assigned, so every `exclude_*` bit was 0. Setting
  them was the first hypothesis. It was rebuilt, redeployed, and re-run:
  **the numbers did not move by more than ±1.** Hypothesis wrong, or the flag
  ignored.

### 2.4 The decisive characterisation

With the real per-call harness cost known (5 instructions), the gap fit a
single relation across all eight kernels:

| function | cycles | perf | true | gap | gap/cycles |
| --- | --- | --- | --- | --- | --- |
| `ntt_8l_rv64im` | 17787 | 7843 | 7670 | 173 | 0.973% |
| `ntt_8l_rv64im_opt` | 6025 | 7726 | 7668 | 58 | 0.963% |
| `ntt_8l_dual_rv64im` | 8467 | 7848 | 7766 | 82 | 0.968% |
| `ntt_8l_dual_rv64im_opt` | 6627 | 7830 | 7766 | 64 | 0.966% |
| `intt_…_rv64im` | 15371 | 8362 | 8213 | 149 | 0.969% |
| `intt_…_opt_c908` | 7325 | 8282 | 8211 | 71 | 0.969% |
| `intt_…_dual` | 8695 | 8378 | 8294 | 84 | 0.966% |
| `intt_…_dual_opt_c908` | 6781 | 8360 | 8294 | 66 | 0.973% |

**0.968% of cycles, spread 0.963–0.973%.** A 0.5% relative spread across four
kernels at IPCs from 0.44 to 1.25 is not noise.

### 2.5 A purpose-built probe

`tools/pmu_probe.c` was written to test the mechanism without depending on
QEMU at all. It runs inline-assembly loops whose retired instruction count is
known *by arithmetic* — 6 instructions per iteration, no compiler involved —
in two flavours with **identical instruction counts but very different IPC**:

- `fast_loop`: four independent `addi` + counter + branch → CPI ≈ 0.86
- `slow_loop`: four serially dependent `mul` + counter + branch → CPI ≈ 2.74

Results from the board, at 120,000,000 expected instructions each:

| loop | cycles | error | % of cycles | % of instructions |
| --- | --- | --- | --- | --- |
| fast | 102,365,099 | 928,500 | **0.907%** | 0.774% |
| slow | 327,431,338 | 2,955,239 | **0.903%** | 2.463% |

Four conclusions:

1. **The error scales with cycles, not instructions.** Identical work, 3.2× the
   error, and error-per-cycle matching to within 0.5%.
2. **It is not stalls, replays, or speculative issue.** If the counter were
   counting re-issued instructions, `slow_loop` — stalling 3× harder per
   instruction — would show a higher error *per cycle*. It does not.
3. **`exclude_kernel` is silently ignored.** The probe opened two instruction
   counters differing only in that flag. They agree to within 0.05% on every
   row. `perf_event_open` accepts the flag and the RISC-V PMU driver discards
   it — which is why the fix in §2.3 was inert.
4. **PMU multiplexing is a minor contributor.** 3 events open → 0.905%;
   16 events open → 1.100%. About a fifth of the total.

Separately, the `instret`/`cycle` CSRs read from U-mode return unusable values
(555,871 for a 12,000,000-instruction loop, and wildly inconsistent between
identical runs). T-Head's counters are not exposed to U-mode in any usable
form. That route is dead.

---

## 3. What the problem is

**Periodic kernel work — almost certainly interrupt handling — retires
instructions into the counter at a rate proportional to elapsed time, and it
cannot be filtered out.**

What is proven: the over-count grows with elapsed cycles, is independent of the
instruction mix, is unaffected by `exclude_kernel`, and grows further when more
perf events are opened (which adds PMU rotation work that is unambiguously
kernel-side).

What is inferred from the clean-window statistics in §5.1: an interrupt period
of roughly **110 µs (~8.5 kHz)** at about **1700 instructions per interrupt**,
which multiplies out to the observed ~0.9% of cycles. Note this is far faster
than a 250 Hz scheduler tick, so the source is something else — the exact
device or timer was not identified, and identifying it was not necessary.

### Why it was so hard to spot

The bias is **not noise**. It is a fixed *rate* multiplied by a *duration*, and
the harness averaged over 100,000 iterations:

```
bias per call = rate_per_cycle × cycles_per_call = 0.0097 × 17787 ≈ 173
```

Two consecutive runs of the unmodified binary produced 7843/7843, 7849/7848,
8363/8362, 8379/8378 — reproducible to ±1 instruction out of ~173 of bias. A
stable number looks like a correct measurement. The tell was never run-to-run
variation; it was that the number scaled with *cycles* instead of with
*instructions*.

### Why it corrupts the comparison specifically

A constant offset would cancel in a naive-vs-optimized difference. This one is
proportional to cycles, and the whole point of SLOTHY is that the optimized
kernel takes far fewer cycles for the *same* instructions. So the bias was 173
for the naive kernel and 58 for the optimized one, and the 115-instruction
difference between those two biases was the entire reported "saving".

The **noise floor of this counter is ~170 instructions** on a 7,700-instruction
kernel, and the effect being measured is **2 instructions**. Averaged over one
long window, this counter can never answer the question.

Cycle counts carry the same ~1% inflation, but it is multiplicative and
therefore cancels almost exactly in speedup ratios.

---

## 4. Solutions considered

| option | verdict |
| --- | --- |
| Set `exclude_kernel = 1` | **Tried, inert.** Silently ignored by the PMU driver (§2.5). |
| Isolate a core: `isolcpus` / `nohz_full` / `rcu_nocbs` / `irqaffinity` | **Impossible.** `k230.dtsi` declares only `cpu@0` — Linux runs on a single hart, the other C908 runs RT-Smart. `nohz_full` needs a housekeeping CPU to hand timekeeping to, so the kernel refuses to make the only CPU tickless. |
| Lower `CONFIG_HZ` | Only *shrinks* the bias, needs an SDK kernel rebuild, and the ~8.5 kHz source is not the scheduler tick anyway. |
| Disable interrupts from user space | **Impossible on RISC-V.** Interrupt enable lives in `sstatus.SIE`/`mstatus.MIE`, which are S- and M-mode CSRs; a U-mode write traps. The "N" extension for user-level interrupts was withdrawn from the ISA. |
| Kernel module with `local_irq_save()` | Exact, and the rigorous answer. But interrupts can only be held off for microseconds, so it forces single-call measurement regardless, plus the counter must be read from atomic context. Substantial work. |
| Report statically computed counts only | Always available as a fallback; deterministic and independently verifiable from the `.s` files. Loses hardware confirmation. |
| **Minimum over many short windows** | **Chosen.** See below. |

### Why minimum-of-N works

Interrupts can only ever *add* to a counter; they never subtract. So the
distribution of per-call measurements has a hard floor at the interrupt-free
count, plus a tail from windows that happened to contain an interrupt. **The
minimum over many trials is the interrupt-free measurement.**

One NTT call is ~11 µs. The interrupt period is ~110 µs. So roughly 90% of
single-call windows are already clean — whereas the original 1.1-second window
was guaranteed to contain thousands of interrupts.

---

## 5. What was implemented

### 5.1 Validation first

A minimum-of-N mode was added to `tools/pmu_probe.c` before touching the
harness: 2000 trials per window length, two events only, with an empty-window
floor subtracted. Board results:

```
empty-window floor: instrs min=2144 mean=2178.9 max=12182 (1961/2000 at min)
```

| window | ~duration | expect | min−floor | err | trials@min |
| --- | --- | --- | --- | --- | --- |
| n=3000 | 9.4 µs | 18,000 | 18,009 | **+9** | 1794/2000 |
| n=30000 | 94 µs | 180,000 | 180,009 | **+9** | 397/2000 |
| n=300000 | 955 µs | 1,800,000 | 1,810,362 | +10,362 | 386/2000 |
| n=3000000 | 9.6 ms | 18,000,000 | 18,129,474 | +129,474 | 1/2000 |

The `+9` is **identical** at 9.4 µs and 94 µs despite 10× the work: a fixed
additive constant, not a proportional bias. 1794 of 2000 trials returned that
same exact value, so clean windows are deterministic rather than lucky. The
clean-window rates (90% at 9.4 µs, 20% at 94 µs) are what the ~110 µs interrupt
period in §3 was estimated from. Windows at 1 ms and above are never clean,
which is exactly why the original harness had no chance.

### 5.2 `tests/common/perf.c`

Correctness fixes independent of the main issue:

- `attr.size = sizeof(struct perf_event_attr)`. It was 0; the kernel
  reinterprets that as `PERF_ATTR_SIZE_VER0` via an ABI-compat quirk, silently
  ignoring every field past the first 64 bytes.
- `exclude_kernel` / `exclude_hv` / `exclude_idle` set. Inert on this board,
  but correct, and it lets the harness run on boards with the usual
  `perf_event_paranoid = 2` without root.
- Window bounded by `PERF_EVENT_IOC_RESET`/`ENABLE`/`DISABLE` instead of by a
  pair of `read()` calls, so the 15 setup syscalls no longer sit inside the
  instruction counter's own window. The two reported counters are enabled last
  and disabled first, nesting the instruction window inside the cycle window.
  `val1`/`val2` are gone; the counter value *is* the delta.
- `pin_to_cpu()` via `sched_setaffinity`, overridable with `PQRV_PIN_CPU`.
  `perf_event_open` is called with `cpu = -1` (follow-the-task), so a migration
  mid-measurement would splice counts from two PMUs.
- Accumulator reset in `init_perf_events()`. `stop_and_read_events()`
  accumulates with `+=` and nothing zeroed it, so each row inherited the
  previous row's average. Harmless at 100,000 iterations, wrong at any lower
  count. This also fixed cross-snippet accumulation in `tests/profiling`.
- `count` widened to `uint64_t` (`c908_events.h`). It holds the raw sum before
  `calc_average()` divides; `17789 × 100000 = 1.78e9` against an `INT32_MAX` of
  `2.15e9` left about 20% of headroom.

New minimum-of-N API:

- `perf_set_min_mode(1)` — keep the minimum instead of accumulating.
- `perf_calibrate_floor(trials)` — measure the empty-window cost (2144 here)
  for subtraction.
- `perf_finish_min()` — subtract the floor, and **warn on stderr** if the
  minimum was reached in under 5% of trials. That is the guard against the
  `1/2000` failure mode in §5.1: it means no window was ever clean, or the PMU
  multiplexed and some window under-counted, either of which makes a minimum
  untrustworthy.

### 5.3 `tests/common/c908_events.h`

`NUM_EVENTS` reduced from 16 to **2**. Required for min-mode safety: with more
events open than the PMU has counters, the kernel rotates them, a rotated-off
window under-counts, and a minimum would latch onto that. It also removes the
+0.2 percentage points measured in §2.5.

`INSTR_TOTAL` and `CYCLES_TOTAL` were moved to the **front** of both
`events_list` and `events_counter_list`, and `EV_INSTR`/`EV_CYCLES` fixed at
0/1. Previously the code addressed them as `NUM_EVENTS-2`/`NUM_EVENTS-1`, which
at `NUM_EVENTS 2` would have resolved to the slots labelled
`MULT_INNER_FORWARD`/`ALU_INSTR`.

To collect the raw C908 events again, raise `NUM_EVENTS` back to 16 — but then
use `calc_average()`, not min-mode.

### 5.4 `tests/ntt-dilithium/main.c`

`MAKE_BENCH` converted to one call per window, 2000 trials, keeping the
minimum.

---

## 6. Results

| function | traced | measured | Δ |
| --- | --- | --- | --- |
| `ntt_8l_rv64im` | 7665 | 7668 | +3 |
| `ntt_8l_rv64im_opt` | 7663 | 7666 | +3 |
| `ntt_8l_dual_rv64im` | 7761 | 7764 | +3 |
| `ntt_8l_dual_rv64im_opt` | 7761 | 7764 | +3 |
| `intt_…_rv64im` | 8208 | 8211 | +3 |
| `intt_…_opt_c908` | 8206 | 8209 | +3 |
| `intt_…_dual` | 8289 | 8292 | +3 |
| `intt_…_dual_opt_c908` | 8289 | 8292 | +3 |

**Exactly +3 on all eight** — four kernels, IPCs from 0.44 to 1.25, one uniform
constant. That constant is the call site: the zetas pointer load, `mv a0, src`,
and the `jal`, identical for every variant because every `_wrap` has the same
shape.

Pairwise differences: **2 / 0 / 2 / 0**, exactly as predicted from static
analysis. Two independent methods — hardware PMU and instruction-stream
tracing — now agree to a constant.

**Conclusion: SLOTHY's output executes 2 fewer instructions on the
software-pipelined kernels and exactly the same number on the non-pipelined
ones. The 117 was entirely measurement artifact.** The value of the
optimization is in IPC (0.44 → 1.20 for the single-issue NTT), not in
instruction count — as expected for a scheduler.

### Caveat: the cycles column changed meaning

```
ntt_8l_rv64im       17787 → 17344   −2.5%
ntt_8l_rv64im_opt    6025 →  6378   +5.9%   speedup 2.95 → 2.72
```

This is a **methodology change, not a bug fix**. The old loop measured
*throughput*: 100,000 back-to-back calls, where consecutive invocations overlap
in the pipeline and branch predictors stay hot. Minimum-of-single-call measures
*latency* of one isolated invocation, paying pipeline fill and drain with no
cross-call overlap. High-ILP scheduled code has the most to lose from that,
which is why the optimized variants went *up* while the naive ones went down.

Both figures are honest and they mean different things. For a crypto primitive
the throughput figure is conventional, and the ~1% bias is multiplicative so it
cancels in the ratio — meaning the old cycle measurement was sound.

**Recommended final shape: dual-pass.** Long back-to-back loop for cycles
(throughput, conventional), minimum-of-N for instructions (exact), IPC computed
from the two.

---

## 7. Open items

- Convert `MAKE_BENCH` to dual-pass per §6, restoring throughput cycles while
  keeping exact instruction counts.
- Nine test directories still use `calc_average` and so still carry the ~1%
  bias in their instruction columns: `ntt-kyber`, `dilithium-poly-basemul`,
  `kyber-poly-basemul`, `kyber-poly-reduce`, `kyber-sampling`, `barret-mul`,
  `test`, `profiling`.
- `tests/profiling/profiling.c` brackets *every single call* with
  `start_counting_events`/`stop_and_read_events`. Before the ioctl windowing in
  §5.2 the 15 intra-window `read()` syscalls were charged in full to snippets a
  few instructions long — which is very likely what the unused `offset` fields
  in `c908_events.h` (40 for `INSTR_TOTAL`, 26 for `CYCLES_TOTAL`) and the
  commented-out subtraction in `calc_average()` were empirically compensating
  for. Those can probably now be deleted.
- Duplicate raw event code in `c908_events.h`: `L1_ICACHE_MISS = 0x36` and
  `BRANCH_INSTR = 0x36`. One is wrong; resolving it needs table 14.6 of the
  XuanTie C908 manual.
- The ~8.5 kHz interrupt source was never identified. Not needed for the fix,
  but if it turns out to be something disableable, the bias could be removed
  outright and long-window averaging would become safe again.
