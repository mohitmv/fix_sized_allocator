// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include "quick/fix_sized_allocator.hpp"

#include <iostream>
#include <list>

int main() {
  std::list<int, FixSizedAllocator<int>> l;
  l.push_back(3);
  l.push_back(3);
  l.push_back(3);
  l.push_back(3);
  l.push_back(3);
  std::cout << l.size() << std::endl;
  l.pop_back();
  l.pop_back();
  l.pop_back();
  std::cout << l.size() << std::endl;
}
