#define _GNU_SOURCE
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <pthread.h>
#include <assert.h>
#include <sys/time.h>
#include <unistd.h>
#include <asm/unistd.h>
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>
#include <sys/mman.h>
#include <sched.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include "example.h"
#include <perfmon/pfmlib.h>

uint64_t other_pages_cnt = 0;
uint64_t total_pages_cnt = 0;
uint64_t zero_pages_cnt = 0;
uint64_t throttle_cnt = 0;
uint64_t unthrottle_cnt = 0;

static struct perf_event_mmap_page *perf_page[PEBS_NPROCS][NPBUFTYPES];
int pfd[PEBS_NPROCS][NPBUFTYPES];

static long perf_event_open(struct perf_event_attr *hw_event, pid_t pid, 
    int cpu, int group_fd, unsigned long flags)
{
  int ret;

  ret = syscall(__NR_perf_event_open, hw_event, pid, cpu,
		group_fd, flags);
  return ret;
}

static struct perf_event_mmap_page* perf_setup(__u64 config, __u64 config1, __u64 cpu, __u64 type)
{
  struct perf_event_attr attr;

  memset(&attr, 0, sizeof(struct perf_event_attr));

   /*
   If type is PERF_TYPE_RAW, then a custom "raw" config value is needed. 
   Most CPUs support events that are not covered by the "generalized" events. 
   These are implementation defined; see your CPU manual (for example the Intel Volume 3B documentation or the AMD BIOS and Kernel Developer Guide). 
   The libpfm4 library can be used to translate from the name in the architectural manuals to the raw hex value perf_event_open() expects in this field.
  */
  attr.type = PERF_TYPE_RAW;
  attr.size = sizeof(struct perf_event_attr);

  attr.config = config;
  attr.config1 = config1;
  attr.sample_period = SAMPLE_PERIOD;

  attr.sample_type = PERF_SAMPLE_IP | PERF_SAMPLE_TID | PERF_SAMPLE_WEIGHT | PERF_SAMPLE_ADDR;
  attr.disabled = 0;
  //attr.inherit = 1;
  attr.exclude_kernel = 1;
  attr.exclude_hv = 1;
  attr.exclude_callchain_kernel = 1;
  attr.exclude_callchain_user = 1;
  attr.precise_ip = 1;

  
  pfd[cpu][type] = perf_event_open(&attr, -1, cpu, -1, 0);
  if(pfd[cpu][type] == -1) {
    perror("perf_event_open");
  }
  assert(pfd[cpu][type] != -1);

  size_t mmap_size = sysconf(_SC_PAGESIZE) * PERF_PAGES;
  /* printf("mmap_size = %zu\n", mmap_size); */
  struct perf_event_mmap_page *p = mmap(NULL, mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, pfd[cpu][type], 0);
  if(p == MAP_FAILED) {
    perror("mmap");
  }
  assert(p != MAP_FAILED);

  return p;
}


void *pebs_scan_thread()
{

  cpu_set_t cpuset;
  pthread_t thread;

  thread = pthread_self();
  CPU_ZERO(&cpuset);
  CPU_SET(SCANNING_THREAD_CPU, &cpuset);
  int s = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
  if (s != 0) {
    perror("pthread_setaffinity_np");
    assert(0);
  }
  pid_t   tid = gettid();

  char file_name[64];
  snprintf(file_name, sizeof(file_name), "profiling_results_%d.txt", tid);

  FILE *f = fopen(file_name, "w");
  if (f == NULL)
  {
    printf("Error opening file!\n");
    exit(1);
  }


  for(;;) {
    for (int i = 0; i < PEBS_NPROCS; i++) {
      for(int j = 0; j < NPBUFTYPES; j++) {
        struct perf_event_mmap_page *p = perf_page[i][j];
        char *pbuf = (char *)p + p->data_offset;

        __sync_synchronize();

        if(p->data_head == p->data_tail) {
          continue;
        }

        struct perf_event_header *ph = (void *)(pbuf + (p->data_tail % p->data_size));
        struct perf_sample* ps;

        switch(ph->type) {
        case PERF_RECORD_SAMPLE:
            ps = (struct perf_sample*)ph;
            assert(ps != NULL);
            if(ps->addr != 0) {
              //__u64 pfn = ps->addr & HUGE_PFN_MASK;
                fprintf(f,"%p\n", (void *)ps->addr);
            }
            else {
              zero_pages_cnt++;
            }
  	      break;
        case PERF_RECORD_THROTTLE:
        case PERF_RECORD_UNTHROTTLE:
          //fprintf(stderr, "%s event!\n",
          //   ph->type == PERF_RECORD_THROTTLE ? "THROTTLE" : "UNTHROTTLE");
          if (ph->type == PERF_RECORD_THROTTLE) {
              throttle_cnt++;
          }
          else {
              unthrottle_cnt++;
          }
          break;
        default:
          fprintf(stderr, "Unknown type %u\n", ph->type);
          //assert(!"NYI");
          break;
        }

        p->data_tail += ph->size;
      }

      close(f);
    }
  }

  return NULL;
}






void pebs_init(void)
{
  pthread_t scan_thread;

  printf("pebs_init: started\n");

  /*
   * Initialize pfm library (required before we can use it)
   */
  int ret = pfm_initialize();
  if (ret != PFM_SUCCESS)
    errx(1, "Cannot initialize library: %s", pfm_strerror(ret));

  struct perf_event_attr attr;

  memset(&attr, 0, sizeof(attr));

  ret = pfm_get_perf_event_encoding("MEM_LOAD_L3_MISS_RETIRED.LOCAL_DRAM", PFM_PLMH, &attr, NULL, NULL);
  if (ret != PFM_SUCCESS)
      err(1, " cannot get encoding %s", pfm_strerror(ret));
  
  __u64 event1 = attr.config;

  ret = pfm_get_perf_event_encoding("MEM_LOAD_RETIRED.LOCAL_PMM", PFM_PLMH, &attr, NULL, NULL);
  if (ret != PFM_SUCCESS)
      err(1, " cannot get encoding %s", pfm_strerror(ret));
  __u64 event2 = attr.config;

  ret = pfm_get_perf_event_encoding("MEM_INST_RETIRED.ALL_STORES", PFM_PLMH, &attr, NULL, NULL);
  if (ret != PFM_SUCCESS)
      err(1, " cannot get encoding %s", pfm_strerror(ret));
  __u64 event3 = attr.config;

  printf("events number are %x, %x, %x\n", event1, event2, event3);

  for (int i = 0; i < PEBS_NPROCS; i++) {
    
    perf_page[i][DRAMREAD] = perf_setup(event1, 0, i, DRAMREAD);      // MEM_LOAD_L3_MISS_RETIRED.LOCAL_DRAM
    perf_page[i][NVMREAD] = perf_setup(event2, 0, i, NVMREAD);     // MEM_LOAD_RETIRED.LOCAL_PMM
    perf_page[i][WRITE] = perf_setup(event3, 0, i, WRITE);    // MEM_INST_RETIRED.ALL_STORES
  }
  int r = pthread_create(&scan_thread, NULL, pebs_scan_thread, NULL);
  assert(r == 0);

  printf("pebs_init: finished\n");

}

void pebs_shutdown()
{
  for (int i = 0; i < PEBS_NPROCS; i++) {
    for (int j = 0; j < NPBUFTYPES; j++) {
      ioctl(pfd[i][j], PERF_EVENT_IOC_DISABLE, 0);
      //munmap(perf_page[i][j], sysconf(_SC_PAGESIZE) * PERF_PAGES);
    }
  }
}


