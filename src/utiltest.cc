<<<<<<< HEAD
#include <cstdint>

#include "absl/flags/flag.h"
#include "absl/status/statusor.h"

#include "src/allocator_interface.h"
#include "src/heap_factory.h"
#include "src/tracefile_reader.h"

ABSL_FLAG(bool, effective_util, false,
          "If set, uses a \"more fair\" measure of memory utilization, "
          "rounding up each allocation size to its alignment requirement.");

namespace bench {

size_t RoundUp(size_t size) {
  if (!absl::GetFlag(FLAGS_effective_util)) {
    return size;
  }

  if (size <= 8) {
    return 8;
  }
  return (size + 0xf) & ~0xf;
}

absl::StatusOr<double> MeasureUtilization(TracefileReader& reader,
                                          HeapFactory& heap_factory) {
  std::vector<std::pair<void*, uint64_t>> ptrs(
      reader.Tracefile().max_simultaneous_allocs());

  heap_factory.Reset();
  initialize_heap(heap_factory);

  size_t total_allocated_bytes = 0;
  size_t max_allocated_bytes = 0;
  size_t max_heap_size = 0;
  for (const TraceLine& line : reader) {
    switch (line.op_case()) {
      case TraceLine::kMalloc: {
        uint64_t size = line.malloc().input_size();
        void* ptr = malloc(size);
        ptrs[line.malloc().result_id()] = { ptr, size };
        total_allocated_bytes += RoundUp(size);
        break;
      }
      case TraceLine::kCalloc: {
        uint64_t nmemb = line.calloc().input_nmemb();
        uint64_t size = line.calloc().input_size();
        void* ptr = calloc(nmemb, size);
        size_t allocated_bytes = nmemb * size;
        ptrs[line.calloc().result_id()] = { ptr, allocated_bytes };
        total_allocated_bytes += RoundUp(allocated_bytes);
        break;
      }
      case TraceLine::kRealloc: {
        void* input_ptr = line.realloc().has_input_id()
                              ? ptrs[line.realloc().input_id()].first
                              : nullptr;
        if (line.realloc().has_input_id()) {
          total_allocated_bytes -=
              RoundUp(ptrs[line.realloc().input_id()].second);
          ptrs[line.realloc().input_id()].first = nullptr;
        }

        uint64_t size = line.realloc().input_size();
        void* new_ptr = realloc(input_ptr, size);
        total_allocated_bytes += RoundUp(size);
        ptrs[line.realloc().result_id()] = { new_ptr, size };
        break;
      }
      case TraceLine::kFree: {
        if (!line.free().has_input_id()) {
          free(nullptr);
          break;
        }

        free(ptrs[line.free().input_id()].first);
        total_allocated_bytes -= RoundUp(ptrs[line.free().input_id()].second);
        ptrs[line.free().input_id()].first = nullptr;
        break;
      }
      case TraceLine::OP_NOT_SET: {
        __builtin_unreachable();
=======
#include <optional>

#include "absl/container/flat_hash_map.h"
#include "absl/status/statusor.h"
#include "util/absl_util.h"

#include "src/allocator_interface.h"
#include "src/singleton_heap.h"
#include "src/tracefile_reader.h"

namespace bench {

absl::StatusOr<double> MeasureUtilization(const std::string& tracefile) {
  absl::flat_hash_map<void*, std::pair<void*, size_t>> id_to_ptrs;
  DEFINE_OR_RETURN(TracefileReader, reader, TracefileReader::Open(tracefile));

  SingletonHeap::GlobalInstance()->Reset();
  initialize_heap();

  size_t total_allocated_bytes = 0;
  size_t max_allocated_bytes = 0;
  while (true) {
    DEFINE_OR_RETURN(std::optional<TraceLine>, line, reader.NextLine());
    if (!line.has_value()) {
      break;
    }

    switch (line->op) {
      case TraceLine::Op::kMalloc: {
        void* ptr = malloc(line->input_size);
        id_to_ptrs[line->result] = { ptr, line->input_size };
        total_allocated_bytes += line->input_size;
        break;
      }
      case TraceLine::Op::kCalloc: {
        void* ptr = calloc(line->nmemb, line->input_size);
        id_to_ptrs[line->result] = { ptr, line->input_size };
        total_allocated_bytes += line->input_size;
        break;
      }
      case TraceLine::Op::kRealloc: {
        void* new_ptr =
            realloc(id_to_ptrs[line->input_ptr].first, line->input_size);
        if (line->input_ptr != nullptr) {
          total_allocated_bytes -= id_to_ptrs[line->input_ptr].second;
          id_to_ptrs.erase(line->input_ptr);
        }
        total_allocated_bytes += line->input_size;
        id_to_ptrs[line->result] = { new_ptr, line->input_size };
        break;
      }
      case TraceLine::Op::kFree: {
        free(id_to_ptrs[line->input_ptr].first);
        total_allocated_bytes -= id_to_ptrs[line->input_ptr].second;
        id_to_ptrs.erase(line->input_ptr);
        break;
>>>>>>> d3b973fd6e938786ae4ec0560b204de2d3ba8e58
      }
    }

    max_allocated_bytes = std::max(total_allocated_bytes, max_allocated_bytes);
<<<<<<< HEAD

    size_t heap_size = 0;
    for (const auto& heap : heap_factory.Instances()) {
      heap_size += heap->Size();
    }
    max_heap_size = std::max(heap_size, max_heap_size);
=======
>>>>>>> d3b973fd6e938786ae4ec0560b204de2d3ba8e58
  }

  if (total_allocated_bytes != 0) {
    for (const auto& [id, ptr] : id_to_ptrs) {
      printf("%p: %p %zu\n", id, ptr.first, ptr.second);
    }
    return absl::InternalError(
        "Tracefile does not free all the memory it allocates.");
  }

<<<<<<< HEAD
  return max_heap_size != 0
             ? static_cast<double>(max_allocated_bytes) / max_heap_size
             : -1;
=======
  return static_cast<double>(max_allocated_bytes) /
         SingletonHeap::GlobalInstance()->Size();
>>>>>>> d3b973fd6e938786ae4ec0560b204de2d3ba8e58
}

}  // namespace bench
