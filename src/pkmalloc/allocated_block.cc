#include "src/pkmalloc/allocated_block.h"

#include <cstddef>

#include "src/singleton_heap.h"

uint8_t* AllocatedBlock::GetBody() {
  return body_;
}

AllocatedBlock* AllocatedBlock::FromRawPtr(void* ptr) {
  return reinterpret_cast<AllocatedBlock*>(reinterpret_cast<uint8_t*>(ptr) -
                                           offsetof(AllocatedBlock, body_));
}

AllocatedBlock* AllocatedBlock::take_free_block() {
  // fix later, if you change size to be smaller, the remainder of this block
  // should be made to be a new free block SetBlockSize(size);
  // should take parameter size_t size in the future to allow user to change
  // size of allocated block
  Block::SetFree(false);
  return this;
}

AllocatedBlock* AllocatedBlock::create_block_extend_heap(size_t size) {
  size_t block_size = AllocatedBlock::space_needed_with_header(size);
  auto* block = reinterpret_cast<AllocatedBlock*>(
      bench::SingletonHeap::GlobalInstance()->sbrk(block_size));
  block->SetBlockSize(block_size);
  block->SetFree(false);
  return block;
}

size_t AllocatedBlock::space_needed_with_header(const size_t& size) {
  // add size of header to size, 8 bytes
  size_t round_up = size + sizeof(AllocatedBlock);
  // round up size of memory, needs to be 16 byte aligned
  round_up += 0xf;
  // zero out first four bits
  round_up = round_up & ~0xf;
  return round_up;
}

AllocatedBlock* AllocatedBlock::free_to_alloc(FreeBlock* current_block) {
  current_block->SetFree(false);
  auto* result = reinterpret_cast<AllocatedBlock*>(current_block);
  return result;
}

FreeBlock* AllocatedBlock::alloc_to_free(AllocatedBlock* current_block) {
  current_block->SetFree(true);
  auto* result = reinterpret_cast<FreeBlock*>(current_block);
  return result;
}