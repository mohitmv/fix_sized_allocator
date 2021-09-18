// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include "quick/fix_sized_allocator.hpp"

#include <iostream>
#include <list>
#include <vector>
#include <chrono>  // NOLINT

inline int64_t GetEpochMicroSeconds() {
  using namespace std::chrono;  // NOLINT
  auto epoch_time = system_clock::now().time_since_epoch();
  return duration_cast<microseconds>(epoch_time).count();
}

template<class LL>
int64_t Benchmark1() {
  auto t_start = GetEpochMicroSeconds();
  LL ll;
  for (int i = 0; i < 20; i++) {
    if (i % 3 == 1) {
      for (int j = 0; j < 10000; j++) {
        ll.pop_back();
      }
    } else {
      for (int j = 0; j < 10000; j++) {
        ll.push_back(i+j);
        if (i + j % 10 == 4) {
          std::vector<int> v(10, 4);
          int sum = 0;
          for (int k : v) sum += k;
          ll.push_back(v.size() + sum);
        }
      }
    }
  }
  size_t x = ll.size();
  int y = x * x -  (x + 1) * (x-1);
  return y - 1 + GetEpochMicroSeconds() - t_start;
}


int main() {
  using LL1 = std::list<int>;
  using LL2 = std::list<int, quick::FixSizedAllocator<int>>;
  std::cout << "Time1: " << Benchmark1<LL1>() << std::endl;
  std::cout << "Time2: " << Benchmark1<LL2>() << std::endl;
}
