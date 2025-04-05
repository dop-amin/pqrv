#ifndef C908_EVENTS_H
#define C908_EVENTS_H

#define NUM_EVENTS 16  // maximum 16 events at once possible

// list of available events (not complete, see table 14.6 XuanTie-C908 manual)
enum events {
  L1_ICACHE_ACCESS = 0x1,
  L1_ICACHE_MISS = 0x36 ,
  STORE_INSTR = 0xB,
  ISSUE_INSTR = 0x16,
  ALU_INSTR = 0x1D,
  VECTOR_SIMD_INSTR = 0x1F,
  CSR_INSTR = 0x20,
  STALLED_CYCLES_FRONTEND = 0x27,
  STALLED_CYCLES_BACKEND = 0x28,
  M_MODE_CYCLES = 0x2B,
  S_MODE_CYCLES = 0x2C,
  U_MODE_CYCLES = 0x2D,
  LOAD_INSTR = 0x30,
  FUSED_INSTR = 0x31,
  MULT_INSTR = 0x32,
  DIV_INSTR = 0x33,
  BRANCH_INSTR = 0x36,
  CYCLES_TOTAL = 0x00,  // perf built-in instruction
  INSTR_TOTAL = 0x00,  // perf built-in instruction
  UNALIGN_LOAD_INSTR = 0x3d,
  UNALIGN_STORE_INSTR = 0x3e,
  LR_INSTR = 0x3f,
  SC_INSTR = 0x40,
  AMO_INSTR = 0x41,
  BARRIER_INSTR = 0x42
};

// List of events to count
enum events events_list[] = {
  STORE_INSTR, ALU_INSTR, CSR_INSTR,
  LOAD_INSTR, FUSED_INSTR, MULT_INSTR, DIV_INSTR, BRANCH_INSTR,
  UNALIGN_LOAD_INSTR, UNALIGN_STORE_INSTR, LR_INSTR, SC_INSTR, AMO_INSTR, BARRIER_INSTR, CYCLES_TOTAL, INSTR_TOTAL,
};

// Define a struct to hold event data and its counter
struct event_counter {
  enum events event;   // Event type
  const char *name;    // Event name
  int offset;          // Baseline correction
  int count;           // Counter for occurrences
};

// Initialize an array of event_counter structs
struct event_counter events_counter_list[] = {
  {STORE_INSTR, "STORE_INSTR", 3, 0},
  {ALU_INSTR, "ALU_INSTR", 11, 0},
  {CSR_INSTR, "CSR_INSTR", 0, 0},
  {LOAD_INSTR, "LOAD_INSTR", 4, 0},
  {FUSED_INSTR, "FUSED_INSTR", 0, 0},
  {MULT_INSTR, "MULT_INSTR", 0, 0},
  {DIV_INSTR, "DIV_INSTR", 0, 0},
  {BRANCH_INSTR, "BRANCH_INSTR", 6, 0},
  {UNALIGN_LOAD_INSTR, "UNALIGN_LOAD_INSTR", 0, 0},
  {UNALIGN_STORE_INSTR, "UNALIGN_STORE_INSTR", 0, 0},
  {LR_INSTR, "LR_INSTR", 0, 0},
  {SC_INSTR, "SC_INSTR", 0, 0},
  {AMO_INSTR, "AMO_INSTR", 0, 0},
  {BARRIER_INSTR, "BARRIER_INSTR", 0, 0},
  {CYCLES_TOTAL, "CYCLES_TOTAL", 25, 0},
  {INSTR_TOTAL, "INSTR_TOTAL", 29, 0},
  /*{ISSUE_INSTR, "ISSUE_INSTR", 0},
  {VECTOR_SIMD_INSTR, "VECTOR_SIMD_INSTR", 0},
  {STALLED_CYCLES_FRONTEND, "STALLED_CYCLES_FRONTEND", 0},
  {STALLED_CYCLES_BACKEND, "STALLED_CYCLES_BACKEND", 0},
  {M_MODE_CYCLES, "M_MODE_CYCLES", 0},
  {S_MODE_CYCLES, "S_MODE_CYCLES", 0},
  {U_MODE_CYCLES, "U_MODE_CYCLES", 0},*/
};

#endif //C908_EVENTS_H
