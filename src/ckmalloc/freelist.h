#pragma once

#include <cstddef>
#include <cstring>

#include "src/ckmalloc/block.h"
#include "src/ckmalloc/common.h"
#include "src/ckmalloc/linked_list.h"
#include "src/ckmalloc/slab_manager.h"
#include "src/ckmalloc/slab_map.h"

namespace ckmalloc {

template <MetadataAllocInterface MetadataAlloc, SlabMapInterface SlabMap,
          SlabManagerInterface SlabManager>
class FreelistImpl {
 public:
  FreelistImpl(SlabMap* slab_map, SlabManager* slab_manager)
      : slab_map_(slab_map), slab_manager_(slab_manager) {}

  // Allocates a region of memory `user_size` bytes long, returning a pointer to
  // the beginning of the region.
  void* Alloc(size_t user_size);

  // Re-allocates a region of memory to be `user_size` bytes long, returning a
  // pointer to the beginning of the new region and copying the data from `ptr`
  // over. The returned pointer may equal the `ptr` argument. If `user_size` is
  // larger than the previous size of the region starting at `ptr`, the
  // remaining data after the size of the previous region is uninitialized, and
  // if `user_size` is smaller, the data is truncated.
  void* Realloc(void* ptr, size_t user_size);

  // Frees a region of memory returned from `Alloc`, allowing that memory to be
  // reused by future `Alloc`s.
  void Free(void* ptr);

 private:
  // Tries to find a free block large enough for `user_size`, and if one is
  // found, returns the `AllocatedBlock` large enough to serve this request.
  AllocatedBlock* MakeBlockFromFreelist(size_t user_size);

  // Searches the freelists for a block large enough to fit `user_size`. If none
  // is found, `nullptr` is returned.
  FreeBlock* FindFree(size_t user_size);

  // Allocates a new large slab large enough for `user_size`, and returns a
  // pointer to the newly created `AllocatedBlock` that is large enough for
  // `user_size`.
  AllocatedBlock* AllocLargeSlabAndMakeBlock(size_t user_size);

  SlabMap* slab_map_;

  SlabManager* slab_manager_;

  LinkedList<FreeBlock> free_blocks_;
};

template <MetadataAllocInterface MetadataAlloc, SlabMapInterface SlabMap,
          SlabManagerInterface SlabManager>
void* FreelistImpl<MetadataAlloc, SlabMap, SlabManager>::Alloc(
    size_t user_size) {
  AllocatedBlock* block = MakeBlockFromFreelist(user_size);

  // If allocating from the freelist fails, we need to request another slab of
  // memory.
  if (block == nullptr) {
    block = AllocLargeSlabAndMakeBlock(user_size);
  }

  return block != nullptr ? block->UserDataPtr() : nullptr;
}

template <MetadataAllocInterface MetadataAlloc, SlabMapInterface SlabMap,
          SlabManagerInterface SlabManager>
void* FreelistImpl<MetadataAlloc, SlabMap, SlabManager>::Realloc(
    void* ptr, size_t user_size) {
  void* new_ptr = Alloc(user_size);
  if (new_ptr != nullptr) {
    // TODO: copy min of sizes.
    std::memcpy(new_ptr, ptr, user_size);
  }
  Free(ptr);
  return new_ptr;
}

template <MetadataAllocInterface MetadataAlloc, SlabMapInterface SlabMap,
          SlabManagerInterface SlabManager>
void FreelistImpl<MetadataAlloc, SlabMap, SlabManager>::Free(void* ptr) {}

template <MetadataAllocInterface MetadataAlloc, SlabMapInterface SlabMap,
          SlabManagerInterface SlabManager>
AllocatedBlock*
FreelistImpl<MetadataAlloc, SlabMap, SlabManager>::MakeBlockFromFreelist(
    size_t user_size) {
  FreeBlock* free_block = FindFree(user_size);
  if (free_block == nullptr) {
    return nullptr;
  }

  AllocatedBlock* allocated = free_block->MarkAllocated();
  // TODO: split.
  return allocated;
}

template <MetadataAllocInterface MetadataAlloc, SlabMapInterface SlabMap,
          SlabManagerInterface SlabManager>
FreeBlock* FreelistImpl<MetadataAlloc, SlabMap, SlabManager>::FindFree(
    size_t user_size) {
  for (FreeBlock& block : free_blocks_) {
    // Take the first block that fits.
    if (block.UserDataSize() >= user_size) {
      return &block;
    }
  }

  return nullptr;
}

template <MetadataAllocInterface MetadataAlloc, SlabMapInterface SlabMap,
          SlabManagerInterface SlabManager>
AllocatedBlock*
FreelistImpl<MetadataAlloc, SlabMap, SlabManager>::AllocLargeSlabAndMakeBlock(
    size_t user_size) {
  uint32_t n_pages = LargeSlab::NPagesForBlock(user_size);
  auto result = slab_manager_->template Alloc<LargeSlab>(n_pages);
  if (result == std::nullopt) {
    return nullptr;
  }
  LargeSlab* slab = result->second;

  uint64_t block_size = Block::BlockSizeForUserSize(user_size);
  AllocatedBlock* block =
      slab_manager_->FirstBlockInLargeSlab(slab)->InitAllocated(block_size,
                                                                false);
  block->NextAdjacentBlock()->InitFree(
      n_pages * kPageSize - block_size - Block::kFirstBlockInSlabOffset,
      free_blocks_);

  return block;
}

using Freelist = FreelistImpl<GlobalMetadataAlloc, SlabMap, SlabManager>;

}  // namespace ckmalloc