#include "src/ckmalloc/allocator.h"

#include <cstdint>

#include "src/ckmalloc/util.h"
#include "src/heap_interface.h"

namespace ckmalloc {

Allocator::Allocator(bench::Heap* heap) : heap_(heap) {
  CK_ASSERT(heap != nullptr);
  CK_ASSERT(heap->Start() == heap->End());
}

void* Allocator::Alloc(size_t size, std::align_val_t alignment) {
  // Alignment must be a power of two.
  CK_ASSERT((static_cast<size_t>(alignment) &
             (static_cast<size_t>(alignment) - 1)) == 0);
  // Size must already be aligned to `alignment`.
  CK_ASSERT((size & (static_cast<size_t>(alignment) - 1)) == 0);

  auto current_end = reinterpret_cast<uintptr_t>(heap_->End());
  auto align_mask = static_cast<uintptr_t>(alignment) - 1;
  uintptr_t alignment_increment = (~current_end + 1) & align_mask;

  void* alloc_start = static_cast<uint8_t*>(heap_->End()) + alignment_increment;
  void* new_end = heap_->sbrk(alignment_increment + size);
  if (new_end == nullptr) {
    // We do not have enough remaining memory in the heap to allocate this size.
    return nullptr;
  }

  return alloc_start;
}

}  // namespace ckmalloc