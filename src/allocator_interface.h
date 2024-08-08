#pragma once

#include <cassert>
#include <cstddef>
#include <cstring>

#include "src/singleton_heap.h"

namespace bench {

#define MALLOC_ASSERT(cond) assert(cond)
#define MALLOC_ASSERT_EQ(lhs, rhs)                                          \
  do {                                                                      \
    auto __l = (lhs);                                                       \
    auto __r = (rhs);                                                       \
    if (__l != __r) {                                                       \
      std::cerr << __FILE__ << ":" << __LINE__ << ": Expected equality of " \
                << std::hex << __l << " and " << __r << std::endl;          \
    }                                                                       \
  } while (0)

// #define MALLOC_ASSERT(cond)

class Block {
 public:
  // Block* take_free_block(size_t size) {
  Block* take_free_block() {
    // fix later, if you change size to be smaller, the remainder of this block
    // should be made to be a new free block SetBlockSize(size);
    SetFree(false);
    return this;
  }

  uint64_t GetBlockSize() const {
    uint64_t block_size = header_ & ~0xf;
    CheckValid();
    MALLOC_ASSERT(block_size < SingletonHeap::kHeapSize);
    MALLOC_ASSERT(block_size != 0);
    return block_size;
  }

  uint64_t GetUserSize() const {
    return GetBlockSize() - 8;
  }

  void SetBlockSize(uint64_t size) {
    // check first 4 bits are 0, 16 byte alligned size
    MALLOC_ASSERT((size & 0xf) == 0);
    MALLOC_ASSERT(size != 0);
    header_ = size | (header_ & 0x1);
  }

  bool IsFree() const {
    return (header_ & 0x1) == 0x1;
  }

  void SetFree(bool free) {
    if (free) {
      header_ = header_ | 0x1;
    } else {
      header_ = header_ & ~0x1;
    }
  }

  void SetMagic() {
    magic_value = 123456;
  }

  uint8_t* GetBody() {
    return body_;
  }

  static Block* FromRawPtr(void* ptr) {
    return reinterpret_cast<Block*>(reinterpret_cast<uint8_t*>(ptr) -
                                    offsetof(Block, body_));
  }

  void CheckValid() const {
    MALLOC_ASSERT_EQ(magic_value, 123456);
  }

  Block* GetNextBlock() {
    return reinterpret_cast<Block*>(reinterpret_cast<uint8_t*>(this) +
                                    GetBlockSize());
  }

 private:
  uint64_t header_;
  uint64_t magic_value = 123456;
  uint64_t garbage = -0;
  // flexible size, returns pointer to beginning of body
  uint8_t body_[];
};

static size_t space_needed_with_header(const size_t& size) {
  // add size of header to size, 8 bytes
  size_t round_up = size + sizeof(Block);
  // round up size of memory needed to be 16 byte aligned
  round_up += 0xf;
  // zero out first four bits
  round_up = round_up & ~0xf;
  return round_up;
}

/*
static size_t space_needed(const size_t& size) {
  // round up size of memory needed to be 16 byte aligned
  // zero out first four bits
  size_t round_up = size & ~0xf;
  return round_up;
}
*/

static Block* create_block_extend_heap(size_t size) {
  size_t block_size = space_needed_with_header(size);
  auto* block = reinterpret_cast<Block*>(
      SingletonHeap::GlobalInstance()->sbrk(block_size));
  block->SetBlockSize(block_size);
  block->SetFree(false);
  block->SetMagic();
  return block;
}

// Called before any allocations are made.
inline void initialize_heap() {}

// implicit implementation
// more asserts ??
// check for consistency on size w and wo header
// is my return value correct?
// how do i know thw blocks im creating/editing are contiguous in memory? does
// singletonheap manage that?
inline void* malloc(size_t size) {
  // allocating no space, do nothing
  if (size == 0) {
    return nullptr;
  }
  // heap is empty, allocate space
  if (SingletonHeap::GlobalInstance()->Size() == 0) {
    // to align
    SingletonHeap::GlobalInstance()->sbrk(8);
    Block* start_block = create_block_extend_heap(size);
    start_block->CheckValid();
    return start_block->GetBody();
  }
  auto* start_block = reinterpret_cast<Block*>(
      reinterpret_cast<uint8_t*>(SingletonHeap::GlobalInstance()->Start()) + 8);
  auto* current_block = start_block;
  auto* end_block =
      reinterpret_cast<Block*>(SingletonHeap::GlobalInstance()->End());
  // search current heap for free memory of size size
  while (current_block < end_block) {
    current_block->CheckValid();
    if (current_block->IsFree()) {
      current_block->CheckValid();
      if (current_block->GetBlockSize() >= size) {
        // current_block->take_free_block(size);
        current_block->take_free_block();
        current_block->CheckValid();
        return current_block->GetBody();
      }
    }
    auto* temp = current_block;
    current_block = current_block->GetNextBlock();
    MALLOC_ASSERT(current_block > temp);
  }
  // need to increase heap size for this call
  // could be current block but must assert this equals end block???? maybe
  // doesnt matter
  end_block = create_block_extend_heap(size);
  end_block->CheckValid();
  return end_block->GetBody();
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
  if (ptr == nullptr) {
    return;
  }
  Block* block = Block::FromRawPtr(ptr);
  block->CheckValid();
  block->SetFree(true);
}

}  // namespace bench

// add comments about allocating at certain address and freeing at certain
// address *******************************
