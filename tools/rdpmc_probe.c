/* rdpmc_probe.c -- can the C908's programmable counters be read from U-mode?
 *
 * perf can expose a counter to userspace through the mmap'd
 * perf_event_mmap_page: cap_user_rdpmc says it is allowed, index says which
 * hpmcounter the kernel assigned. On RISC-V the counter is then a plain CSR
 * read, 0xc00 + index - 1, i.e. ONE instruction -- no syscall.
 *
 * Needs kernel.perf_user_access = 1 (the mmap mode). Note that is NOT the
 * same as 2, which is the legacy mode that only opens up cycle/instret.
 *
 * Build:
 *   riscv64-unknown-linux-gnu-gcc -O2 -static -march=rv64imdv_zicsr -mabi=lp64d \
 *     tools/rdpmc_probe.c -o rdpmc_probe
 *
 * _zicsr is required or the assembler rejects csrr; lp64d because the
 * toolchains here ship only the hard-float sysroot.
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <linux/perf_event.h>

#define C908_U_MODE_CYCLES 0x2d

/* csrr takes the CSR number as an immediate, so one case per counter. */
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

/* ~10 us of user-mode work, the size of a real measurement window */
static void short_loop(void)
{
    volatile uint64_t x = 0;
    for (volatile int i = 0; i < 3000; i++) x += i;
}

static void spin(void)
{
    volatile uint64_t x = 0;
    for (volatile int i = 0; i < 2000000; i++) x += i;
}

static int probe_event(const char *name, uint32_t type, uint64_t config)
{
    struct perf_event_attr attr;

    printf("================ %s ================\n", name);

    memset(&attr, 0, sizeof attr);
    attr.type   = type;
    attr.size   = sizeof attr;
    attr.config = config;  // counter ID

    int fd = syscall(__NR_perf_event_open, &attr, 0, -1, -1, 0);
    if (fd < 0) { perror("perf_event_open"); return 1; }

    struct perf_event_mmap_page *pc = mmap(NULL, sysconf(_SC_PAGESIZE),
                                           PROT_READ, MAP_SHARED, fd, 0);
    if (pc == MAP_FAILED) { perror("mmap"); return 1; }

    printf("cap_user_rdpmc = %u\n", pc->cap_user_rdpmc);
    printf("cap_user_time  = %u\n", pc->cap_user_time);
    printf("index          = %u\n", pc->index);
    printf("offset         = %lld\n", (long long) pc->offset);
    printf("pmc_width      = %u\n", pc->pmc_width);

    if (!pc->cap_user_rdpmc || !pc->index) {
        printf("\nNOT usable: userspace counter access is not granted.\n"
               "Check: sysctl kernel.perf_user_access (needs 1 for this path)\n");
        return 2;
    }

    unsigned csr = 0xc00 + pc->index - 1;
    printf("\n-> counter is CSR 0x%x (hpmcounter%u)\n\n", csr, pc->index - 1);

    /* Same window measured two ways: one csrr pair, and a read() syscall. */
    uint64_t seq0 = pc->lock;
    uint64_t c0 = read_csr_counter(csr);
    uint64_t r0 = 0; ssize_t n0 = read(fd, &r0, sizeof r0); (void) n0;
    spin();
    uint64_t r1 = 0; ssize_t n1 = read(fd, &r1, sizeof r1); (void) n1;
    uint64_t c1 = read_csr_counter(csr);
    uint64_t seq1 = pc->lock;

    printf("csrr  delta = %llu\n", (unsigned long long)(c1 - c0));
    printf("read  delta = %llu\n", (unsigned long long)(r1 - r0));
    printf("seqlock %s during the window\n",
           seq0 == seq1 ? "stable" : "CHANGED -- counter was rescheduled");

    /* Cost of the read itself, minimum over many back-to-back pairs. */
    uint64_t best = ~0ull;
    for (int i = 0; i < 100000; i++) {
        uint64_t a = read_csr_counter(csr);
        uint64_t b = read_csr_counter(csr);
        if (b - a < best) best = b - a;
    }
    printf("\ncsrr read cost = %llu cycles (min of 100000)\n\n",
           (unsigned long long) best);
    munmap(pc, sysconf(_SC_PAGESIZE));
    close(fd);
    return 0;
}

/* ---------------------------------------------------------------------------
 * Overhead comparison.
 *
 * Everything is timed with csrr only, so the yardstick never perturbs what it
 * measures. Each row is the MINIMUM over many trials of (counter after -
 * counter before) with the operation in between, minus the empty-pair
 * baseline. That isolates the cost of the operation itself.
 *
 * This is what the probe's csrr-vs-read deltas were hinting at: the csrr
 * window encloses the read() calls, so the difference between them was never
 * csrr overhead -- it was the syscall cost, measured.
 * ------------------------------------------------------------------------ */
struct ctr { int fd; struct perf_event_mmap_page *pc; unsigned csr; };

static int open_ctr(struct ctr *c, uint32_t type, uint64_t config)
{
    struct perf_event_attr attr;
    memset(&attr, 0, sizeof attr);
    attr.type = type; attr.size = sizeof attr; attr.config = config;
    c->fd = syscall(__NR_perf_event_open, &attr, 0, -1, -1, 0);
    if (c->fd < 0) return -1;
    c->pc = mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ, MAP_SHARED, c->fd, 0);
    if (c->pc == MAP_FAILED || !c->pc->cap_user_rdpmc || !c->pc->index) return -1;
    c->csr = 0xc00 + c->pc->index - 1;
    return 0;
}

#define TRIALS_OH 20000
/* op: 0 nothing, 1 one read(), 2 one ioctl(), 3 the harness's whole window */
static void measure_op(const char *label, struct ctr *cy, struct ctr *in, int op,
                       uint64_t base_c, uint64_t base_i)
{
    uint64_t bc = ~0ull, bi = ~0ull, dummy;
    for (int t = 0; t < TRIALS_OH; t++) {
        uint64_t c0 = read_csr_counter(cy->csr);
        uint64_t i0 = read_csr_counter(in->csr);

        if (op == 1) { ssize_t r = read(cy->fd, &dummy, sizeof dummy); (void) r; }
        else if (op == 2) { ioctl(cy->fd, PERF_EVENT_IOC_RESET, 0); }
        else if (op == 3) {
            /* The harness's window shape, MINUS the ENABLE/DISABLE pair:
             * PERF_EVENT_IOC_DISABLE makes the kernel revoke U-mode access
             * (it clears the scounteren bit), so the csrr that closes this
             * measurement would trap with SIGILL. That is itself the finding:
             * a csrr-based harness must leave the counters running and take
             * deltas, never enable/disable them. */
            ioctl(cy->fd, PERF_EVENT_IOC_RESET, 0);
            ioctl(in->fd, PERF_EVENT_IOC_RESET, 0);
            ssize_t r1 = read(in->fd, &dummy, sizeof dummy); (void) r1;
            ssize_t r2 = read(cy->fd, &dummy, sizeof dummy); (void) r2;
        }

        uint64_t i1 = read_csr_counter(in->csr);
        uint64_t c1 = read_csr_counter(cy->csr);
        if (c1 - c0 < bc) bc = c1 - c0;
        if (i1 - i0 < bi) bi = i1 - i0;
    }
    printf("  %-34s %8llu %12llu\n", label,
           (unsigned long long)(bc - base_c), (unsigned long long)(bi - base_i));
}

static void compare_overhead(void)
{
    struct ctr cy, in;

    printf("================ overhead comparison ================\n");
    if (open_ctr(&cy, PERF_TYPE_RAW, C908_U_MODE_CYCLES) ||
        open_ctr(&in, PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS)) {
        printf("  could not open both counters for user-mode reads\n\n");
        return;
    }

    /* baseline: the four csrr reads with nothing between them */
    uint64_t bc = ~0ull, bi = ~0ull;
    for (int t = 0; t < TRIALS_OH; t++) {
        uint64_t c0 = read_csr_counter(cy.csr), i0 = read_csr_counter(in.csr);
        uint64_t i1 = read_csr_counter(in.csr), c1 = read_csr_counter(cy.csr);
        if (c1 - c0 < bc) bc = c1 - c0;
        if (i1 - i0 < bi) bi = i1 - i0;
    }
    printf("  (min of %d trials, baseline subtracted)\n\n", TRIALS_OH);
    printf("  %-34s %8s %12s\n", "operation", "u-cycles", "instructions");
    printf("  %-34s %8llu %12llu\n", "4x csrr baseline (NOT subtracted)",
           (unsigned long long) bc, (unsigned long long) bi);
    measure_op("one read()", &cy, &in, 1, bc, bi);
    measure_op("one ioctl()", &cy, &in, 2, bc, bi);
    measure_op("harness-shaped: 2 ioctl + 2 read", &cy, &in, 3, bc, bi);
    printf("\n  csrr pair measures a window for ~the baseline above;\n"
           "  the harness row is what a syscall-bounded window costs instead.\n\n");

    munmap(cy.pc, sysconf(_SC_PAGESIZE)); close(cy.fd);
    munmap(in.pc, sysconf(_SC_PAGESIZE)); close(in.fd);
}

/* ---------------------------------------------------------------------------
 * How often does the seqlock really move?
 *
 * Every run so far reported "CHANGED", but those windows were ~19 ms AND
 * contained two read() calls -- a perf read makes the kernel refresh the
 * userpage, which bumps pc->lock. So the probe may have been detecting its
 * own syscalls rather than a reschedule.
 *
 * Here the window is ~10 us of pure user code with no syscall in it, which is
 * what a real measurement looks like. If the lock is stable nearly always,
 * the retry loop is cheap insurance; if not, it is mandatory and we want the
 * rate before trusting csrr.
 * ------------------------------------------------------------------------ */
static void seqlock_rate(void)
{
    struct ctr cy;
    const int N = 20000;
    int changed = 0, idx_moved = 0;
    unsigned idx0;

    printf("================ seqlock stability ================\n");
    if (open_ctr(&cy, PERF_TYPE_RAW, C908_U_MODE_CYCLES)) {
        printf("  could not open the counter for user-mode reads\n\n");
        return;
    }
    idx0 = cy.pc->index;

    for (int t = 0; t < N; t++) {
        uint32_t s0 = cy.pc->lock;
        __atomic_thread_fence(__ATOMIC_ACQUIRE);

        uint64_t a = read_csr_counter(cy.csr);
        short_loop();                      /* ~10 us, no syscalls */
        uint64_t b = read_csr_counter(cy.csr);
        (void) a; (void) b;

        __atomic_thread_fence(__ATOMIC_ACQUIRE);
        if (cy.pc->lock != s0) { changed++; }
        if (cy.pc->index != idx0) { idx_moved++; }
    }

    printf("  window ~10 us of user code, no syscalls, %d trials\n", N);
    printf("  seqlock changed : %d / %d  (%.2f%%)\n", changed, N,
           100.0 * changed / N);
    printf("  index changed   : %d / %d\n", idx_moved, N);
    printf("\n  near 0%% -> retry loop is cheap insurance, csrr harness is safe\n"
           "  high    -> retry loop is mandatory; note the rate\n\n");

    munmap(cy.pc, sysconf(_SC_PAGESIZE)); close(cy.fd);
}

int main(void)
{
    cpu_set_t set;
    CPU_ZERO(&set); CPU_SET(0, &set);
    sched_setaffinity(0, sizeof set, &set);

    /* U-mode cycles: a programmable counter, known to work. */
    probe_event("U Mode Cycles (raw 0x2d)", PERF_TYPE_RAW, C908_U_MODE_CYCLES);

    /* Instructions: maps to the FIXED instret CSR (0xc02), so index should
     * come back as 3. A bare `csrr instret` traps -- the mmap is what makes
     * the kernel grant access. If this works, both columns of the harness
     * become two-instruction reads with no syscall in the window at all. */
    probe_event("Instructions (instret, expect index 3 / CSR 0xc02)",
                PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS);

    compare_overhead();
    seqlock_rate();
    return 0;
}
