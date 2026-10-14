#ifndef PERF_H
#define PERF_H

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <sched.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <sys/time.h>
#include <assert.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/unistd.h>
#include <linux/perf_event.h>
#include <sys/ioctl.h>
#include <hal.h>


/**
 * @brief perf_event_open system call wrapper (glibc provides no wrapper)
 * @param hw    event attributes (type, config, enable/exclude flags, ...)
 * @param pid   process to monitor: 0 = calling process, >0 = given pid, -1 = all processes (requires cpu >= 0)
 * @param cpu   CPU to monitor: -1 = any CPU the process runs on, >=0 = only that CPU
 * @param grp   fd of the group leader event, or -1 to create a new group with this event as leader
 * @param flags PERF_FLAG_* flags (e.g. PERF_FLAG_FD_CLOEXEC), usually 0
 * @return file descriptor of the new event on success, -1 on failure (errno set)
 */
extern int perf_event_open(struct perf_event_attr * hw, pid_t pid, int cpu, int grp, unsigned long flags);

/**
 * @brief Open perf events to enable counters and map memory from kernel space to user space
 * @return 0 on success, 2 if memory mapping was not possible and counters not accessible
 */
extern int init_perf_events();

/**
 * @brief Stores the current value of all counters
 */
extern void start_counting_events();

/**
 * @brief Get the counters after the benchmark code ran 
 * @return list of event_counters
 */
extern struct event_counter* stop_and_read_events();

/**
 * @brief Unmap memory and close all file descriptors
 */
extern void cleanup_perf_events();

/**
 * @brief Empty dummy fct to measure overhead
 * @param p Dummy value to consider overhead of passing parameters to the dummy fct
 */
__attribute__((noipa)) static void perf_empty_fn(void *p) {
  (void) p;
}

/**
 * @brief Calculate measurement overhead and set as offset for the event counters. Will be 
 * substracted when getting the counter values after benchmarking.
 * @param ITER_PER_TEST Loop bound of nested, inner loop of the benchmark (calls per window)
 * @param TEST_COUNT Loop bound of outer loop of the benchmark (number of windows)
 */
extern void perf_calibrate_offset(int ITER_PER_TEST, int TEST_COUNT);

/**
 * @brief Convert the raw counters collected over all measurement windows into per-call values.
 * Overwrites events_counter_list[i].count with the result.
 *
 * - Instructions: takes the minimum count of any single window, subtracts the calibration
 *   offset (also a per-window minimum) and divides by the calls per window (ITER_PER_TEST).
 * - Cycles and all other events: takes the median count over all windows (inner loop, ITER_PER_TEST), subtracts the
 *   calibration offset and divides by the calls per window (outer loop, TEST_COUNT).
 *
 * Results are clamped to 0 if the offset exceeds the measured count.
 * @param ITER_PER_TEST Loop bound of nested, inner loop of the benchmark (calls per window)
 * @param TEST_COUNT Loop bound of outer loop of the benchmark (number of windows)
 */
extern void calc_average(int ITER_PER_TEST, int TEST_COUNT);

/**
 * @brief Derive the naive counterpart of an optimized benchmark name by deleting the
 * first optimization token ("_opt_c908" or "_opt"), e.g. "ntt_opt_c908" -> "ntt".
 * @param name  benchmark name
 * @param len   number of characters of name to consider (name need not be terminated there)
 * @param out   buffer receiving the null-terminated baseline name
 * @param outsz size of out in bytes
 * @return 1 if name is an optimized variant and out was written, 0 otherwise
 */
extern int bench_baseline_of(const char *name, size_t len, char *out, size_t outsz);

/**
 * @brief Print one CSV row for a benchmark: name, cycles, instructions, IPC, speedup and,
 * if more than one window was measured, how many windows hit the instruction minimum (aka
 * probably a clean window).
 * A trailing "_wrap" is stripped from the name. The speedup compare to the non-optimized 
 * versionis reported (otherwise "-") if the previously printed benchmark is the baseline 
 * of this one (see bench_baseline_of). Call after calc_average().
 * @param func_name benchmark name
 */
extern void print_counter(const char* func_name);

#endif //PERF_H
