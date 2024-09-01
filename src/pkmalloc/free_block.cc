#include "src/pkmalloc/free_block.h"

void FreeBlock::SetNext(FreeBlock* current_block, FreeBlock* next) {
  current_block->next_ = next;
}

FreeBlock* FreeBlock::GetNext(FreeBlock* current_block) {
  return current_block->next_;
}

void FreeBlock::RemoveNext(FreeBlock* current_block, FreeBlock* next) {
  SetNext(current_block, GetNext(next));
}

FreeBlock* FreeBlock::combine(FreeBlock* left_block, FreeBlock* right_block) {
  // Do I need to track that address left < address right?
  // this should happen in free list - edit free list struc
  // merge two free blocks
  left_block->SetFree(true);
  left_block->SetBlockSize(left_block->GetBlockSize() +
                           right_block->GetBlockSize());
  RemoveNext(left_block, right_block);
  left_block->next_ = right_block->next_;
}

void FreeBlock::coalesce(FreeBlock* current, FreeBlock* prev) {
  // check in both directions for free blocks, if free, combine
  if (prev != nullptr) {
    if (prev->IsFree()) {
      current = combine(prev, current);
    }
  }
  if (current->next_ != nullptr) {
    if (current->next_->IsFree()) {
      current = combine(current, current->next_);
    }
  }
  // update current free list to make these blocks only one in the linked list?
  // return current;
}

FreeBlock* FreeBlock::alloc_to_free(AllocatedBlock* current_block) {
  current_block->SetFree(true);
  auto* result = reinterpret_cast<FreeBlock*>(current_block);
  return result;
}