#include <array>
#include <bpf/bpf.h>
#include <bpf/libbpf.h>
#include <csignal>
#include <fcntl.h>
#include <format>
#include <functional>
#include <memory>
#include <stdexcept>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <xdp/libxdp.h>

#include "main.skel.h"
#include "utils.hxx"

const int EBPF_BUFFER_SIZE = 1024;
static volatile sig_atomic_t IS_RUNNING = 1; // NOLINT

void read_trace_pipe()
{
  auto trace_fd = open("/sys/kernel/debug/tracing/trace_pipe", O_RDONLY, 0);
  if (trace_fd < 0) {
    return;
  }
  while (IS_RUNNING) {
    static std::array<char, EBPF_BUFFER_SIZE> buffer;
    auto size = read(trace_fd, buffer.data(), EBPF_BUFFER_SIZE - 1);
    if (size > 0) {
      buffer[size] = 0; // NOLINT
      puts(buffer.data());
    }
  }
}

void bpf_deleter(main_bpf* bpf_ptr) {
  main_bpf::destroy(bpf_ptr);
  print("The eBPF program was released");
}

void xdp_deleter(xdp_program* xdp_ptr) {
  xdp_program__detach(xdp_ptr, 1, XDP_MODE_UNSPEC, 0);
  xdp_program__close(xdp_ptr);
  print("The XDP program was released");
}

void finish_application(int) {
  IS_RUNNING = 0;
}

using BpfProgramPtr = std::unique_ptr<main_bpf, std::function<void(main_bpf*)>>;
using XdpProgramPtr = std::unique_ptr<xdp_program, std::function<void(xdp_program*)>>;

int main()
{
  // Ctrl+C handling
  signal(SIGINT, finish_application);
  try {
    long error = 0;

    BpfProgramPtr obj {main_bpf::open(), bpf_deleter};
    if (!obj) {
      throw std::runtime_error("Can't open eBPF program");
    }

    XdpProgramPtr xdp {xdp_program__from_bpf_obj(obj->obj, "xdp"), xdp_deleter};
    error = libxdp_get_error(xdp.get());
    if (error) {
      throw std::runtime_error("Can't create XDP program");
    }

    error = xdp_program__attach(xdp.get(), 1, XDP_MODE_UNSPEC, 0);
    if (error) {
      throw std::runtime_error(std::format("Can't attach XDP program to interface. Err code: {}", error));
    }

    read_trace_pipe();
  } catch(std::runtime_error& exception) {
    print_err(exception.what());
  }
}
