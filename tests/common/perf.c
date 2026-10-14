#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "perf.h"
#include "c908_events.h"
#include "string.h"
#include "common.h"   // median()
#include <inttypes.h>
#include <sched.h>

#define BENCH_NAME_MAX 80
#define MAX_WINDOWS 10000  // >= largest TEST_COUNT

/* events_list keeps the architectural instruction and cycle counters at index
 * 0 and 1. Everything from index 2 on is a raw C908 event. */
#define EV_INSTR  0
#define EV_CYCLES 1
#define EV_RAW_0  2

struct perf_event_attr attr[NUM_EVENTS];
int fd[NUM_EVENTS];  // file descriptors for perf events
struct perf_event_mmap_page* mapping[NUM_EVENTS];
unsigned csrs[NUM_EVENTS];
int last_cnt = 0;
unsigned t1[NUM_EVENTS];
unsigned t2[NUM_EVENTS];

static int      n_windows;
static uint64_t windows[NUM_EVENTS][MAX_WINDOWS];  // per-event delta of each window
static uint64_t instr_min;
static int      instr_min_hits;
static char     prev_name[BENCH_NAME_MAX];
static uint64_t prev_cycles;

/* csrr takes the CSR number as an immediate, so this macro creates methods for all 32 possible counters. */
#define CASE(n) case n: { uint64_t v; __asm__ volatile("csrr %0, " #n : "=r"(v)); return v; }
static uint64_t read_csr_counter(unsigned csr)
{
    switch (csr) {
    CASE(0xc00) CASE(0xc01) CASE(0xc02) CASE(0xc03) CASE(0xc04) CASE(0xc05)
    CASE(0xc06) CASE(0xc07) CASE(0xc08) CASE(0xc09) CASE(0xc0a) CASE(0xc0b)
    CASE(0xc0c) CASE(0xc0d) CASE(0xc0e) CASE(0xc0f) CASE(0xc10) CASE(0xc11)
    CASE(0xc12) CASE(0xc13) CASE(0xc14) CASE(0xc15) CASE(0xc16) CASE(0xc17)
    CASE(0xc18) CASE(0xc19) CASE(0xc1a) CASE(0xc1b) CASE(0xc1c) CASE(0xc1d)
    CASE(0xc1e) CASE(0xc1f)
    }
    return 0;
}
#undef CASE

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

int init_perf_events() {
  for (int i = 0; i < NUM_EVENTS; i++) {
    memset(&attr[i], 0, sizeof(attr[i]));
    attr[i].size = sizeof(struct perf_event_attr);

    // Count user mode only (not implemented on C908 though)
    attr[i].exclude_kernel = 1;
    attr[i].exclude_hv     = 1;
    attr[i].exclude_idle   = 1;
    attr[i].disabled = 0;

    if (i == EV_INSTR) {
      // perf built in counter: Total instructions
      attr[i].type   = PERF_TYPE_HARDWARE;
      attr[i].config = PERF_COUNT_HW_INSTRUCTIONS;
    } else if (i == EV_CYCLES) {
      /* C908 raw event 0x2D: U-mode cycles only, instead of
       * PERF_COUNT_HW_CPU_CYCLES. Requires an adjusted device tree that enables all event counters.
       * Latest kernel for the C908 supports it.
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

  // memory map the file descriptor from kernel to user space s.t. we can read the counters with the csr command.
  // requires kernel.perf_user_access = 1
  for (int i = 0; i < NUM_EVENTS; i++) {
    mapping[i] = mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ, MAP_SHARED, fd[i], 0);
    if (mapping[i] == MAP_FAILED) { perror("mmap"); return 1; }
    if (!mapping[i]->cap_user_rdpmc || !mapping[i]->index) {
      perror("\nNOT usable: userspace counter access is not granted.\n"
               "Check: sysctl kernel.perf_user_access (needs 1 for this path)\n");
      return 2;
    }
    csrs[i] = 0xc00 + mapping[i]->index - 1;
  }
  perf_reset_windows();
  return 0;
}

void start_counting_events() {
  for (int i = 0; i < NUM_EVENTS; i++) {
    t1[i] = read_csr_counter(csrs[i]);
  }
}

struct event_counter* stop_and_read_events() {
  asm volatile ("nop;"); // pseudo-barrier

  // Read the counters; each holds the delta over the window just closed.
  for (int i = 0; i < NUM_EVENTS; i++) {
    t2[i] = read_csr_counter(csrs[i]);
    uint64_t d = t2[i] - t1[i];
    if (i == EV_INSTR) {
      if (d < instr_min) {
        instr_min = d;
        instr_min_hits = 1;
      } else if (d == instr_min) {
        instr_min_hits++;
      }
    }
    assert(n_windows < MAX_WINDOWS);
    windows[i][n_windows] = d;
    events_counter_list[i].count += d;
  }
  n_windows++;
  return events_counter_list;
}

void cleanup_perf_events() {
  for (int i = 0; i < NUM_EVENTS; i++) {
    munmap(mapping[i], sysconf(_SC_PAGESIZE));
    close(fd[i]);
  }
}

/**
 * @brief Empty dummy fct to measure overhead
 * @param p Dummy value to consider overhead of passing parameters to the dummy fct
 */
__attribute__((noipa)) static void perf_empty_fn(void *p) {
  (void) p;
}

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
    if (i == EV_INSTR) {
      events_counter_list[i].offset = instr_min;
    } else {
      events_counter_list[i].offset = median(windows[i], n_windows);
    }
  }
  perf_reset_windows();
}

void calc_average(int ITER_PER_TEST, int TEST_COUNT) {
  uint64_t n = (uint64_t) ITER_PER_TEST * (uint64_t) TEST_COUNT;
  uint64_t w = (n_windows > 0) ? (uint64_t) n_windows : 1;

  for (int i = 0; i < NUM_EVENTS; i++) {
    uint64_t c, div;
    if (i == EV_INSTR) {
      // for counting instructions, take the minimum window, divided by the calls per window
      c   = (n_windows > 0) ? instr_min : 0;
      div = n / w;
    } else {
      // for cycles and raw events, use median window, divided by the calls per window
      c   = median(windows[i], n_windows);
      div = n / w;
    }
    uint64_t ovh = events_counter_list[i].offset;

    c = (c > ovh) ? c - ovh : 0;
    events_counter_list[i].count = (div > 0) ? c / div : 0;
  }
}

/**
 * @brief Derive the naive counterpart of an optimized benchmark name by deleting the
 * first optimization token ("_opt_c908" or "_opt"), e.g. "ntt_opt_c908" -> "ntt".
 * @param name  benchmark name
 * @param len   number of characters of name to consider (name need not be terminated there)
 * @param out   buffer receiving the null-terminated baseline name
 * @param outsz size of out in bytes
 * @return 1 if name is an optimized variant and out was written, 0 otherwise
 */
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
