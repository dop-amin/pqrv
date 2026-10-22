#!/usr/bin/env python3
"""Turn the mul-timing board output into per-instruction cycle tables.

Usage: ./mul-timing-test.elf > mul_timing.csv ; python3 analyze.py mul_timing.csv

Throughput (tp): (cycles - baseline) / 96 per instruction.
Latency (lat):   per chain step minus the add reference (1 cycle), i.e. the extra
                 cycles of the multiply over a 1-cycle ALU op, plus 1.
"""
import sys
from collections import defaultdict

N_TP, N_LAT = 96, 48
rows = {}
for line in open(sys.argv[1]):
    parts = [p.strip() for p in line.split(",")]
    if len(parts) >= 2 and parts[1].isdigit():
        rows[parts[0]] = int(parts[1])

base = {"scalar": rows["base_scalar"]}
for sew in (16, 32, 64):
    base[f"e{sew}"] = rows[f"base_vec_e{sew}"]
lat_add_step = (rows["lat_add_rs1_full"] - base["scalar"]) / N_LAT

tables = defaultdict(dict)  # (kind, instr) -> {value: {pos: cycles}}
for name, cyc in rows.items():
    kind = name.split("_")[0]
    if kind not in ("tp", "lat") or name.startswith("lat_add"):
        continue
    parts = name.split("_")
    if parts[1].startswith("v"):  # tp_vmul_vv_e32_vs2_d8
        instr, sew, pos, val = f"{parts[1]}.{parts[2]} {parts[3]}", parts[3], parts[4], "_".join(parts[5:])
        per = (cyc - base[sew]) / N_TP
    else:                         # tp_mul_rs1_d8 / lat_mul_rs1_d8
        instr, pos, val = parts[1], parts[2], "_".join(parts[3:])
        if kind == "tp":
            per = (cyc - base["scalar"]) / N_TP
        else:
            per = (cyc - base["scalar"]) / N_LAT - lat_add_step + 1
    tables[(kind, instr)].setdefault(val, {})[pos] = per

for (kind, instr), vals in sorted(tables.items()):
    poss = sorted({p for v in vals.values() for p in v})
    print(f"\n## {kind} {instr}   (cycles per instruction)")
    print("| value | " + " | ".join(poss) + " |")
    print("|---|" + "--:|" * len(poss))
    for val, d in vals.items():
        print(f"| {val} | " + " | ".join(f"{d.get(p, float('nan')):.2f}" for p in poss) + " |")
