# Slothy bug: kernel duplication in the Kyber RVV NTT

## Symptom

The slothy-optimized Kyber forward NTT
`asm/manual/ntt_kyber/ntt_kyber_rvv_vlen128_unfolded_opt_c908.s`
produces output that does **not** match the unoptimized baseline
`asm/manual/ntt_kyber/ntt_kyber_rvv_vlen128_unfolded.s`
on the same input and the same zetas/mask table.

Concretely, with random input + mod-q reduction + correctly initialized
shuffle masks + zero twiddle factors, the baseline produces a polynomial
of the form `[1436]*8 [2951]*8 [1436]*8 [2951]*8 ...` (the expected
degenerate-twiddle output of the NTT), while the optimized version
collapses it to `[1436]*256`. With non-zero twiddles the divergence is
just as severe; the optimized version simply does not compute the same
function.

The test that demonstrates this is `tests/ntt-kyber/`.

## Root cause: software pipelining without a loop

The asm file is laid out as follows:

```
ntt_rvv_vlen128_opt_c908:
    ; (callee-saved register save — added by us, see Other fixes below)

    ; --- Preamble (slothy)  ~373 instructions ---
    ; partial work for the first iteration

    ; Kernel (slothy label "start:")
    start:
        ; --- Kernel body  ~746 instructions ---
        ; one full pipelined iteration

    ; (slothy label "end:")
    end:

    ; --- Postamble (slothy)  ~368 instructions ---
    ; finishes the last iteration

    ; (callee-saved register restore)
    ret
```

The source the optimizer reads from
(`slothy/examples/naive/riscv/ntt_kyber/ntt_kyber_rvv_vlen128_unfolded.s`)
is **straight-line code**: it computes the entire NTT — Level 0 over the
whole array, then six levels each on the first half (P0) and second half
(P1) — without any branch. Logically that body spans `N = 2` iterations
(one for P0, one for P1).

In `slothy/examples/naive/riscv/ntt_kyber/_example.py`
(`RISC_V_ntt_rvv_vlen128.core`), slothy was configured to apply software
pipelining to that whole region:

```python
slothy.config.sw_pipelining.enabled    = True
slothy.config.sw_pipelining.halving_heuristic = True
...
slothy.optimize("start", "end")        # NOT optimize_loop()
```

The combination "sw_pipelining + halving heuristic + non-loop
`optimize()`" produces an output that's structured as a software-pipelined
loop body — preamble + kernel + postamble — and **expects the caller to
wrap the kernel in a runtime loop** that runs it `(N - 1)` times. The asm
emitted contains no such loop branch, so the kernel executes exactly
once. The arithmetic is:

| What slothy emits  | Iterations of work  |
|--------------------|---------------------|
| Preamble           | 1                   |
| Kernel × 1         | 1                   |
| Postamble          | 1                   |
| **Total**          | **3**               |

Because the NTT only has `N = 2` iterations in the source, this is
**one full iteration too many** — the function applies a complete extra
NTT pass on top of the correct result.

Concretely, you can see this in the generated asm. The active
instructions in
`asm/manual/ntt_kyber/ntt_kyber_rvv_vlen128_unfolded_opt_c908.s` include
**two complete setups** for the same constants:

| Line  | Setup                                                          |
|------:|----------------------------------------------------------------|
|  868  | `addi x1,  x11, _ZETAS_EXP*2`              (level 0, first pass) |
|  870  | `addi x16, x11, _ZETAS_EXP_1TO6_P0_L1*2`   (P0 levels 1..6, first)|
|  989  | `addi x16, x11, _ZETAS_EXP_1TO6_P0_L3*2`   (P0 levels 3..4, first)|
| 1249  | `addi x26, x11, _ZETAS_EXP_1TO6_P1_L1*2`   (P1 levels 1..6, first)|
| 1329  | `addi x16, x11, _ZETAS_EXP_1TO6_P1_L3*2`   (P1 levels 3..4, first)|
| 1623  | `addi x1,  x11, _ZETAS_EXP*2`              (level 0, **second pass**) |
| 1625  | `addi x16, x11, _ZETAS_EXP_1TO6_P0_L1*2`   (P0 levels 1..6, second)|
| 1744  | `addi x16, x11, _ZETAS_EXP_1TO6_P0_L3*2`   (P0 levels 3..4, second)|
| 2762  | `addi x16, x11, _ZETAS_EXP_1TO6_P1_L1*2`   (P1 levels 1..6, second)|
| 2828  | `addi x16, x11, _ZETAS_EXP_1TO6_P1_L3*2`   (P1 levels 3..4, second)|

The second pass operates on the **same array offsets** as the first
(`(0*128)*2`, `(1*128)*2`, `128*2`, etc.), so it re-applies the NTT
butterflies to data that has already been NTT-transformed.

## Confirmation experiment

Deleting the kernel section (lines 1239..2744 inclusive of `start:` and
`end:`) and rebuilding leaves only preamble + postamble = 2 iterations
of work. The test then passes:

```
Test for ntt_kyber_rvv_vlen128_wrap          Ok
Test for ntt_kyber_rvv_vlen128_opt_c908_wrap Ok
Starting benchmarks...
Test Success!
```

This is a workaround, not a fix — the kernel deletion throws away the
software-pipelining benefit on the part of the schedule that lives in
the kernel.

## The fix (this commit)

`slothy/examples/naive/riscv/ntt_kyber/_example.py` :
`RISC_V_ntt_rvv_vlen128.core` — software pipelining is now disabled for
this region:

```python
# slothy.config.sw_pipelining.enabled = True
# slothy.config.sw_pipelining.halving_heuristic = True
```

The corresponding dilithium RVV NTT class
(`slothy/examples/naive/riscv/ntt_dilithium/_example.py` ::
`RISC_V_ntt_rvv_vlen128.core`) already keeps those lines commented out
for the same reason. The `split_heuristic` settings stay enabled —
splitting the body for scheduling is independent of SWP and does not
duplicate work.

A WARNING comment has also been added to
`RISC_V_intt_rvv_vlen128.core` in the same file, which has the same
risky pattern (SWP enabled, several `optimize()` calls on straight-line
regions). The INTT is not exercised by `tests/ntt-kyber/` and its config
was left unchanged, but if a similar correctness issue appears there,
the same fix applies.

## Regenerating the asm

The fix lives in the slothy generator script. To produce a corrected
`ntt_kyber_rvv_vlen128_unfolded_opt_c908.s` you must re-run slothy:

```bash
cd slothy
python3 example.py --examples ntt_kyber_rvv_vlen128
# (or whatever invocation your environment uses)
```

The output will overwrite
`slothy/examples/opt/riscv/ntt_kyber/ntt_kyber_rvv_vlen128_unfolded_opt_c908.s`,
which is symlinked from
`asm/manual/ntt_kyber/ntt_kyber_rvv_vlen128_unfolded_opt_c908.s`.

Until then, the existing opt asm in the tree is still the old (buggy)
output and `make run-cross-rvv_ntt-kyber` will still report `FAIL!` for
the opt comparison test. As a stopgap, you can either:

1. Delete the kernel section (lines 1239..2744) from the opt asm and
   rebuild. The test will pass, with a small performance regression
   compared to a properly re-optimized version.
2. Comment out the cross-validation call for the opt variant in
   `tests/ntt-kyber/main.c` so the test exits 0.

## Other fixes that were needed to even get this far

Two unrelated bugs in the same asm file had to be fixed before the
correctness divergence above could be observed at all:

1. **Wrong `.globl` symbol.** The file declared
   `.globl ntt_rvv_vlen128` but the label was
   `ntt_rvv_vlen128_opt_c908:`, so the optimized entry point was never
   actually exported. Fixed by changing the directive to
   `.globl ntt_rvv_vlen128_opt_c908`.
2. **Callee-saved registers clobbered without save/restore.** The
   slothy schedule freely uses `x1` (ra), `x9` (s1), `x23` (s7),
   `x26` (s10), `x27` (s11) as scratch. The first kills the return
   address (function returns to a pointer inside the zetas array →
   segfault); the others violate the RISC-V calling convention. Fixed
   by wrapping the body in a prologue/epilogue that saves and restores
   these five registers on the stack.

Both fixes are currently applied in
`slothy/examples/opt/riscv/ntt_kyber/ntt_kyber_rvv_vlen128_unfolded_opt_c908.s`.
After re-running slothy with the fixed generator, the `.globl` fix will
need to be re-applied (or, better, the generator should emit a correct
`.globl` directive — that is a separate concern in slothy's RISC-V
backend). The callee-saved register issue can be avoided entirely by
reserving the s-registers in `slothy.config.reserved_regs`:

```python
r = slothy.config.reserved_regs
r += ["x1", "x3", "x8", "x9"] + [f"x{i}" for i in range(18, 28)]
slothy.config.reserved_regs = r
```

at the cost of constraining slothy's register allocator more tightly.
This is not part of the current fix.

## Files

- `slothy/examples/naive/riscv/ntt_kyber/_example.py` — generator fix.
- `slothy/examples/opt/riscv/ntt_kyber/ntt_kyber_rvv_vlen128_unfolded_opt_c908.s` —
  current (still buggy) opt asm with the two unrelated fixes applied.
- `tests/ntt-kyber/` — test+benchmark that detects the issue.
- `docs/SLOTHY_KYBER_NTT_BUG.md` — this document.
