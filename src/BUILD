cc_binary(
    name = "driver",
    srcs = ["driver.cc"],
    data = [
        "//traces",
    ],
    deps = [
        ":correctness_checker",
        ":perftest",
        ":tracefile_reader",
        "@cc-util//util:gtest_util",
        ":utiltest",
        "@abseil-cpp//absl/flags:flag",
        "@abseil-cpp//absl/flags:parse",
        "@abseil-cpp//absl/status",
        "@abseil-cpp//absl/strings",
        "@cc-util//util:absl_util",
    ],
)

cc_library(
    name = "perftest",
    srcs = ["perftest.cc"],
    hdrs = ["perftest.h"],
    data = [
        "//traces",
    ],
    deps = [
        ":allocator_interface",
        ":singleton_heap",
        ":tracefile_reader",
        "@cc-util//util:gtest_util",
        "@abseil-cpp//absl/container:flat_hash_map",
        "@abseil-cpp//absl/status:statusor",
        "@abseil-cpp//absl/time",
        "@cc-util//util:absl_util",
    ],
)

cc_library(
    name = "utiltest",
    srcs = ["utiltest.cc"],
    hdrs = ["utiltest.h"],
    data = [
        "//traces",
    ],
    deps = [
        ":allocator_interface",
        ":singleton_heap",
        ":tracefile_reader",
        "@cc-util//util:gtest_util",
        "@abseil-cpp//absl/container:flat_hash_map",
        "@abseil-cpp//absl/status:statusor",
        "@cc-util//util:absl_util",
    ],
)

cc_library(
    name = "allocator_interface",
    hdrs = ["allocator_interface.h"],
    deps = [
        ":singleton_heap",
    ],
)

cc_library(
    name = "correctness_checker",
    srcs = ["correctness_checker.cc"],
    hdrs = ["correctness_checker.h"],
    deps = [
        ":allocator_interface",
        ":rng",
        ":singleton_heap",
        ":tracefile_reader",
        "@cc-util//util:gtest_util",
        "@abseil-cpp//absl/container:btree",
        "@abseil-cpp//absl/container:flat_hash_map",
        "@abseil-cpp//absl/status",
        "@abseil-cpp//absl/strings",
        "@abseil-cpp//absl/strings:str_format",
        "@cc-util//util:absl_util",
    ],
)

cc_test(
    name = "correctness_test",
    srcs = ["correctness_test.cc"],
    data = [
        "//traces",
    ],
    deps = [
        ":correctness_checker",
        "@cc-util//util:gtest_util",
        "@abseil-cpp//absl/status",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "singleton_heap",
    srcs = ["singleton_heap.cc"],
    hdrs = ["singleton_heap.h"],
    visibility = ["//src:__subpackages__"],
    deps = [
        ":heap_interface",
        "@abseil-cpp//absl/status:statusor",
        "@abseil-cpp//absl/strings:str_format",
    ],
)

cc_library(
    name = "heap_interface",
    srcs = ["heap_interface.cc"],
    hdrs = ["heap_interface.h"],
    visibility = ["//src:__subpackages__"],
    deps = [
    ],
)

cc_library(
    name = "sim_heap",
    hdrs = ["sim_heap.h"],
    visibility = ["//src:__subpackages__"],
    deps = [
        ":heap_interface",
    ],
)

cc_library(
    name = "tracefile_reader",
    srcs = ["tracefile_reader.cc"],
    hdrs = ["tracefile_reader.h"],
    deps = [
        "@cc-util//util:gtest_util",
        "@abseil-cpp//absl/status:statusor",
        "@abseil-cpp//absl/strings",
        "@cc-util//util:absl_util",
    ],
)

cc_library(
    name = "rng",
    hdrs = ["rng.h"],
    deps = [
        "@abseil-cpp//absl/status:statusor",
    ],
)
