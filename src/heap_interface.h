#pragma once

#include <cstddef>
#include <cstdint>

namespace bench {

// Abstract interface for managing a single region of memory. Implementers are
// responsible for allocating memory, and passing a pointer to the beginning of
// the memory region to this class's constructor.
class Heap {
 protected:
  // `heap_start` is a pointer to the start of the region of memory this heap
  // manages.
  //
  // `size` is the maximum size that the heap can extend to in bytes. This is
  // typically the size of the `mmap`ped region for the heap.
  explicit Heap(void* heap_start, size_t size)
      : max_size_(size), heap_start_(heap_start), heap_end_(heap_start) {}

 public:
  Heap(Heap&& heap) noexcept
      : max_size_(heap.max_size_),
        heap_start_(heap.heap_start_),
        heap_end_(heap.heap_end_) {
    heap.heap_start_ = nullptr;
    heap.heap_end_ = nullptr;
  }

<<<<<<< HEAD
  virtual ~Heap() = default;

=======
>>>>>>> d3b973fd6e938786ae4ec0560b204de2d3ba8e58
  // Increments the size of the heap by `increment` bytes. The heap starts off
  // empty and must be increased by calling `sbrk()` before anything can be
  // written to it.
  //
  // On success, `sbrk()` returns the previous program break. (If the break was
  // increased, then this value is a pointer to the start of the newly allocated
  // memory). On error, `nullptr` is returned, and errno is set to ENOMEM.
  void* sbrk(intptr_t increment);

  // Resets the heap and returns a pointer to the beginning of the heap.
  void* Reset() {
    heap_end_ = heap_start_;
    return heap_start_;
  }

  // Returns the start of the heap.
  void* Start() const {
    return heap_start_;
  }

  // Returns the end of the heap.
  void* End() const {
    return heap_end_;
  }

  // Returns the number of allocated bytes in the heap.
  size_t Size() const {
    return static_cast<uint8_t*>(heap_end_) -
           static_cast<uint8_t*>(heap_start_);
  }

<<<<<<< HEAD
  size_t MaxSize() const {
    return max_size_;
  }

=======
>>>>>>> d3b973fd6e938786ae4ec0560b204de2d3ba8e58
 private:
  const size_t max_size_;
  void* heap_start_;
  void* heap_end_;
};

}  // namespace bench
