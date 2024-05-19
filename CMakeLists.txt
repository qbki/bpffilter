cmake_minimum_required(VERSION 3.27)

project(bpffilter VERSION 1.0.1)

# begin : Fetch Catch2 - a unit test library
# Keep it above compiler flags, or you stuck in an
# infinite loop of cmake's reconfiguration.
include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0
)
FetchContent_MakeAvailable(Catch2)
# end

set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_CXX_FLAGS_INIT "-std=gnu17")
set(CMAKE_C_OUTPUT_EXTENSION_REPLACE 1)
set(CMAKE_C_OUTPUT_EXTENSION ".o")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
set(CMAKE_CXX_FLAGS_INIT "-std=c++20")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(GENERATED_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/include")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

function(add_flag target flag)
  target_compile_options(${target} PRIVATE ${flag})
endfunction()

function(add_common_flags target)
  add_flag(${target} "-Wall")
  add_flag(${target} "-Wextra")
endfunction()

file(MAKE_DIRECTORY ${GENERATED_INCLUDE_DIR})

add_library(MAIN_BPF OBJECT "${CMAKE_SOURCE_DIR}/src/kernel/main.bpf.c")
# Required. You can't use BPF maps without it.
add_flag(MAIN_BPF "-g")
# Required. Invalid opcodes are generated without it.
add_flag(MAIN_BPF "-O2")
# begin : this is one argument, add_flag has a naive implementation...
add_flag(MAIN_BPF "-target")
add_flag(MAIN_BPF "bpf")
# end
add_flag(MAIN_BPF "-D__TARGET_ARCH_x86_64")
add_common_flags(MAIN_BPF)

add_custom_target(dummy_target ALL
  COMMAND ${CMAKE_COMMAND} -E env sh -c
          "bpftool gen skeleton $<TARGET_OBJECTS:MAIN_BPF> > ${GENERATED_INCLUDE_DIR}/main.skel.h"
  VERBATIM
)

include_directories(SYSTEM ${GENERATED_INCLUDE_DIR})
add_executable(${PROJECT_NAME}
  "${CMAKE_SOURCE_DIR}/src/bpf_program.cpp"
  "${CMAKE_SOURCE_DIR}/src/cmdline.cpp"
  "${CMAKE_SOURCE_DIR}/src/main.cpp"
  "${CMAKE_SOURCE_DIR}/src/screen.cpp"
  "${CMAKE_SOURCE_DIR}/src/utils.cpp"
  "${CMAKE_SOURCE_DIR}/src/xdp_program.cpp"
)
target_link_libraries(${PROJECT_NAME} xdp bpf elf zstd z)
target_link_options(${PROJECT_NAME} PRIVATE -stdlib=libstdc++ -static)

add_executable(tests
  "${CMAKE_SOURCE_DIR}/src/kernel/tests/collect_addresses.cpp"
  "${CMAKE_SOURCE_DIR}/src/kernel/tests/collect_ports.cpp"
  "${CMAKE_SOURCE_DIR}/src/kernel/tests/collect_proto.cpp"
  "${CMAKE_SOURCE_DIR}/src/kernel/tests/remove_bits.cpp"
)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)

add_common_flags(${PROJECT_NAME})
add_flag(${PROJECT_NAME} "-Wpedantic")
add_flag(${PROJECT_NAME} "-pedantic-errors")
add_flag(${PROJECT_NAME} "-std=c++20")

# Packing
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_GENERATOR "ZIP")
set(CPACK_OUTPUT_FILE_PREFIX ${CMAKE_SOURCE_DIR}/build/pack)
include(CPack)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)
