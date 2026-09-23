/* pmu_probe.c -- characterise the PMU on the K230 C908 Linux core.
 *
 * Three independent tests:
 *
 *   1. exclude_kernel      Does perf actually filter kernel-mode instructions?
 *                          Runs the two settings in SEPARATE passes -- never two
 *                          PERF_COUNT_HW_INSTRUCTIONS events at once. There is
 *                          only one instret counter (a fixed CSR), so two such
 *                          events time-share it and each returns an unscaled
 *                          fraction; an earlier version of this probe did that
 *                          and the readings summed to ~100% of the truth.
 *
 *   2. multiplexing        With one instructions event plus N filler events,
 *                          does anything get rotated off? Answers whether
 *                          NUM_EVENTS can safely exceed 2 in tests/common.
 *
 *   3. min-of-N            Is the minimum over many short windows the
 *                          interrupt-free count, and up to what window length?
 *
 * All cycle measurements use C908 raw event 0x2D (U Mode Cycles), not
 * PERF_COUNT_HW_CPU_CYCLES -- see the comment on C908_U_MODE_CYCLES below.
 *
 * Every read requests time_enabled/time_running, so multiplexing is reported
 * rather than silently halving a result.
 *
 * Ground truth is arithmetic, not measurement: the two loops are inline asm
 * with exactly 6 retired instructions per iteration.
 *
 * Build:
 *   riscv64-unknown-linux-gnu-gcc -O2 -static -march=rv64imdv -mabi=lp64d \
 *       -o pmu_probe tools/pmu_probe.c
 * Run as root (or perf_event_paranoid <= 2).
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <sched.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <linux/perf_event.h>

#define MAX_EV 20
#define TRIALS 2000

/* C908 raw event 0x2D: cycles spent in U-mode only.
 *
 * Used instead of PERF_COUNT_HW_CPU_CYCLES everywhere in this probe, because
 * exclude_kernel does nothing on this core (see test 1) while this event
 * filters in hardware. Verified against the total: M+S+U summed to the
 * unfiltered cycle count to within 0.016%, and U-mode came out at 17% of the
 * total on a syscall-heavy workload rather than ~100%.
 *
 * Requires the widened riscv,raw-event-to-mhpmcounters property in k230.dtsi;
 * with the stock device tree only codes 0x01-0x2A are reachable and this one
 * comes back <not counted>.
 *
 * Caveat: this excludes the interrupt handler's own cycles, but not the cache,
 * TLB and branch-predictor damage it leaves behind, which is paid afterwards
 * in U-mode and therefore still counted. Min-of-N (test 3) discards the whole
 * polluted window and removes that too. */
#define C908_U_MODE_CYCLES 0x2D

static long perf_open(struct perf_event_attr *a) {
  return syscall(__NR_perf_event_open, a, 0, -1, -1, 0UL);
}

/* ---- loops with an exactly known retired-instruction count -------------- */

/* 6 instructions/iteration, independent adds -> high IPC */
static void __attribute__((noinline)) fast_loop(uint64_t n) {
  __asm__ volatile(
      "1:\n\t"
      "addi t0, t0, 1\n\t" "addi t1, t1, 1\n\t"
      "addi t2, t2, 1\n\t" "addi t3, t3, 1\n\t"
      "addi %0, %0, -1\n\t"
      "bnez %0, 1b\n\t"
      : "+r"(n) : : "t0", "t1", "t2", "t3");
}

/* 6 instructions/iteration, serial multiply chain -> low IPC. Same instruction
 * count as fast_loop, ~3x the cycles: separates an error that tracks cycles
 * from one that tracks instructions. */
static void __attribute__((noinline)) slow_loop(uint64_t n) {
  uint64_t x = 3;
  __asm__ volatile(
      "1:\n\t"
      "mul  %1, %1, %1\n\t" "mul  %1, %1, %1\n\t"
      "mul  %1, %1, %1\n\t" "mul  %1, %1, %1\n\t"
      "addi %0, %0, -1\n\t"
      "bnez %0, 1b\n\t"
      : "+r"(n), "+r"(x) : : );
}

/* ---- counter plumbing --------------------------------------------------- */

struct rd { uint64_t val, ena, run; };

static int open_ev(uint32_t type, uint64_t config, int excl_kernel) {
  struct perf_event_attr a;
  memset(&a, 0, sizeof(a));
  a.size           = sizeof(a);
  a.type           = type;
  a.config         = config;
  a.disabled       = 1;
  a.exclude_kernel = excl_kernel ? 1 : 0;
  a.exclude_hv     = 1;
  /* report whether the event was ever rotated off the hardware */
  a.read_format    = PERF_FORMAT_TOTAL_TIME_ENABLED | PERF_FORMAT_TOTAL_TIME_RUNNING;
  int fd = (int) perf_open(&a);
  if (fd < 0) { fprintf(stderr, "perf_event_open failed: %s\n", strerror(errno)); }
  return fd;
}

static int read_ev(int fd, struct rd *r) {
  uint64_t buf[3] = {0, 0, 0};
  r->val = r->ena = r->run = 0;
  if (fd < 0) { return -1; }
  if (read(fd, buf, sizeof(buf)) != (ssize_t) sizeof(buf)) { return -1; }
  r->val = buf[0]; r->ena = buf[1]; r->run = buf[2];
  return 0;
}

/* "ok", or how badly the event was multiplexed */
static const char *mux_note(const struct rd *r) {
  static char s[64];
  if (r->ena == 0) { return "no time"; }
  if (r->run == r->ena) { return "not multiplexed"; }
  snprintf(s, sizeof(s), "MULTIPLEXED: on hw %.1f%% of the time",
           100.0 * (double) r->run / (double) r->ena);
  return s;
}

/* ---- test 1: does exclude_kernel filter? -------------------------------- */

static void test_exclude_kernel(uint64_t n) {
  printf("=== 1. exclude_kernel (one instructions event per pass) ===\n");
  printf("    loop  excl_kernel     expect        value       err err/u-cyc   status\n");

  for (int ek = 0; ek <= 1; ek++) {
    for (int slow = 0; slow <= 1; slow++) {
      /* instructions is a fixed counter and u-cycles a programmable one, so
       * not contend with each other */
      int fi = open_ev(PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS, ek);
      int fc = open_ev(PERF_TYPE_RAW, C908_U_MODE_CYCLES, ek);

      ioctl(fc, PERF_EVENT_IOC_RESET, 0); ioctl(fi, PERF_EVENT_IOC_RESET, 0);
      ioctl(fc, PERF_EVENT_IOC_ENABLE, 0); ioctl(fi, PERF_EVENT_IOC_ENABLE, 0);
      if (slow) { slow_loop(n); } else { fast_loop(n); }
      ioctl(fi, PERF_EVENT_IOC_DISABLE, 0); ioctl(fc, PERF_EVENT_IOC_DISABLE, 0);

      struct rd ri, rc;
      read_ev(fi, &ri); read_ev(fc, &rc);
      uint64_t expect = 6u * n;
      long long err = (long long) ri.val - (long long) expect;

      printf("    %-5s %6d %13llu %12llu %+10lld %+8.4f%%   %s\n",
             slow ? "slow" : "fast", ek,
             (unsigned long long) expect, (unsigned long long) ri.val, err,
             rc.val ? 100.0 * (double) err / (double) rc.val : 0.0,
             mux_note(&ri));

      close(fi); close(fc);
    }
  }
  printf("\n    err ~ 0 for excl_kernel=1 but > 0 for =0  -> the flag works\n");
  printf("    err identical for both                    -> the flag is ignored\n\n");
}

/* ---- test 2: how many events before multiplexing kicks in? -------------- */

static void test_multiplex(uint64_t n) {
  static const uint64_t filler[] = {0x1D, 0x30, 0x32, 0x36, 0x0B, 0x16, 0x31,
                                    0x33, 0x3d, 0x3e, 0x3f, 0x40, 0x41};
  const int nfill_max = (int) (sizeof(filler) / sizeof(filler[0]));
  int fd[MAX_EV];

  printf("=== 2. multiplexing with one instructions event + N raw fillers ===\n");
  printf("    total_evs     expect        value       err   status\n");

  for (int nf = 0; nf <= nfill_max; nf += nfill_max) {
    int k = 0;
    fd[k++] = open_ev(PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS, 1);
    fd[k++] = open_ev(PERF_TYPE_RAW, C908_U_MODE_CYCLES, 1);
    for (int i = 0; i < nf && k < MAX_EV; i++) {
      fd[k++] = open_ev(PERF_TYPE_RAW, filler[i], 1);
    }

    for (int i = k - 1; i >= 0; i--) { ioctl(fd[i], PERF_EVENT_IOC_RESET, 0); }
    for (int i = k - 1; i >= 0; i--) { ioctl(fd[i], PERF_EVENT_IOC_ENABLE, 0); }
    fast_loop(n);
    for (int i = 0; i < k; i++) { ioctl(fd[i], PERF_EVENT_IOC_DISABLE, 0); }

    struct rd ri;
    read_ev(fd[0], &ri);
    uint64_t expect = 6u * n;
    printf("    %9d %10llu %12llu %+10lld   %s\n", k,
           (unsigned long long) expect, (unsigned long long) ri.val,
           (long long) ri.val - (long long) expect, mux_note(&ri));

    for (int i = 0; i < k; i++) { close(fd[i]); }
  }
  printf("\n    if 'not multiplexed' at the larger count, NUM_EVENTS may exceed 2\n\n");
}

/* ---- test 2b: which raw C908 event codes actually allocate? -------------
 * Table 14.6 of the C908 manual lists event indices 0x01..0xB4, but the manual
 * also says "a counter corresponds to an event, which cannot be modified" --
 * the counter<->event binding is wired in silicon, and riscv-pmu-sbi does not
 * know that mapping. Empirically some codes allocate and some do not (the
 * per-mode cycle counters 0x2B/0x2C/0x2D come back <not counted>), so sweep
 * every code, one event at a time to avoid contention, and report which ones
 * the SBI path can actually reach. */
static const char *raw_name(unsigned code) {
  switch (code) {
  case 0x01: return "L1-icache Access";
  case 0x02: return "L1-icache Miss";
  case 0x0B: return "Store Instruction";
  case 0x16: return "Issue Instruction";
  case 0x1D: return "ALU Instruction";
  case 0x27: return "Stalled Cycles Frontend";
  case 0x28: return "Stalled Cycles Backend";
  case 0x2B: return "M Mode Cycles";
  case 0x2C: return "S Mode Cycles";
  case 0x2D: return "U Mode Cycles";
  case 0x30: return "LOAD Instruction";
  case 0x31: return "Fused Instruction";
  case 0x32: return "MULT Instruction";
  case 0x33: return "DIV Instruction";
  case 0x36: return "Branch Instruction";
  case 0x9B: return "L1 Dcache Access";
  case 0xA5: return "L2 Access";
  default:   return NULL;
  }
}

static void test_raw_sweep(uint64_t n) {
  int n_ok = 0, n_fail = 0;

  printf("=== 2b. raw C908 event sweep (0x01-0xB4), one event at a time ===\n");
  printf("    workload: fast_loop(%llu) = %llu instructions\n\n",
         (unsigned long long) n, (unsigned long long) (6 * n));
  printf("    code  value          status        name\n");

  for (unsigned code = 0x01; code <= 0xB4; code++) {
    int fd = open_ev(PERF_TYPE_RAW, code, 0);
    if (fd < 0) { n_fail++; continue; }

    ioctl(fd, PERF_EVENT_IOC_RESET, 0);
    ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);
    fast_loop(n);
    ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);

    struct rd r;
    read_ev(fd, &r);
    close(fd);

    /* run == 0 means it never reached hardware: perf's "<not counted>" */
    int ok = (r.ena > 0 && r.run > 0);
    if (ok) { n_ok++; } else { n_fail++; }

    const char *nm = raw_name(code);
    if (ok || nm) {
      printf("    0x%02X  %-14llu %-13s %s\n", code,
             (unsigned long long) r.val,
             ok ? "counted" : "NOT counted", nm ? nm : "");
    }
  }
  printf("\n    %d of %d codes allocated; the rest never reached hardware.\n",
         n_ok, n_ok + n_fail);
  printf("    Codes listed with no value but a name are ones you had in\n"
         "    events_list and are now unreachable via the SBI raw path.\n\n");
}

/* ---- test 3: min-of-N ---------------------------------------------------- */

static int fd_i = -1, fd_c = -1;

static uint64_t min_test(uint64_t n, int with_call, uint64_t *min_cyc,
                         double *mean, int *n_at_min, int *n_mux) {
  uint64_t mn = UINT64_MAX, sum = 0, mnc = UINT64_MAX;
  int at = 0, mux = 0;

  for (int t = 0; t < TRIALS; t++) {
    ioctl(fd_c, PERF_EVENT_IOC_RESET, 0);
    ioctl(fd_i, PERF_EVENT_IOC_RESET, 0);
    ioctl(fd_c, PERF_EVENT_IOC_ENABLE, 0);
    ioctl(fd_i, PERF_EVENT_IOC_ENABLE, 0);

    if (with_call) { fast_loop(n); }

    ioctl(fd_i, PERF_EVENT_IOC_DISABLE, 0);
    ioctl(fd_c, PERF_EVENT_IOC_DISABLE, 0);

    struct rd ri, rc;
    read_ev(fd_i, &ri); read_ev(fd_c, &rc);
    if (ri.ena && ri.run != ri.ena) { mux++; }

    if (ri.val < mn)       { mn = ri.val; at = 1; }
    else if (ri.val == mn) { at++; }
    sum += ri.val;
    if (rc.val < mnc) { mnc = rc.val; }
  }

  *min_cyc = mnc; *mean = (double) sum / TRIALS; *n_at_min = at; *n_mux = mux;
  return mn;
}

static void test_min(void) {
  fd_i = open_ev(PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS, 1);
  fd_c = open_ev(PERF_TYPE_RAW, C908_U_MODE_CYCLES, 1);
  if (fd_i < 0 || fd_c < 0) { return; }

  uint64_t mc; double mean; int at, mux;

  uint64_t floor_i = min_test(0, 0, &mc, &mean, &at, &mux);
  uint64_t floor_c = mc;
  printf("=== 3. min-of-%d, 2 events, fast_loop (6 insns/iter) ===\n", TRIALS);
  printf("    empty-window floor: instrs min=%llu mean=%.1f (%d/%d at min)  cycles min=%llu"
         "  multiplexed trials=%d\n\n",
         (unsigned long long) floor_i, mean, at, TRIALS,
         (unsigned long long) floor_c, mux);

  printf("    %-12s %-11s %-11s %-10s %-12s %-11s %s\n", "window", "expect",
         "min-floor", "err", "mean-floor", "min u-cycles", "trials@min");
  for (uint64_t n = 3000; n <= 3000000; n *= 10) {
    uint64_t mn = min_test(n, 1, &mc, &mean, &at, &mux);
    uint64_t expect = 6u * n;
    long long err = (long long) (mn - floor_i) - (long long) expect;
    printf("    n=%-10llu %-11llu %-11llu %+-10lld %-12.1f %-11llu %d/%d%s\n",
           (unsigned long long) n, (unsigned long long) expect,
           (unsigned long long) (mn - floor_i), err, mean - (double) floor_i,
           (unsigned long long) (mc > floor_c ? mc - floor_c : 0), at, TRIALS,
           mux ? "  [MUX!]" : "");
  }
  printf("\n    err constant across window sizes -> minimum is the interrupt-free\n"
         "    count and the constant is the call-site cost; err growing with the\n"
         "    window -> that window exceeds the interrupt period.\n\n");

  close(fd_i); close(fd_c);
}

int main(void) {
  cpu_set_t set;
  CPU_ZERO(&set); CPU_SET(0, &set);
  if (sched_setaffinity(0, sizeof(set), &set) != 0) { perror("sched_setaffinity"); }

  test_exclude_kernel(20000000ull);
  test_multiplex(20000000ull);
  test_raw_sweep(200000ull);
  test_min();
  return 0;
}
