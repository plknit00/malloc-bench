#pragma once

#include <cassert>
#include <cstring>

#include "src/singleton_heap.h"

namespace bench {

#define MALLOC_ASSERT(cond) assert(cond)
// #define MALLOC_ASSERT(cond)

class Block {
 public:
  Block* create_block_extend_heap(size_t size) {
    size_t header_length = 8;
    body_* = reinterpret_cast<uint8_t*>(
        SingletonHeap::GlobalInstance()->sbrk(size + header_length));
    SetBlockSize(size);
    SetFree(false);
    return this;
  }

  Block* take_free_block(size_t size) {
    SetBlockSize(size);
    SetFree(false);
    return this;
  }

  uint64_t GetBlockSize() const {
    return header_ & ~0xf;
  }

  uint64_t GetUserSize() const {
    return GetBlockSize() - 8;
  }

  void SetBlockSize(uint64_t size) {
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

  size_t static space_needed_with_header(const size_t& size) {
    // add size of header to size, 8 bytes
    size_t round_up = size + 0xff;
    // round up size of memory needed to be 16 byte aligned
    // zero out first four bits
    round_up = round_up & ~0xf;
    return round_up;
  }

  size_t static space_needed(const size_t& size) {
    // round up size of memory needed to be 16 byte aligned
    // zero out first four bits
    size_t round_up = size & ~0xf;
    return round_up;
  }

  Block* GetNextBlock(Block* current_block) const {
    return current_block + GetBlockSize();
  }
  // which??????????????
  Block* GetNextBlock() const {
    return this + GetBlockSize();
  }

 private:
  uint64_t header_;
  // flexible size, returns pointer to beginning of body
  uint8_t body_[];
};

// Called before any allocations are made.
inline void initialize_heap() {}

// implicit implementation
// more asserts ????????????????????????
// check for consistency on size w and wo header
// what am i returning?
inline void* malloc(size_t size) {
  // allocating no space, do nothing
  if (size == 0) {
    return nullptr;
  }
  auto* start_block =
      reinterpret_cast<Block*>(SingletonHeap::GlobalInstance()->Start());
  // heap is empty, allocate space
  if (SingletonHeap::GlobalInstance()->Size() == 0) {
    start_block->create_block_extend_heap(size);
    return start_block;
  }
  auto* current_block = start_block;
  auto* end_block =
      reinterpret_cast<Block*>(SingletonHeap::GlobalInstance()->End());
  // search current heap for free memory of size size
  while (current_block != end_block) {
    if (current_block->IsFree()) {
      if (current_block->GetUserSize() >= size) {
        current_block->take_free_block(size);
        return current_block;
      }
    }
    // WHAT??????????????????????
    current_block = current_block->GetNextBlock(current_block);
  }
  // need to increase heap size for this call
  // could be current block but must assert this equals end block???? maybe
  // doesnt matter
  end_block->create_block_extend_heap(size);
  return end_block;
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
