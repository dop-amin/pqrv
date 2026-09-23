#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "perf.h"
#include "c908_events.h"
#include "string.h"
#include <inttypes.h>
#include <sched.h>

/* longest benchmark name in the tree is ~52 chars */
#define BENCH_NAME_MAX 80

/* events_list keeps the architectural instruction and cycle counters at index
 * 0 and 1 -- the two that feed the printed report -- so these stay correct at
 * any NUM_EVENTS. Everything from index 2 on is a raw C908 event. */
#define EV_INSTR  0
#define EV_CYCLES 1
#define EV_RAW_0  2

struct perf_event_attr attr[NUM_EVENTS];
int fd[NUM_EVENTS];  // file descriptors for perf events
int last_cnt = 0;

/* Per-window state. Instructions take the MINIMUM over windows: the counter is
 * U-mode filtered so nothing systematic inflates it, and whatever residual
 * interrupt or preemption cost survives can only ever add. One clean window is
 * therefore exact. Cycles keep the sum, since they vary legitimately between
 * clean windows. */
static int      n_windows;
static uint64_t instr_min;
static int      instr_min_hits;

static void perf_reset_windows(void) {
  n_windows = 0;
  instr_min = UINT64_MAX;
  instr_min_hits = 0;
  for (int i = 0; i < NUM_EVENTS; i++) {
    events_counter_list[i].count = 0;
  }
}

int perf_event_open(struct perf_event_attr * hw, pid_t pid, int cpu, int grp, unsigned long flags) {
    return syscall(__NR_perf_event_open, hw, pid, cpu, grp, flags);
}

void init_perf_events() {
  for (int i = 0; i < NUM_EVENTS; i++) {
    memset(&attr[i], 0, sizeof(attr[i]));
    attr[i].size = sizeof(struct perf_event_attr);

    // Count user mode only (not implemented on C908 though)
    attr[i].exclude_kernel = 1;
    attr[i].exclude_hv     = 1;
    attr[i].exclude_idle   = 1;
    attr[i].disabled = 1;  // event is disabled by default

    if (i == EV_INSTR) {
      // perf built in counter: Total instructions
      attr[i].type   = PERF_TYPE_HARDWARE;
      attr[i].config = PERF_COUNT_HW_INSTRUCTIONS;
    } else if (i == EV_CYCLES) {
      /* C908 raw event 0x2D: U-mode cycles only, instead of
       * PERF_COUNT_HW_CPU_CYCLES. Requires an adjusted device tree that enables all event counters.
      */
      attr[i].type = PERF_TYPE_RAW;
      attr[i].config = U_MODE_CYCLES;
    } else {  // default case: custom raw event
      attr[i].type   = PERF_TYPE_RAW;
      attr[i].config = events_list[i];
    }

    fd[i] = perf_event_open(&attr[i], getpid(), -1, -1, 0);
    if (fd[i] < 0) {
      perror("Opening performance counter");
    }

    events_counter_list[i].offset = 0;
  }

  perf_reset_windows();
}

void start_counting_events() {
  // Tell Linux to start counting events: reset and enable
  for (int i = 0; i < NUM_EVENTS; i++) {
    ioctl(fd[i], PERF_EVENT_IOC_RESET, 0);
  }
  for (int i = EV_RAW_0; i < NUM_EVENTS; i++) {  // enable raw events first
    ioctl(fd[i], PERF_EVENT_IOC_ENABLE, 0);
  }
  asm volatile ("nop;"); // pseudo-barrier
  ioctl(fd[EV_CYCLES], PERF_EVENT_IOC_ENABLE, 0);  // enable instruction and cycle coutner last
  ioctl(fd[EV_INSTR],  PERF_EVENT_IOC_ENABLE, 0);
}

struct event_counter* stop_and_read_events() {
  asm volatile ("nop;"); // pseudo-barrier
  ioctl(fd[EV_INSTR],  PERF_EVENT_IOC_DISABLE, 0);  // disable important counters first (no loop overhead)
  ioctl(fd[EV_CYCLES], PERF_EVENT_IOC_DISABLE, 0);
  for (int i = EV_RAW_0; i < NUM_EVENTS; i++) {
    ioctl(fd[i], PERF_EVENT_IOC_DISABLE, 0);
  }

  // Read the counters; each holds the delta over the window just closed.
  for (int i = 0; i < NUM_EVENTS; i++) {
    uint64_t val = 0;
    if (read(fd[i], &val, sizeof(val)) != (ssize_t) sizeof(val)) {
      perror("Reading performance counter");
      continue;
    }
    if (i == EV_INSTR) {
      if (val < instr_min) {
        instr_min = val;
        instr_min_hits = 1;
      } else if (val == instr_min) {
        instr_min_hits++;
      }
    }
    events_counter_list[i].count += val;
  }
  n_windows++;
  return events_counter_list;
}


void cleanup_perf_events() {
  // Close the counter
  for (int i = 0; i < NUM_EVENTS; i++) {
    close(fd[i]);
  }
}


__attribute__((noipa)) static void perf_empty_fn(void *p) {
  (void) p;
}

/* Run the benchmark's exact loop shape over an empty body and keep the totals
 * as each counter's offset. Same window count and same calls per window, so
 * the loop bookkeeping, call/return and per-window ioctl cost cancel on
 * subtraction rather than being estimated. Call after init_perf_events(). */
void perf_calibrate_offset(int ITER_PER_TEST, int TEST_COUNT) {
  uint64_t dummy = 0;

  perf_reset_windows();
  for (unsigned cnt = 0; cnt < (unsigned) TEST_COUNT; cnt++) {
    start_counting_events();
    for (unsigned cntp = 0; cntp < (unsigned) ITER_PER_TEST; cntp++) {
      perf_empty_fn(&dummy);
    }
    stop_and_read_events();
  }

  for (int i = 0; i < NUM_EVENTS; i++) {
    events_counter_list[i].offset =
        (i == EV_INSTR) ? instr_min : events_counter_list[i].count;
  }
  perf_reset_windows();
}

void calc_average(int ITER_PER_TEST, int TEST_COUNT) {
  uint64_t n = (uint64_t) ITER_PER_TEST * (uint64_t) TEST_COUNT;
  uint64_t w = (n_windows > 0) ? (uint64_t) n_windows : 1;

  for (int i = 0; i < NUM_EVENTS; i++) {
    /* Instructions are a per-window minimum, so they divide by the calls in
     * one window; everything else is a sum over all windows. */
    uint64_t c   = (i == EV_INSTR) ? instr_min : events_counter_list[i].count;
    uint64_t div = (i == EV_INSTR) ? (n / w) : n;
    uint64_t ovh = events_counter_list[i].offset;

    if (i == EV_INSTR && n_windows == 0) { c = 0; }
    c = (c > ovh) ? c - ovh : 0;
    events_counter_list[i].count = (div > 0) ? c / div : 0;
  }
}

uint64_t get_total_cycles() {
    return events_counter_list[EV_CYCLES].count;
}

/* Speedup column: remembers the previously printed benchmark so an optimized
 * variant can be divided by its naive counterpart. Benchmark calls in main()
 * are ordered so each naive is immediately followed by its optimized version.
 * The name check below means a mispaired or unpaired row prints "-" rather than
 * a wrong ratio. */
static char     prev_name[BENCH_NAME_MAX];
static uint64_t prev_cycles;

/* Derive the naive counterpart of an optimized benchmark name by deleting the
 * optimization token in place. "_opt_c908" must be tried before "_opt", else
 * "x_opt_c908" would wrongly become "x_c908". Handles all spellings in the tree:
 *   foo_opt_c908        -> foo
 *   foo_dual_opt_c908   -> foo_dual
 *   foo_opt_c908_dual   -> foo_dual   (token is an infix)
 *   foo_opt             -> foo
 * Returns 0 if name is not an optimized variant. */
static int bench_baseline_of(const char *name, size_t len, char *out, size_t outsz)
{
  static const char *tokens[] = { "_opt_c908", "_opt" };
  for (unsigned t = 0; t < sizeof(tokens) / sizeof(tokens[0]); t++) {
    const char *tok = tokens[t];
    size_t toklen = strlen(tok);
    for (size_t i = 0; i + toklen <= len; i++) {
      if (strncmp(name + i, tok, toklen) == 0) {
        size_t tail = len - i - toklen;
        if (i + tail + 1 > outsz) { return 0; }
        memcpy(out, name, i);
        memcpy(out + i, name + i + toklen, tail);
        out[i + tail] = '\0';
        return 1;
      }
    }
  }
  return 0;
}

void print_counter(const char* func_name) {
  float ipc = (float)events_counter_list[EV_INSTR].count / (float)events_counter_list[EV_CYCLES].count;
  size_t len = strlen(func_name);
  if (len > 5 && strcmp(func_name + len - 5, "_wrap") == 0) {
    len -= 5;
  }
  uint64_t cycles = events_counter_list[EV_CYCLES].count;

  /* speedup vs the immediately preceding benchmark, if it is our baseline */
  char speedup[16];
  char baseline[BENCH_NAME_MAX];
  speedup[0] = '-';
  speedup[1] = '\0';
  if (bench_baseline_of(func_name, len, baseline, sizeof(baseline)) &&
      strcmp(baseline, prev_name) == 0 && cycles > 0 && prev_cycles > 0) {
    snprintf(speedup, sizeof(speedup), "%.2f", (float)prev_cycles / (float)cycles);
  }

  printf("%.*s, %" PRIu64 ", %" PRIu64 ", %.2f, %s", (int)len, func_name,
         events_counter_list[EV_CYCLES].count, events_counter_list[EV_INSTR].count, ipc, speedup);
  /* How many windows tied the instruction minimum. 0 of N would mean no window
   * was ever clean; only emitted when the benchmark actually used windows. */
  if (n_windows > 1) {
    printf(", %d/%d", instr_min_hits, n_windows);
  }
  printf(" \n");

  /* remember this row for the next call (func_name is a string literal - copy it) */
  if (len < sizeof(prev_name)) {
    memcpy(prev_name, func_name, len);
    prev_name[len] = '\0';
    prev_cycles = cycles;
  } else {
    prev_name[0] = '\0';
    prev_cycles = 0;
  }
}
