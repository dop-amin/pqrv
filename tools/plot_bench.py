#!/usr/bin/env python3
"""Bar charts of naive vs. SLOTHY-optimized cycles, one chart per benchmark CSV.

Each optimized row (name containing "_opt_c908" or ending in "_opt") is paired with its
baseline, i.e. the same name without that suffix. Rows without a partner are skipped.

Examples:
  tools/plot_bench.py                                   # all CSVs in bench_results/ -> bench_results/plots/
  tools/plot_bench.py bench_results/ntt-kyber-test-cross-rvv.csv --format svg
  tools/plot_bench.py --metric speedup --out /tmp/plots
  tools/plot_bench.py --only rvv                        # only kernels whose name contains "rvv"
"""
import argparse
import csv
import glob
import os
import re

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402
import matplotlib.ticker  # noqa: E402

# Reference palette (dataviz skill), light mode: categorical slots 1 and 2 + text/surface tokens
NAIVE, OPT = "#2a78d6", "#eb6834"
TEXT, TEXT_2, GRID, SURFACE = "#0b0b0b", "#52514e", "#e4e3df", "#fcfcfb"
OPT_RE = re.compile(r"_opt_c908|_opt$")
DEFAULT_EXCLUDE = ("mul_timing", "mul-timing", "test-test")


def read_csv(path):
    """Return [(name, cycles, instructions)] from a pqrv bench CSV."""
    rows = []
    with open(path) as f:
        for rec in csv.reader(f):
            rec = [c.strip() for c in rec]
            if len(rec) >= 3 and rec[1].isdigit():
                rows.append((rec[0], int(rec[1]), int(rec[2])))
    return rows


def pair(rows, only=None):
    """[(label, naive_cycles, opt_cycles)] in CSV order."""
    by_name = {n: c for n, c, _ in rows}
    pairs = []
    for name, cyc, _ in rows:
        if not OPT_RE.search(name):
            continue
        base = OPT_RE.sub("", name)
        if base in by_name and (only is None or only in name):
            pairs.append((base, by_name[base], cyc))
    return pairs


def title_of(path):
    stem = os.path.splitext(os.path.basename(path))[0]
    m = re.match(r"(.*)-test-(cross-\w+)$", stem)
    return f"{m.group(1)}  ({m.group(2)})" if m else stem


def plot(path, pairs, out_path, metric, log):
    n = len(pairs)
    labels = [p[0] for p in pairs]
    fig_h = 1.3 + (0.55 if metric == "cycles" else 0.32) * n
    fig, ax = plt.subplots(figsize=(10, fig_h), facecolor=SURFACE)
    ax.set_facecolor(SURFACE)
    y = list(range(n))[::-1]  # first CSV row on top

    if metric == "cycles":
        h = 0.3
        bar = dict(height=h, edgecolor=SURFACE, linewidth=1)  # surface gap between touching bars
        ax.barh([v + h / 2 for v in y], [p[1] for p in pairs], color=NAIVE, label="naive", **bar)
        ax.barh([v - h / 2 for v in y], [p[2] for p in pairs], color=OPT, label="optimized (SLOTHY)", **bar)
        xmax = max(max(p[1], p[2]) for p in pairs)
        for v, (_, naive, opt) in zip(y, pairs):
            gain = naive / opt
            txt = f"{opt:,}  ×{gain:.2f}" + ("  slower" if gain < 1 else "")
            ax.text(opt, v - h / 2, "  " + txt, va="center", fontsize=8, color=TEXT,
                    fontweight="bold" if gain < 1 else "normal")
            ax.text(naive, v + h / 2, f"  {naive:,}", va="center", fontsize=8, color=TEXT_2)
        ax.set_xlabel("cycles per call (lower is better)", color=TEXT_2)
        if log:
            ax.set_xscale("log")
        else:
            ax.set_xlim(0, xmax * 1.22)
        ax.legend(loc="lower right", frameon=False, fontsize=9, labelcolor=TEXT)
    else:
        gains = [p[1] / p[2] for p in pairs]
        ax.barh(y, gains, height=0.5, color=OPT)
        ax.axvline(1.0, color=TEXT_2, linewidth=1)
        for v, g in zip(y, gains):
            # labels of slower kernels go right of the 1.0 line so the line never crosses them
            ax.text(max(g, 1.0), v, f"  ×{g:.2f}" + ("  slower" if g < 1 else ""), va="center", fontsize=8, color=TEXT,
                    fontweight="bold" if g < 1 else "normal")
        ax.set_xlim(0, max(gains) * 1.18)
        ax.set_xlabel("speedup (naive cycles / optimized cycles, higher is better; 1 = no change)", color=TEXT_2)

    ax.set_yticks(y)
    ax.set_yticklabels(labels, fontsize=8, color=TEXT)
    ax.set_title(title_of(path), loc="left", fontsize=11, color=TEXT, fontweight="bold")
    ax.grid(axis="x", color=GRID, linewidth=0.8)
    ax.set_axisbelow(True)
    for side in ("top", "right", "left"):
        ax.spines[side].set_visible(False)
    ax.spines["bottom"].set_color(GRID)
    ax.tick_params(axis="x", colors=TEXT_2, labelsize=8)
    if metric == "cycles" and not log:
        ax.xaxis.set_major_formatter(matplotlib.ticker.FuncFormatter(lambda x, _: f"{x:,.0f}"))
    ax.tick_params(axis="y", length=0)
    fig.tight_layout()
    fig.savefig(out_path, dpi=150, facecolor=SURFACE)
    plt.close(fig)


def main():
    here = os.path.dirname(os.path.abspath(__file__))
    default_dir = os.path.join(here, "..", "bench_results")
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("csv", nargs="*", help="CSV files (default: all in bench_results/)")
    ap.add_argument("--out", help="output directory (default: <csv dir>/plots)")
    ap.add_argument("--format", default="png", choices=("png", "svg", "pdf"))
    ap.add_argument("--metric", default="cycles", choices=("cycles", "speedup"),
                    help="cycles: naive vs optimized bars; speedup: one bar per kernel")
    ap.add_argument("--log", action="store_true", help="log x-axis (cycles metric only)")
    ap.add_argument("--only", help="only kernels whose name contains this substring")
    ap.add_argument("--exclude", nargs="*", default=list(DEFAULT_EXCLUDE),
                    help="skip CSV files whose name contains one of these (default: %(default)s)")
    args = ap.parse_args()

    files = args.csv or sorted(glob.glob(os.path.join(default_dir, "*.csv")))
    files = [f for f in files if not any(x in os.path.basename(f) for x in args.exclude)]
    for path in files:
        pairs = pair(read_csv(path), args.only)
        if not pairs:
            print(f"skip {path}: no naive/optimized pairs")
            continue
        out_dir = args.out or os.path.join(os.path.dirname(os.path.abspath(path)), "plots")
        os.makedirs(out_dir, exist_ok=True)
        stem = os.path.splitext(os.path.basename(path))[0]
        suffix = "" if args.metric == "cycles" else "_speedup"
        out_path = os.path.join(out_dir, f"{stem}{suffix}.{args.format}")
        plot(path, pairs, out_path, args.metric, args.log)
        print(f"{out_path}  ({len(pairs)} kernels)")


if __name__ == "__main__":
    main()
