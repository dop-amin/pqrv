#!/usr/bin/env python3
"""Markdown report of naive vs. SLOTHY-optimized kernels from the benchmark CSVs.

Layout: summary per category (scalar single-issue, scalar dual-issue, RVV), then one section per
kernel group (Dilithium NTT/INTT, ...) with the scalar kernels from the cross-rv64im CSV and the
vector kernels from the cross-rvv CSV, then the kernels whose optimized version is slower.
As in plot_bench.py, an optimized row (name containing "_opt_c908" or ending in "_opt") is paired
with its baseline, the same name without that suffix.

Examples:
  tools/report_bench.py                                 # bench_results/*.csv -> bench_results/report.md
  tools/report_bench.py --out report.md
  tools/report_bench.py --out -                         # to stdout
"""
import argparse
import csv
import glob
import os
import re
import statistics

OPT_RE = re.compile(r"_opt_c908|_opt$")
DEFAULT_EXCLUDE = ("mul_timing", "mul-timing", "test-test")
# benchmark (CSV name without -test-<platform>) -> section title; listed order = report order
GROUPS = {"ntt-dilithium": "Dilithium — NTT / INTT",
          "dilithium-poly-basemul": "Dilithium — Poly Basemul & Reduce",
          "ntt-kyber": "Kyber — NTT / INTT",
          "kyber-poly-basemul": "Kyber — Poly Basemul",
          "kyber-poly-reduce": "Kyber — Poly Reduce / Domain Conversion",
          "kyber-sampling": "Kyber — Sampling"}
SCALAR, VECTOR = "cross-rv64im", "cross-rvv"
CATEGORIES = ("Scalar, single-issue", "Scalar, dual-issue", "RVV (VLEN=128)")

HEADER = """\
# SLOTHY Optimization Results — PQRV on XuanTie C908

Measured on the K230 board (XuanTie C908, RVV 1.0, VLEN=128) with the hardware performance
counters, read from user mode (`csrr`). **Cycles**: C908 raw event `0x2D` (U-mode cycles) per
call. **Instr.**: minimum over all windows per call. **Clean**: windows that reached that minimum.
**Faster by** = (naive cycles / optimized cycles − 1) × 100, against the baseline of the same
variant (dual-issue against dual-issue); `—` marks a baseline. Scalar kernels from the
`cross-rv64im` build, vector kernels from `cross-rvv`. Raw data: `bench_results/*.csv`.
"""


def read_csv(path):
    """([row dicts], failed): rows of a pqrv bench CSV; failed if the test printed FAIL."""
    rows, failed = [], False
    with open(path) as f:
        for rec in csv.reader(f):
            rec = [c.strip() for c in rec]
            if rec and "FAIL" in rec[0]:
                failed = True
            if len(rec) >= 4 and rec[1].isdigit():
                rows.append(dict(name=rec[0], cycles=int(rec[1]), instr=int(rec[2]), ipc=rec[3],
                                 clean=rec[5] if len(rec) > 5 else ""))
    return rows, failed


def split_name(path):
    """(benchmark, platform) from <benchmark>-test-<platform>.csv."""
    stem = os.path.splitext(os.path.basename(path))[0]
    m = re.match(r"(.*)-test-(cross-\w+)$", stem)
    return (m.group(1), m.group(2)) if m else (stem, "")


def is_vector(name):
    return "rvv" in name


def category(name):
    return CATEGORIES[2] if is_vector(name) else CATEGORIES[1] if "dual" in name else CATEGORIES[0]


def faster_by(naive, opt):
    """Speed change in percent: (naive / opt - 1) * 100."""
    return (naive / opt - 1) * 100


def fmt_pct(p, bold_from=100):
    s = f"{p:+.0f}%".replace("-", "−")
    return f"⚠️ {s}" if p < 0 else f"**{s}**" if p >= bold_from else s


def table(header, rows, right):
    """Markdown table with padded columns; right = indices of right-aligned columns."""
    w = [max(len(str(r[i])) for r in [header] + rows) for i in range(len(header))]
    line = lambda r: "| " + " | ".join(str(c).rjust(w[i]) if i in right else str(c).ljust(w[i])
                                       for i, c in enumerate(r)) + " |"
    sep = "| " + " | ".join("-" * (w[i] - 1) + ":" if i in right else "-" * w[i]
                            for i in range(len(header))) + " |"
    return [line(header), sep] + [line(r) for r in rows]


def kernel_rows(rows):
    """Table rows in CSV order (baseline '—', optimized with its speed change) and the pairs."""
    by_name = {r["name"]: r for r in rows}
    out, pairs = [], []
    for r in rows:
        base = OPT_RE.sub("", r["name"])
        if OPT_RE.search(r["name"]) and base in by_name:
            p = faster_by(by_name[base]["cycles"], r["cycles"])
            pairs.append((by_name[base], r, p))
            fb = fmt_pct(p)
        else:
            fb = "—"
        out.append([f"`{r['name']}`", r["cycles"], r["instr"], r["ipc"], r["clean"], fb])
    return out, pairs


def report(files):
    data = {}                       # benchmark -> platform -> rows
    failed = []
    for path in files:
        bench, platform = split_name(path)
        rows, fail = read_csv(path)
        if fail:
            failed.append((os.path.basename(path), len(rows)))
        data.setdefault(bench, {})[platform] = rows

    sections, all_pairs = [], []
    for bench in list(GROUPS) + sorted(set(data) - set(GROUPS)):
        if bench not in data:
            continue
        plats = data[bench]
        scalar_src = SCALAR if SCALAR in plats else VECTOR
        parts = [("Scalar (`cross-rv64im`)" if scalar_src == SCALAR else f"Scalar (`{scalar_src}`)",
                  [r for r in plats.get(scalar_src, []) if not is_vector(r["name"])]),
                 ("Vector, VLEN=128 (`cross-rvv`)", [r for r in plats.get(VECTOR, []) if is_vector(r["name"])])]
        sec = [f"### {GROUPS.get(bench, bench)}", ""]
        for label, rows in parts:
            if not rows:
                continue
            trows, pairs = kernel_rows(rows)
            all_pairs += pairs
            sec += [f"**{label}**", ""]
            sec += table(["Function", "Cycles", "Instr.", "IPC", "Clean", "Faster by"], trows, {1, 2, 3, 4, 5})
            sec += [""]
        if len(sec) > 2:
            sections += sec

    out = [HEADER]
    if failed:
        out += ["> ⚠️ **Test failures** (`FAIL!` in the CSV, numbers of these suites are not valid):"]
        out += [f"> - `{f}`" + (" (no benchmark rows)" if not n else "") for f, n in failed]
        out += [""]
    summary = []
    for cat in CATEGORIES:
        p = [x for b, o, x in all_pairs if category(o["name"]) == cat]
        if p:
            summary.append([cat, len(p), f"**{fmt_pct(statistics.median(p), 1e9)}**",
                            fmt_pct(max(p), 1e9), fmt_pct(min(p), 1e9)])
    out += ["## Summary", ""]
    out += table(["Category", "Kernels", "Median", "Best", "Worst"], summary, {1, 2, 3, 4}) + [""]
    out += ["## Results by kernel group", ""] + sections

    slower = sorted(((o, b, o["cycles"] / b["cycles"] - 1) for b, o, _ in all_pairs if o["cycles"] > b["cycles"]),
                    key=lambda t: -t[2])
    out += ["## Regressions", ""]
    if slower:
        out += table(["Kernel", "Naive cycles", "Optimized cycles", "Extra cycles"],
                     [[f"`{o['name']}`", b["cycles"], o["cycles"], f"**+{x * 100:.0f}%**"] for o, b, x in slower],
                     {1, 2, 3})
    else:
        out += ["None: every optimized kernel is at least as fast as its baseline."]
    return "\n".join(out) + "\n"


def main():
    here = os.path.dirname(os.path.abspath(__file__))
    default_dir = os.path.join(here, "..", "bench_results")
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("csv", nargs="*", help="CSV files (default: all in bench_results/)")
    ap.add_argument("--out", help="output file, - for stdout (default: bench_results/report.md)")
    ap.add_argument("--exclude", nargs="*", default=list(DEFAULT_EXCLUDE),
                    help="skip CSV files whose name contains one of these (default: %(default)s)")
    args = ap.parse_args()

    files = args.csv or sorted(glob.glob(os.path.join(default_dir, "*.csv")))
    files = [f for f in files if not any(x in os.path.basename(f) for x in args.exclude)]
    if not files:
        raise SystemExit("no CSV files found")
    md = report(files)
    out = args.out or os.path.join(default_dir, "report.md")
    if out == "-":
        print(md, end="")
    else:
        with open(out, "w") as f:
            f.write(md)
        print(f"{os.path.relpath(out)}  ({len(files)} CSV files)")


if __name__ == "__main__":
    main()
