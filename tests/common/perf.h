#ifndef PERF_H
#define PERF_H

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
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



//perf counter syscall
extern int perf_event_open(struct perf_event_attr * hw, pid_t pid, int cpu, int grp, unsigned long flags);

extern void init_perf_events();

extern void start_counting_events();

extern struct event_counter* stop_and_read_events();

extern void cleanup_perf_events();

extern void calc_average(int ITER_PER_TEST, int TEST_COUNT);

extern int get_total_cycles();

extern void print_counter();

#endif //PERF_H
