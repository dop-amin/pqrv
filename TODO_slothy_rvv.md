# SLOTHY RISC-V / pqrv sampling+reduce TODOs

Tracking the open items from wiring the Kyber reduction (`kyber-poly-reduce`) and
sampling (`kyber-sampling`) tests, and the SLOTHY RVV codegen bugs found along the way.

Branches: pqrv tests on `add-test-ntt-kyber-rvv-vlen128`; SLOTHY submodule on `riscv-rvv`.

## ✅ Fixed in SLOTHY (submodule `riscv-rvv` @ 4ca4af0)

- [x] **`vsext.vf2` illegal overlap** — `RISCVVectorWidenExtend.make` now sets
  `args_in_out_different=[(0,0)]` so the widening source can't overlap the low part of the
  destination group. Fixed the cbd2/cbd3 "Illegal instruction" trap.
- [x] **Merged branch emission** — `AddiLoop.end()` was missing the space after the mnemonic
  (→ `bnet4`/`bltua0`) and the comma before the label. Fixed. (Cause of the mangled branches
  in `reduce_rvv` and the dual reductions.)

## 🔧 Outstanding SLOTHY fixes

- [ ] **[fix mask] Masked-instruction modeling — `rej_uniform` bug.**
  For general vector ops used with an optional mask (`…, v0.t`) under mask-undisturbed,
  declare **`Vd` as an input (`in_out`)** and **`v0` as an input**. `Vd` is currently modeled
  write-only, so the allocator renames dest≠src and corrupts the inactive lanes
  (`vsrl.vi v22, v9, …, v0.t` instead of the in-place `vsrl.vi vt0, vt0, …`). SLOTHY's own
  `# TODO: declare input register if vm (mask) is used` comments flag exactly this. Affects
  every op that can take `v0.t`, not just `vsrl.vi`.

- [x] **[reg alloc] Loop-counter register clobbered — dual-reduction segfault. (ROOT CAUSE FOUND)**
  The earlier "sw-pipelining trip-count fixup" diagnosis was WRONG. `AddiLoop.start`'s `fixup`
  block works on both the halving and non-halving paths. The real bug: the loop-counter register
  `t4`/`x29` was NOT in `reserved_regs` for the plantard/toplant examples, so SLOTHY reused it as
  a scratch register inside the kernel (e.g. `mulw x29, ...`). Within the extracted body the counter
  looks dead — the `addi t4,t4,-1` / `bne` are the loop boundary, outside the region — so the
  allocator grabbed it. Result: the counter is destroyed mid-iteration, the loop never terminates at
  16, `a0` runs ~255 iterations past the 512-byte buffer → OOB store → SIGSEGV.
  Confirmed under qemu+gdb: crash at `sh …(x10)` with `a0 = base + 0x1FE0` (255*32).
  **Fix:** add `x29` to `reserved_regs` in `RISC_V_poly_plantard_rdc_rv64im.core()` and
  `RISC_V_poly_toplant_rv64im.core()` (mirrors how the basemul examples reserve `loop_control`).
  Verified: regenerated both, x29 no longer written in the body, `kyber-poly-reduce` reports
  "Test Success!" for all four `_opt_c908` variants.
  NOTE: SLOTHY's selfcheck passes even with the clobbered counter (it validates one kernel
  iteration's dataflow, not counter liveness across the loop boundary) — so this class of bug is a
  silent footgun. Consider having SLOTHY auto-reserve the identified loop-counter register.

- [ ] **[symbol emission — verify] Renamed opt-output `.globl`.**
  Generated `_opt_c908.s` had `.globl <naive_symbol>` instead of `<symbol>_opt_c908`, and the
  dual files had a `_opt_c908_dual` vs `_dual_opt_c908` label/`.globl` mismatch — hand-patched
  the `.s`. Confirm SLOTHY's renamed-output emission produces the `_opt_c908` global consistently
  so regeneration doesn't reintroduce it. (Recent regenerations looked correct — may already be
  resolved.)

- [x] **Fix additional comma in emmitted branch instruction loop**
## 📋 Test / repo follow-ups

- [ ] Regenerate the affected opt files with the patched SLOTHY so committed `.s` reflect the
  real fixes rather than hand-patches.
- [ ] `kyber-poly-reduce`: re-enable the two `_dual_opt_c908` tests (currently `#if 0`) after the
  sw-pipelining fix.
- [ ] `kyber-sampling`: re-enable `rej_uniform` equivalence (remove the `-DWITH_OPT_KERNELS` gate)
  after the mask fix. Optionally re-enable cbd2 SW pipelining after the sw-pipelining fix.
- [ ] Cycle benchmarks require the physical C908 board (`make run-remote-…`); qemu can't read
  the PMU (`perf_event_open` → "Function not implemented", counters read 0).

## Current pass/fail

| Test | Status |
|------|--------|
| kyber-poly-reduce: plantard/toplant single opt | ✅ |
| kyber-poly-reduce: `reduce_rvv` / `tomont_rvv` | ✅ |
| kyber-poly-reduce: `_dual_opt_c908` (plantard/toplant) | ✅ (fixed: reserve loop counter x29) |
| kyber-sampling: cbd2, cbd3 | ✅ |
| kyber-sampling: `rej_uniform` | ❌ masked-op modeling (gated) |
