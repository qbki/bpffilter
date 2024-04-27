#include <bpf/bpf.h>
#include <cstdint>
#include <exception>
#include <optional>
#include <thread>
#include <chrono>
#include <bpf/libbpf.h>
#include <csignal>
#include <fcntl.h>
#include <format>
#include <stdexcept>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <xdp/libxdp.h>

#include "cmdline.hxx"
#include "main.skel.h"
#include "utils.hxx"
#include "common.h"
#include "bpf_program.hxx"
#include "xdp_program.hxx"

// DEFAULT_PORT and DEFAULT_ADDRESS should not be
// used in logic, it is just a placeholder.
const uint16_t DEFAULT_PORT = 0;
const uint32_t DEFAULT_ADDRESS = 0;

const std::chrono::seconds SLEEP_INTERVAL {1};
static volatile sig_atomic_t IS_RUNNING = 1; // NOLINT

StatData get_stats(XdpProgram::BpfFileDescriptor fd);
void configure_kernel_program(XdpProgram& program,
                              const CmdLineOptions& options);
void finish_application(int);
void run_polling(XdpProgram& program);

int
main(int argc, char **argv)
{
  if (argc <= 1) {
    print_help();
    return 0;
  }
  // Ctrl+C handling
  signal(SIGINT, finish_application);
  try {
    auto config = parse_cmdline_options(argc, argv);
    auto bpf_program = BpfProgram::create();
    auto xdp_program = XdpProgram::from_bpf_obj(
      bpf_program.get()->obj,
      static_cast<int>(config.interface_index));
    xdp_program.attach();
    configure_kernel_program(xdp_program, config);
    run_polling(xdp_program);
  } catch(std::exception& exception) {
    print_err(exception.what());
  } catch(...) {
    print_err("Unexpected error");
  }
  return 0;
}

void
finish_application(int)
{
  IS_RUNNING = 0;
}

StatData
get_stats(XdpProgram::BpfFileDescriptor fd)
{
  StatData result {};
  auto error = bpf_map_lookup_elem(fd.descriptor, &KEY, &result);
	if (error != 0) {
    throw std::runtime_error("Can't get statistics from the kernel");
	}
  return result;
}

void
configure_kernel_program(XdpProgram& program, const CmdLineOptions& options)
{
  uint8_t filter_flags = (
    flag_from_optional(options.src_address, FILTER_SRC_ADDRESS) |
    flag_from_optional(options.dst_address, FILTER_DST_ADDRESS) |
    flag_from_optional(options.src_port, FILTER_SRC_PORT) |
    flag_from_optional(options.dst_port, FILTER_DST_PORT) |
    flag_from_boolean(options.collect_tcp, FILTER_TCP) |
    flag_from_boolean(options.collect_udp, FILTER_UDP)
  );
  ConfigData config {
    .src_address = options.src_address.value_or(DEFAULT_ADDRESS),
    .dst_address = options.dst_address.value_or(DEFAULT_ADDRESS),
    .src_port = options.src_port.value_or(DEFAULT_PORT),
    .dst_port = options.dst_port.value_or(DEFAULT_PORT),
    .filter_flags = filter_flags,
  };
  program.update_map("xdp_config_map", &config);
}

void
run_polling(XdpProgram& program)
{
  auto map_fd = program.find_map_fd("xdp_stats_map");
  while (IS_RUNNING) {
    auto value = get_stats(map_fd);
    print(std::format("Packets: {}", value.received_packets));
    print(std::format("Bytes: {}", value.received_bytes));
    std::this_thread::sleep_for(SLEEP_INTERVAL);
  }
}
