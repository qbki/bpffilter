cmake_minimum_required(VERSION 3.27)

project(bpffilter VERSION 0.0.1)

set(CMAKE_C_COMPILER "clang")
set(CMAKE_C_OUTPUT_EXTENSION_REPLACE 1)
set(CMAKE_C_OUTPUT_EXTENSION ".o")
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_FLAGS_INIT "-Wall -std=c++20")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(GENERATED_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/include")
set(GENERATED_VMLINUX_H "${GENERATED_INCLUDE_DIR}/vmlinux.h")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

include_directories(SYSTEM ${GENERATED_INCLUDE_DIR})

function(add_flag target flag)
  target_compile_options(${target} PRIVATE ${flag})
endfunction()

function(add_common_flags target)
  add_flag(${target} "-O2")
  add_flag(${target} "-Wall")
  add_flag(${target} "-Wextra")
  add_flag(${target} "-Wpedantic")
  add_flag(${target} "-pedantic-errors")
endfunction()

file(MAKE_DIRECTORY ${GENERATED_INCLUDE_DIR})

add_library(MAIN_BPF OBJECT "${CMAKE_SOURCE_DIR}/src/main.bpf.c")
add_flag(MAIN_BPF "-g") # Required. You can't use BPF maps without it.
add_flag(MAIN_BPF "-O2") # Required. Invalid opcodes are generated without it.
# begin : this is one argument, add_flag has a naive implementation...
add_flag(MAIN_BPF "-target")
add_flag(MAIN_BPF "bpf")
# end
add_flag(MAIN_BPF "-D__TARGET_ARCH_x86_64")

add_custom_target(dummy_target ALL
  COMMAND ${CMAKE_COMMAND} -E env sh -c
          "bpftool gen skeleton $<TARGET_OBJECTS:MAIN_BPF> > ${GENERATED_INCLUDE_DIR}/main.skel.h"
  VERBATIM
)

add_executable(${PROJECT_NAME}
  "${CMAKE_SOURCE_DIR}/src/main.cpp"
  "${CMAKE_SOURCE_DIR}/src/utils.cpp"
  "${CMAKE_SOURCE_DIR}/src/cmdline.cpp"
  "${CMAKE_SOURCE_DIR}/src/bpf_program.cpp"
  "${CMAKE_SOURCE_DIR}/src/xdp_program.cpp"
)
target_link_libraries(${PROJECT_NAME} bpf xdp elf z)

add_common_flags(${PROJECT_NAME})
add_flag(${PROJECT_NAME} "-std=c++20")
