#ifndef PEBS_H
#define PEBS_H

#include <pthread.h>
#include <stdint.h>
#include <inttypes.h>
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>

#define PEBS_NPROCS 24
#define PERF_PAGES	(1 + (1 << 6))	// Has to be == 1+2^n, here 1MB
//#define SAMPLE_PERIOD	10007
#define SAMPLE_PERIOD 100
//#define SAMPLE_FREQ	100


#define FAULT_THREAD_CPU 24

#define SCANNING_THREAD_CPU (FAULT_THREAD_CPU + 1)
#define MIGRATION_THREAD_CPU (SCANNING_THREAD_CPU + 1)

struct perf_sample {
  struct perf_event_header header;
  __u64	ip;
  __u32 pid, tid;    /* if PERF_SAMPLE_TID */
  __u64 addr;        /* if PERF_SAMPLE_ADDR */
  __u64 weight;      /* if PERF_SAMPLE_WEIGHT */
  /* __u64 data_src;    /\* if PERF_SAMPLE_DATA_SRC *\/ */
};

enum pbuftype {
  DRAMREAD = 0,
  NVMREAD = 1,  
  WRITE = 2,
  NPBUFTYPES
};


void pebs_init(void);

void pebs_shutdown(void);
#endif 
