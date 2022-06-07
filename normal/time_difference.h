/*
 * Copyright (C) 2019-2020 MemVerge Inc.
 */

/**
 * @file time_difference.h
 * @details make it easier to get elapsed interval time
 */

#ifndef MVM_TIMEDIFF_H
#define MVM_TIMEDIFF_H

#include <time.h>
#include <string>

class TimeDiff {
 public:
  TimeDiff() {
    Reset();
  }

  /* start a nanosecond-resolution timer by recording t0 */
  void Reset() {
    GetTime(&t0_);
  }

  /* Use the monotonic timer for these measurements */
  static void GetTime(struct timespec *ts) {
    clock_gettime(CLOCK_MONOTONIC, ts);
  }

  /* Get time elapsed since last reset */
  long ElapsedNs(struct timespec *end_time = nullptr) const {
    struct timespec t1;
    if (end_time == nullptr) {
      GetTime(&t1);
      end_time = &t1;
    }
    return (end_time->tv_sec - t0_.tv_sec) * 1000000000UL
          + (end_time->tv_nsec - t0_.tv_nsec);
  }

  std::string PrintableNs(struct timespec *end_time = nullptr) const {
    constexpr unsigned kDecimalPlaces = 9;
    std::string s = std::to_string(ElapsedNs(end_time));
    while (s.length() <= kDecimalPlaces) {
      s.insert(s.begin(), '0');
    }
    s.insert(std::prev(s.end(), kDecimalPlaces), '.');
    return s;
  }
 private:
  struct timespec t0_;
};

#endif /* MVM_TIMEDIFF_H */
