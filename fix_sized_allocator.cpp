// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include "quick/fix_sized_allocator.hpp"

#include <sys/mman.h>
#include <unistd.h>

#include <memory>
#include <cassert>
#include <iostream>

namespace quick {
namespace fix_sized_allocator_impl {


// filled_blocks    partially_filled_blocks
//     |                        |
//     |                        |
//     |                        |
//     |                        |
//     |                        |
//     |            |-----------|---|----|----|----|----|----|
//     |            |BlockMetadata  |    |  4 |    | 6  | 9  |  Block1
//     |            |---------------|----|----|----|----|----|
//     |                        |
//     |                        |
//     |            |-----------|---|----|----|----|----|----|
//     |            |BlockMetadata  | 45 |    |    | 4  |    |  Block2
//     |            |---------------|----|----|----|----|----|
//     |
//     |
// |---|-----------|----|----|----|----|----|
// |BlockMetadata  |  4 | 6  | 3  | 6  | 4  |  Block1
// |---------------|----|----|----|----|----|
//     |
//     |
// |---|-----------|----|----|----|----|----|
// |BlockMetadata  | 34 | 9  | 13 |  6 |  2 |  Block2
// |---------------|----|----|----|----|----|

struct BlockMetadata {
  uint32_t first_time_alloc_index = 0;
  uint32_t num_allocated = 0;
  // LinkList head of the linkedlist among deleted positions, within a block.
  void* free_list = nullptr;
  BlockMetadata* next_block = nullptr;
  BlockMetadata* previous_block = nullptr;

  void* allocate(int element_size, int block_offset, int block_capacity);
  void deallocate(void* ptr);
};

void BlockLinkList::Remove(BlockMetadata* node) {
  if (node == head && node == tail) {
    head = tail = nullptr;
    return;
  }
  if (node == head) {
    head = node->next_block;
    head->previous_block = nullptr;
    return;
  }
  if (node == tail) {
    tail = node->previous_block;
    tail->next_block = nullptr;
    return;
  }
  node->previous_block->next_block = node->next_block;
  node->next_block->previous_block = node->previous_block;
}

void BlockLinkList::PushBack(BlockMetadata* node) {
  if (head == nullptr) {
    head = tail = node;
    return;
  }
  tail->next_block = node;
  node->previous_block = tail;
  tail = node;
}

BlockLinkList::~BlockLinkList() {
  assert(head == nullptr && tail == nullptr);
}

void* BlockMetadata::allocate(int element_size, int block_offset, int block_capacity) {
  char* block_main_memory = ((char*)this) + block_offset;
  if (first_time_alloc_index < block_capacity) {
    ++num_allocated;
    return block_main_memory + element_size * (first_time_alloc_index++);
  } else if (free_list != nullptr) {
    ++num_allocated;
    auto output = free_list;
    free_list = *((void**)free_list);
    return output;
  } else {
    assert(false);
    return nullptr; // no space left in this block.
  }
}

void BlockMetadata::deallocate(void* ptr) {
  // Construct an object of type `void*` at the deallocated memory @ptr.
  new (ptr) (void*)(free_list);  // inplace new.
  free_list = ptr;
  --num_allocated;
}

void* AllocPage(int page_size) {
  return mmap(nullptr, page_size, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

void FreePage(void* page, int page_size) {
  munmap(page, page_size);
}

BlockMetadata* MakeNewBlock(int page_size) {
  void* block = AllocPage(page_size);
  BlockMetadata* metadata = (BlockMetadata*) block;
  new (block) BlockMetadata();
  return metadata;
}

void DeleteBlock(BlockMetadata* block, int page_size) {
  block->~BlockMetadata();
  FreePage(block, page_size);
}

FixSizedAllocatorImpl::FixSizedAllocatorImpl(int element_size)
    : element_size(element_size) {
  page_size = getpagesize();
  if (element_size * 4 + sizeof(BlockMetadata) > page_size) {
    throw std::runtime_error("FixSizedAllocator doesn't support objects "
                             "with very large sizeof");
  }
  block_capacity = (page_size - sizeof(BlockMetadata))/element_size;
  block_offset = page_size - element_size * block_capacity;
}

void* FixSizedAllocatorImpl::allocate() {
  if (partially_filled_blocks.head == nullptr) {
    auto* block = MakeNewBlock(page_size);
    partially_filled_blocks.PushBack(block);
    auto* output = block->allocate(element_size, block_offset, block_capacity);
    // printf("Alloc: %p\n", output);
    return output;
  }
  auto* block = partially_filled_blocks.head;
  auto* output = block->allocate(element_size, block_offset, block_capacity);
  if (block->num_allocated == block_capacity) {
    partially_filled_blocks.Remove(block);
    filled_blocks.PushBack(block);
  }
  // printf("Alloc: %p\n", output);
  return output;
}

void FixSizedAllocatorImpl::deallocate(void* p) {
  // printf("Dealloc: %p\n", p);
  static_assert(sizeof(void*) == sizeof(long), "");
  char* page_start_addr = ((char*)p) - (((long)p) & (page_size - 1));
  BlockMetadata* block = (BlockMetadata*)page_start_addr;
  bool is_filled = (block->num_allocated == block_capacity);
  block->deallocate(p);
  if (is_filled) {
    filled_blocks.Remove(block);
    partially_filled_blocks.PushBack(block);
  } else if (block->num_allocated == 0) {
    partially_filled_blocks.Remove(block);
    DeleteBlock(block, page_size);
  }
  // printf("Done\n");
}

}  // namespace fix_sized_allocator_impl
}  // namespace quick

