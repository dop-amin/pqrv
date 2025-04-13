#include "perf.h"
#include "c908_events.h"

struct perf_event_attr attr[NUM_EVENTS];
uint64_t val1[NUM_EVENTS], val2[NUM_EVENTS];
int fd[NUM_EVENTS], rc;

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
    //debug_printf("%i\n", events_counter_list[i].count);
  }
}

int get_total_cycles() {
    return events_counter_list[15].count;
}

void print_counter() {
  for (int i = 0; i < NUM_EVENTS; i++) {
    printf("%s, %i \n", events_counter_list[i].name, events_counter_list[i].count);
  }
}
