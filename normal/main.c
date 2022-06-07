#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <string.h>
#include <thread>
#include <unistd.h>
#include <algorithm>
#include <cstdlib>
#include <random>
#include <sys/mman.h>
#include "example.h"

static bool duration_expired = false;
//static std::map<uint64_t, std::pair<uint32_t, uint32_t>> addr_map;
static uint64_t point_count;
uint32_t *count_ptr;
char* p_buf;

void NormalWork(unsigned long mbytes, unsigned long mean, unsigned long gamma) {
  std::default_random_engine random_engine;
  std::normal_distribution<double> distribution(mean, gamma);
  mbytes = mbytes / 4096 * 4096;
  p_buf = static_cast<char*>( mmap(nullptr, mbytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
  memset(p_buf, 0x7, mbytes);
  uint64_t op_count = 0;
  constexpr uint64_t kOpUnit = 2 * 1024 * 1024;
  point_count = mbytes / kOpUnit;
  count_ptr = (uint32_t*) malloc(sizeof(uint32_t) * point_count);
  for (op_count = 0; op_count < point_count; op_count++) {
    count_ptr[op_count] = 0;
  }
  op_count = 0;
  printf("Work: %p\n", p_buf);
  while (!duration_expired) {
    uint64_t offset = ((uint64_t) distribution(random_engine)) % point_count;
    count_ptr[offset]++;
    memset(reinterpret_cast<uint8_t*>(reinterpret_cast<uintptr_t>(p_buf) + offset * kOpUnit), 0x7, kOpUnit);
    if (offset < point_count - 1 && false) {
      if (memcmp(reinterpret_cast<uint8_t*>(reinterpret_cast<uintptr_t>(p_buf) + offset * kOpUnit),
                 reinterpret_cast<uint8_t*>(reinterpret_cast<uintptr_t>(p_buf) + offset * kOpUnit + kOpUnit),
                 kOpUnit) != 0) {
        printf("memory go die\n");
        abort();
      }
    }
    op_count++;
  }
  //munmap(p_buf, mbytes);
  printf("Data ptr: %p\n", p_buf);
  uintptr_t buf_start = reinterpret_cast<uintptr_t>(p_buf);
  for (op_count = 0; op_count < point_count; op_count++) {
  //  printf("%llu  %llu --  %llu\n", op_count, buf_start + op_count * kOpUnit, count_ptr[op_count]);
  }
}


/*
constexpr static size_t PAGE_SIZE_2MB = 2 * 1024 * 1024;
constexpr static uintptr_t PAGE_LOWER_MASK_2MB = PAGE_SIZE_2MB - 1;
constexpr static uintptr_t PAGE_UPPER_MASK_2MB = ~PAGE_LOWER_MASK_2MB;
*/


int main(int argc, char** argv) {

	pebs_init();
	printf("Thread Starts\n");
	std::thread th(NormalWork, 4UL * 1024 * 1024 * 1024, 2048, 250);
	sleep(60);
	duration_expired = true;
	th.join();
	printf("Thread Completes\n");
	pebs_shutdown();

	return 0;
} 
