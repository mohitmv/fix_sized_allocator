// Author: Mohit Saini (mohitsaini1196@gmail.com)

#ifndef QUICK_FIX_SIZED_ALLOCATOR_HPP_
#define QUICK_FIX_SIZED_ALLOCATOR_HPP_

#include <cassert>

namespace quick {
namespace fix_sized_allocator_impl {

struct BlockMetadata;

class BlockLinkList {
  BlockMetadata* head = nullptr;
  BlockMetadata* tail = nullptr;
  void Remove(BlockMetadata* node);
  void PushBack(BlockMetadata* node);
  ~BlockLinkList();
};

class FixSizedAllocatorImpl {
 public:
  FixSizedAllocatorImpl(int element_size);
  void* allocate();
  void deallocate(void* ptr);
  int page_size, element_size, block_capacity, block_offset;
  BlockLinkList partially_filled_blocks;
  BlockLinkList filled_blocks;
};

template<class T>
class FixSizedAllocator {
 public:
  FixSizedAllocator() : impl(sizeof(T)) { }
  T* allocate(std::size_t n) {
    assert(n == 1);
    return (T*)impl.allocate();
  }
  void deallocate(std::size_t n, T* ptr) {
    assert(n == 1);
    return impl.deallocate(ptr);
  }
 private:
  FixSizedAllocatorImpl impl;
};

}  // namespace fix_sized_allocator_impl

using fix_sized_allocator_impl::FixSizedAllocator;

}  // namespace quick

#endif  // QUICK_FIX_SIZED_ALLOCATOR_HPP_
