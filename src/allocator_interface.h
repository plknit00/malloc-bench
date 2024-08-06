#pragma once

#include <cassert>
#include <cstring>

#include "src/singleton_heap.h"

namespace bench {

#define MALLOC_ASSERT(cond) assert(cond)
// #define MALLOC_ASSERT(cond)

class Block {
 public:
  uint64_t GetSize() const {
    return header_ & ~0xf;
  }

  void SetSize(uint64_t size) {
    // check first 4 bits are 0, 16 byte alligned size
    MALLOC_ASSERT((size & 0xf) == 0);
    header_ = size | (header_ & 0x1);
  }

  bool IsFree() const {
    return (header_ & 0x1) == 0x1;
  }

  void SetFree(bool free) {
    if (free) {
      header_ = header_ | 0x1;
    } else {
      header_ = header_ & 0x0;
    }
  }

  uint8_t* GetBody() {
    return body_;
  }

 private:
  uint64_t header_;
  // flexible size, returns pointer to beginning of body
  uint8_t body_[];
};

// Called before any allocations are made.
inline void initialize_heap() {}

size_t space_needed_with_header(const size_t& size) {
  // add size of header to size, 8 bytes
  size_t round_up = size + 0xff;
  // round up size of memory needed to be 16 byte aligned
  // zero out first four bits
  round_up = round_up & ~0xf;
  return round_up;
}

size_t space_needed(const size_t& size) {
  // round up size of memory needed to be 16 byte aligned
  // zero out first four bits
  size_t round_up = size & ~0xf;
  return round_up;
}

// implicit implementation
// more asserts ????????????????????????
inline void* malloc(size_t size) {
  // allocating no space, do nothing
  if (size == 0) {
    return nullptr;
  }
  size_t new_size_w_header = space_needed_with_header(size);
  size_t new_size = space_needed(size);
  // heap is empty, allocate space
  if (SingletonHeap::GlobalInstance()->Size() == 0) {
    auto* block = reinterpret_cast<Block*>(
        SingletonHeap::GlobalInstance()->sbrk(new_size_w_header));
    block->SetSize(new_size);
    block->SetFree(false);
    return block;
  }
  // search current heap for free memory of size size
  auto* current_block =
      reinterpret_cast<Block*>(SingletonHeap::GlobalInstance()->Start());
  // does this while cond work or do i need to cast end????????????????
  while (current_block != SingletonHeap::GlobalInstance()->End()) {
    if (current_block->IsFree()) {
      if (current_block->GetSize() >= new_size_w_header) {
        auto* block = reinterpret_cast<Block*>(
            SingletonHeap::GlobalInstance()->sbrk(new_size_w_header));
        block->SetSize(new_size);
        block->SetFree(false);
        //  how to combine with current_block ????????????????????
        // overload = ?
        current_block = block;
        return current_block;
      }
    }
  }
  // need to increase heap size for this call
  auto* block = reinterpret_cast<Block*>(
      SingletonHeap::GlobalInstance()->sbrk(new_size_w_header));
  block->SetSize(new_size);
  block->SetFree(false);
  return block;
}

inline void* calloc(size_t nmemb, size_t size) {
  // TODO: implement
  void* ptr = malloc(nmemb * size);
  memset(ptr, 0, nmemb * size);
  return ptr;
}

inline void* realloc(void* ptr, size_t size) {
  // TODO: implement
  void* new_ptr = malloc(size);
  if (size > 0) {
    memcpy(new_ptr, ptr, size);
  }
  return new_ptr;
}

inline void free(void* ptr) {
  // TODO: implement
}

}  // namespace bench
