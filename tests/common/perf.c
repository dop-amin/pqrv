#include "perf.h"
#include "c908_events.h"
#include "string.h"

/* longest benchmark name in the tree is ~52 chars */
#define BENCH_NAME_MAX 80

struct perf_event_attr attr[NUM_EVENTS];
uint64_t val1[NUM_EVENTS], val2[NUM_EVENTS];
int fd[NUM_EVENTS], rc;
int last_cnt = 0;

int perf_event_open(struct perf_event_attr * hw, pid_t pid, int cpu, int grp, unsigned long flags) {
    return syscall(__NR_perf_event_open, hw, pid, cpu, grp, flags);
}

void init_perf_events() {

  for (int i = 0; i < NUM_EVENTS-2; i++) {
    attr[i].type = PERF_TYPE_RAW;
    attr[i].config = events_list[i];
    attr[i].disabled = 0;
    fd[i] = perf_event_open(&attr[i], getpid(), -1, -1, 0);
    if (fd[i] < 0) {
      perror("Opening performance counter");
    }
  }
  // perf built in counter: Total instructions
  attr[14].type = PERF_TYPE_HARDWARE;
  attr[14].config =  PERF_COUNT_HW_INSTRUCTIONS;
  attr[14].disabled = 0;
  fd[14] = perf_event_open(&attr[14], getpid(), -1, -1, 0);
  if (fd[14] < 0) {
    perror("Opening performance counter");
  }

  // perf built in counter: total CPU cycles
  attr[15].type = PERF_TYPE_HARDWARE;
  attr[15].config = PERF_COUNT_HW_CPU_CYCLES;
  attr[15].disabled = 0;
  fd[15] = perf_event_open(&attr[15], getpid(), -1, -1, 0);
  if (fd[15] < 0) {
    perror("Opening performance counter");
  }
}

void start_counting_events() {
  // Tell Linux to start counting events
  asm volatile ("nop;"); // pseudo-barrier
  for (int i = 0; i < NUM_EVENTS; i++) {
    rc = read(fd[i], &val1[i], sizeof(val1[i]));
    assert(rc);
  }
  asm volatile ("nop;"); // pseudo-barrier
}

struct event_counter* stop_and_read_events() {
  // Read the counter
  asm volatile("nop;"); // pseudo-barrier
  for (int i = 0; i < NUM_EVENTS; i++) {
    rc = read(fd[i], &val2[i], sizeof(val2[i]));
    assert(rc);
  }
  asm volatile ("nop;"); // pseudo-barrier

  for (int i = 0; i < NUM_EVENTS; i++) {
    events_counter_list[i].count += val2[i] - val1[i];
  }
  return events_counter_list;
}

void cleanup_perf_events() {
  // Close the counter
  for (int i = 0; i < NUM_EVENTS; i++) {
    close(fd[i]);
  }
}

void calc_average(int ITER_PER_TEST, int TEST_COUNT) {
  for (int i = 0; i < NUM_EVENTS; i++) {
    //events_counter_list[i].count = events_counter_list[i].count - (ITER_PER_TEST * TEST_COUNT)*events_counter_list[i].offset;
    events_counter_list[i].count = events_counter_list[i].count / (ITER_PER_TEST * TEST_COUNT);
  }
}

int get_total_cycles() {
    return events_counter_list[15].count;
}

/* Speedup column: remembers the previously printed benchmark so an optimized
 * variant can be divided by its naive counterpart. Benchmark calls in main()
 * are ordered so each naive is immediately followed by its optimized version.
 * The name check below means a mispaired or unpaired row prints "-" rather than
 * a wrong ratio. */
static char prev_name[BENCH_NAME_MAX];
static int  prev_cycles;

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
  float ipc = (float)events_counter_list[14].count / (float)events_counter_list[15].count;
  size_t len = strlen(func_name);
  if (len > 5 && strcmp(func_name + len - 5, "_wrap") == 0) {
    len -= 5;
  }
  int cycles = events_counter_list[15].count;

  /* speedup vs the immediately preceding benchmark, if it is our baseline */
  char speedup[16];
  char baseline[BENCH_NAME_MAX];
  speedup[0] = '-';
  speedup[1] = '\0';
  if (bench_baseline_of(func_name, len, baseline, sizeof(baseline)) &&
      strcmp(baseline, prev_name) == 0 && cycles > 0 && prev_cycles > 0) {
    snprintf(speedup, sizeof(speedup), "%.2f", (float)prev_cycles / (float)cycles);
  }

  printf("%.*s, %i, %i, %.2f, %s \n", (int)len, func_name, events_counter_list[15].count, events_counter_list[14].count, ipc, speedup);

  /* remember this row for the next call (func_name is a string literal - copy it) */
  if (len < sizeof(prev_name)) {
    memcpy(prev_name, func_name, len);
    prev_name[len] = '\0';
    prev_cycles = cycles;
  } else {
    prev_name[0] = '\0';
    prev_cycles = 0;
  }
  /*for (int i = 0; i < NUM_EVENTS; i++) {
    printf("%s, %i \n", events_counter_list[i].name, events_counter_list[i].count);
  }
  printf("\n\n");*/
}
