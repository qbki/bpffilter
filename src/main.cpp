#include <array>
#include <bpf/bpf.h>
#include <bpf/libbpf.h>
#include <fcntl.h>
#include <format>
#include <functional>
#include <memory>
#include <stdexcept>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "main.skel.h"
#include "utils.hxx"

const int EBPF_BUFFER_SIZE = 1024;

void read_trace_pipe()
{
  auto trace_fd = open("/sys/kernel/debug/tracing/trace_pipe", O_RDONLY, 0);
  if (trace_fd < 0) {
    return;
  }
  while (true) {
    static std::array<char, EBPF_BUFFER_SIZE> buffer;
    auto size = read(trace_fd, buffer.data(), EBPF_BUFFER_SIZE - 1);
    if (size > 0) {
      buffer[size] = 0; // NOLINT
      puts(buffer.data());
    }
  }
}

void ebpf_deleter(main_bpf* bpf_ptr) {
  main_bpf::destroy(bpf_ptr);
  print("The eBPF program was released");
}

using BpfProgramPtr = std::unique_ptr<main_bpf, std::function<void(main_bpf*)>>;

int main()
{
  try {
    int error = 0;

    BpfProgramPtr obj {main_bpf::open(), ebpf_deleter};
    if (!obj) {
      throw std::runtime_error("Can't open eBPF program");
    }

    error = main_bpf::load(obj.get());
    if (error) {
      throw std::runtime_error(std::format("Can't load eBPF program. Err code: {}", error));
    }

    error = main_bpf::attach(obj.get());
    if (error) {
      throw std::runtime_error(std::format("Can't attach eBPF program. Err code: {}", error));
    }

    read_trace_pipe();
  } catch(std::runtime_error& exception) {
    print_err(exception.what());
  }
}
