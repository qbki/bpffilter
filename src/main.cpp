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

#include "cmdline.hxx"
#include "main.skel.h"
#include "utils.hxx"
#include "common.h"
#include "xdp_program.hxx"

const unsigned short DEFAULT_PORT = 0;
const std::chrono::seconds SLEEP_INTERVAL {1};
static volatile sig_atomic_t IS_RUNNING = 1; // NOLINT

using BpfProgramPtr = std::unique_ptr<main_bpf, std::function<void(main_bpf*)>>;
struct BpfFileDescriptor {
  int descriptor;
};

void bpf_deleter(main_bpf* bpf_ptr) {
  main_bpf::destroy(bpf_ptr);
  print("The eBPF program was released");
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

StatData get_kernel_value(BpfFileDescriptor fd, __u32 key)
{
  StatData result {};
  auto error = bpf_map_lookup_elem(fd.descriptor, &key, &result);
	if (error != 0) {
    throw std::runtime_error(std::format("Can't map data from the kernel program: {}", key));
	}
  return result;
}

void configure_kernel_program(XdpProgram& program, const CmdLineOptions& options) {
  ConfigData config {
    .dst_port = options.dst_port.value_or(DEFAULT_PORT),
  };
  auto map_fd = find_map_fd(program.bpf_object_ptr(), "xdp_config_map");
  auto error = bpf_map_update_elem(map_fd.descriptor, &KEY, &config, 0);
  if (error < 0) {
    throw std::runtime_error("Can't send configuration to the kernel program");
  }
}

void run_polling(XdpProgram& program) {
  auto map_fd = find_map_fd(program.bpf_object_ptr(), "xdp_stats_map");
  while (IS_RUNNING) {
    auto value = get_kernel_value(map_fd, KEY);
    print(std::format("Packets: {}", value.received_packets));
    print(std::format("Bytes: {}", value.received_bytes));
    std::this_thread::sleep_for(SLEEP_INTERVAL);
  }
}

int main(int argc, char **argv) {
  // Ctrl+C handling
  signal(SIGINT, finish_application);
  try {
    auto config = parse_cmdline_options(argc, argv);
    BpfProgramPtr bpf_program {main_bpf::open(), bpf_deleter};
    if (!bpf_program) {
      throw std::runtime_error("Can't open eBPF program");
    }
    auto if_index = static_cast<int>(config.interface_index);
    auto xdp_program = XdpProgram::from_bpf_obj(bpf_program->obj, if_index);
    xdp_program.attach();
    configure_kernel_program(xdp_program, config);
    run_polling(xdp_program);
  } catch(std::runtime_error& exception) {
    print_err(exception.what());
  } catch(...) {
    print_err("Unexpected error");
  }
}
