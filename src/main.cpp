#include <bpf/bpf.h>
#include <thread>
#include <chrono>
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
#include "common.h"

const std::chrono::seconds SLEEP_INTERVAL {1};
static volatile sig_atomic_t IS_RUNNING = 1; // NOLINT

using BpfProgramPtr = std::unique_ptr<main_bpf, std::function<void(main_bpf*)>>;
using XdpProgramPtr = std::unique_ptr<xdp_program, std::function<void(xdp_program*)>>;
struct BpfFileDescriptor {
  int descriptor;
};

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

/**
 * Returns a BPF file descriptor based on a map name.
 */
BpfFileDescriptor find_map_fd(bpf_object *bpf_obj, const std::string& map_name)
{
	auto map = bpf_object__find_map_by_name(bpf_obj, map_name.c_str());
  if (!map) {
    throw std::runtime_error(std::format("Can't find a map by name: {}", map_name));
	}
	auto file_descriptor = bpf_map__fd(map);
  if (file_descriptor < 0) {
    throw std::runtime_error(std::format("Can't get a file descriptor of a map: {}", map_name));
  }
  return { file_descriptor };
}

InterfaceData get_kernel_value(BpfFileDescriptor fd, __u32 key)
{
  InterfaceData result {};
  auto error = bpf_map_lookup_elem(fd.descriptor, &key, &result);
	if (error != 0) {
    throw std::runtime_error(std::format("Can't map data from the kernel program: {}", key));
	}
  return result;
}

void run_polling(XdpProgramPtr& program) {
  auto map_df = find_map_fd(xdp_program__bpf_obj(program.get()), "xdp_stats_map");
  while (IS_RUNNING) {
    auto value = get_kernel_value(map_df, KEY);
    print(std::format("Packets: {}", value.received_packets));
    print(std::format("Bytes: {}", value.received_bytes));
    std::this_thread::sleep_for(SLEEP_INTERVAL);
  }
}

int main()
{
  // Ctrl+C handling
  signal(SIGINT, finish_application);
  try {
    long error = 0;

    BpfProgramPtr bpf_program {main_bpf::open(), bpf_deleter};
    if (!bpf_program) {
      throw std::runtime_error("Can't open eBPF program");
    }

    XdpProgramPtr xdp_program {xdp_program__from_bpf_obj(bpf_program->obj, "xdp"), xdp_deleter};
    error = libxdp_get_error(xdp_program.get());
    if (error) {
      throw std::runtime_error("Can't create XDP program");
    }

    error = xdp_program__attach(xdp_program.get(), 1 /*lo or localhost*/, XDP_MODE_UNSPEC, 0);
    if (error) {
      throw std::runtime_error(std::format("Can't attach XDP program to interface. Err code: {}", error));
    }

    run_polling(xdp_program);
  } catch(std::runtime_error& exception) {
    print_err(exception.what());
  }
}
