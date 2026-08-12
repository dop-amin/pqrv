# SLOTHY RISC-V / pqrv sampling+reduce+NTT TODOs

Tracking the open items from wiring the Kyber reduction (`kyber-poly-reduce`),
sampling (`kyber-sampling`) and NTT/INTT (`ntt-kyber`) tests, and the SLOTHY RVV
codegen bugs found along the way. Cycle numbers and per-example status live in
`progress.md`.

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

- [ ] **[stale output] The two dual-issue *forward* NTT `_opt_c908` files are miscompiled.**
  `ntt_kyber_dualissue_plant_rv64im_opt_c908.s` and
  `ntt_kyber_dualissue_l32_plant_rv64im_opt_c908.s` predate the current SLOTHY and were not
  part of the recent re-optimization round. They run to completion but disagree with their
  own naive kernel in **112 of 256** coefficients — identically for both, which is expected
  since they are the l=16/l=32 twins of the same source. The loop-boundary emission looks
  correct (memory-resident counter, one decrement in the preamble + one in the body, matching
  the dual INTT file that passes), so this is a scheduling/allocation defect, not a counter
  bug. **Action:** re-run `RISC_V_ntt_dualissue_plant_rv64im` and
  `RISC_V_ntt_dualissue_l32_plant_rv64im`; the tests are wired and gated behind
  `WITH_NTT_DUAL_OPT` in `tests/ntt-kyber/main.c`.


- [ ] **[missing table] PQRV ships no RV64 l=32 twiddle table.**
  `Kyber/RV64/ntt.c` only defines the `uint32_t zetas_ntt_rv64im[128]` (l=16). The l=32 table
  in `tests/ntt-kyber/pqrv_paper.h` is derived from it: recover `b16 = entry * q mod 2^32`
  (verified `|b16| <= q` for all 128 entries), rescale `b32 = b16 * 2^32 mod± q`, re-multiply
  by `q^-1 mod 2^64`. Fine for naive-vs-opt equivalence (both sides read the same table), but
  see the next item before treating it as a real Kyber NTT table.

- [ ] **[open question] The single-issue and dual-issue forward NTTs do not share a table.**
  Fed the same `zetas_ntt_rv64im`, `ntt_rv64im` (single) and `ntt_dual_rv64im` (dual) produce
  different results even mod q. They consume the table differently — the single-issue kernel
  reads 8-byte packed pairs (`ld`, 16 uint32 per iteration), the dual-issue one reads single
  `lw`s (7 uint32 per iteration) — so PQRV's one table cannot be the right layout for both.
  Worth establishing which variant `zetas_ntt_rv64im` belongs to (check what PQRV's RV64
  Kyber build actually links) before adding any algorithmic reference test for the scalar
  forward NTT. Does not affect the equivalence tests.

## 📋 Test / repo follow-ups

- [x] Regenerate the affected opt files with the patched SLOTHY so committed `.s` reflect the
  real fixes rather than hand-patches.
- [x] `kyber-poly-reduce`: re-enable the two `_dual_opt_c908` tests (currently `#if 0`) after the
  sw-pipelining fix.
- [ ] `kyber-sampling`: re-enable `rej_uniform` equivalence (remove the `-DWITH_OPT_KERNELS` gate)
  after the mask fix. Optionally re-enable cbd2 SW pipelining after the sw-pipelining fix.
- [ ] Cycle benchmarks require the physical C908 board (`make run-remote-…`); qemu can't read
  the PMU (`perf_event_open` → "Function not implemented", counters read 0).
- [ ] `ntt-kyber`: re-enable the two order-conversion `_opt_c908` variants once the two loop
  bugs above are fixed and the files are regenerated.
- [ ] `ntt-kyber`: wire the remaining forward-NTT examples
  (`ntt_kyber_singleissue_plant_rv64im`, `ntt_kyber_dualissue_plant_rv64im`,
  `ntt_kyber_dualissue_l32_plant_rv64im`) after the dual-symbol rename above. That also
  unlocks a real algorithmic reference for the *scalar* INTT (forward-then-inverse round trip);
  today the scalar INTT is only checked naive-vs-opt.
- [x] `asm/manual/*`: audited every symlink. `ntt_kyber_rvv_vlen128_unfolded.s` pointed at a
  file deleted upstream (`dd6253d`), which silently broke the `ntt-kyber` build — repointed to
  the live `ntt_kyber_rvv_vlen128` pair. `asm/manual/ntt_dilithium/*_unfolded*.s` resolve fine.
- [ ] **`asm/manual/kyber_poly_basemul`: 28 dangling symlinks** — all the `*_rv64im*` entries are
  *absolute* paths into a foreign home (`/home/amin.abdulrahman/git/public/pqrv/slothy/…`), so
  `kyber-poly-basemul` cannot build on any other machine. The target files all exist in the
  submodule; the fix is to re-create them as relative links
  (`../../../slothy/examples/naive|opt/riscv/kyber_basemul/<file>.s`), like every other group.
  Left untouched here to keep this change scoped to `ntt-kyber`.

## Current pass/fail

| Test | Status |
|------|--------|
| kyber-poly-reduce: plantard/toplant single opt | ✅ |
| kyber-poly-reduce: `reduce_rvv` / `tomont_rvv` | ✅ |
| kyber-poly-reduce: `_dual_opt_c908` (plantard/toplant) | ✅ (fixed: reserve loop counter x29) |
| kyber-sampling: cbd2, cbd3 | ✅ |
| kyber-sampling: `rej_uniform` | ❌ masked-op modeling (gated) |
| ntt-kyber: `intt_rv64im` single + dual `_opt_c908` | ✅ |
| ntt-kyber: `ntt_rvv_vlen128` / `intt_rvv_vlen128` `_opt_c908` | ✅ |
| ntt-kyber: `normal2ntt_order` / `ntt2normal_order` `_opt_c908` | ✅ (fixed: reserve loop counter x12, reorder counter update above the branch, regenerate) |
| ntt-kyber: `ntt_rv64im` single `_opt_c908` | ✅ (after hand-fixing the stale mangled branches) |
| ntt-kyber: `ntt_dual_rv64im` / `ntt_dual_l32_rv64im` `_opt_c908` | ❌ stale output, 112/256 coefficients wrong (gated on `WITH_NTT_DUAL_OPT`) |
