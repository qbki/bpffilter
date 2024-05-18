#include <bpf/bpf.h>
#include <bpf/libbpf.h>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <exception>
#include <fcntl.h>
#include <format>
#include <netinet/in.h>
#include <optional>
#include <stdexcept>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <xdp/libxdp.h>

#include "bpf_program.hxx"
#include "cmdline.hxx"
#include "kernel/common.h"
#include "main.skel.h"
#include "sampler.hxx"
#include "screen.hxx"
#include "utils.hxx"
#include "xdp_program.hxx"

// DEFAULT_PORT and DEFAULT_ADDRESS should not be
// used in logic, it is just a placeholder.
const uint16_t DEFAULT_PORT = 0;
const uint32_t DEFAULT_ADDRESS = 0;

const std::chrono::seconds SLEEP_INTERVAL{ 1 };
static volatile sig_atomic_t IS_RUNNING = 1; // NOLINT

const std::string XDP_CONFIG_MAP = "xdp_config_map";
const std::string XDP_STATS_MAP = "xdp_stats_map";

/**
 * Returns statistics from the kernel program.
 * @param fd A BPF file descriptor.
 * @return Quantity of bytes and packages since start of application.
 */
StatData
get_stats(BpfFileDescriptor fd);

/**
 * Performs initialization of BPF maps which are used
 * by kernel and user programs.
 * @param program An XDP program.
 * @param options Command line options.
 */
void
configure_kernel_program(XdpProgram& program, const CmdLineOptions& options);

/**
 * A callback for handling of the "Ctrl+C" keyboard shortcut (SIGINT).
 * @param _ Unused.
 */
void
finish_application(int);

/**
 * Performs an infinite loop with printing statistics once per second.
 * @param program An XDP program.
 * @param options Command line options.
 */
void
run_polling(XdpProgram& program, const CmdLineOptions& options);

int
main(int argc, char** argv)
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
      bpf_program.get()->obj, static_cast<int>(config.interface_index));
    xdp_program.attach();
    configure_kernel_program(xdp_program, config);
    run_polling(xdp_program, config);
  } catch (std::exception& exception) {
    print_err(exception.what());
  } catch (...) {
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
get_stats(BpfFileDescriptor fd)
{
  auto stats_per_cpu = generate_stats_per_cpu_array();
  auto error = bpf_map_lookup_elem(fd.descriptor, &KEY, stats_per_cpu.data());
  if (error != 0) {
    throw std::runtime_error("Can't get statistics from the kernel");
  }
  StatData result{
    .received_packets = 0,
    .received_bytes = 0,
  };
  for (auto& stat : stats_per_cpu) {
    result.received_packets += stat.received_packets;
    result.received_bytes += stat.received_bytes;
  }
  return result;
}

void
configure_kernel_program(XdpProgram& program, const CmdLineOptions& options)
{
  uint8_t filter_flags =
    (flag_from_optional(options.src_address, FILTER_SRC_ADDRESS) |
     flag_from_optional(options.dst_address, FILTER_DST_ADDRESS) |
     flag_from_optional(options.src_port, FILTER_SRC_PORT) |
     flag_from_optional(options.dst_port, FILTER_DST_PORT) |
     flag_from_boolean(options.collect_tcp, FILTER_TCP) |
     flag_from_boolean(options.collect_udp, FILTER_UDP));
  ConfigData config{
    .src_address = options.src_address.value_or(DEFAULT_ADDRESS),
    .dst_address = options.dst_address.value_or(DEFAULT_ADDRESS),
    .src_port = options.src_port.value_or(DEFAULT_PORT),
    .dst_port = options.dst_port.value_or(DEFAULT_PORT),
    .filter_flags = filter_flags,
  };
  program.update_map(XDP_CONFIG_MAP, &config);
  auto empty_stats_array = generate_stats_per_cpu_array();
  program.update_map(XDP_STATS_MAP, empty_stats_array.data());
}

void
run_polling(XdpProgram& program, const CmdLineOptions& options)
{
  auto map_fd = program.find_map_fd(XDP_STATS_MAP);
  auto start_time = std::chrono::system_clock::now();
  Sampler<uint64_t> bytes_sampler{};
  auto format_speed = options.speed_format == SpeedFormatEnum::BIT
                        ? format_mbitps
                        : format_mbyteps;
  auto format_address = [](const std::optional<uint32_t>& value) {
    return value.has_value() ? format_ipv4_address(value.value()) : "any";
  };
  auto format_port = [](const std::optional<uint16_t>& value) {
    return value.has_value() ? std::to_string(ntohs(value.value())) : "any";
  };
  auto format_protocol = [](bool value) { return value ? "yes" : "no"; };

  Screen screen;
  while (IS_RUNNING) {
    // Keep the sleep_for function up. It helps gather statistics before first
    // appearance. In the other case you will get default values and wrong
    // minimal speed.
    std::this_thread::sleep_for(SLEEP_INTERVAL);
    auto stats_data = get_stats(map_fd);
    bytes_sampler.sample(stats_data.received_bytes);
    auto elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(
                             std::chrono::system_clock::now() - start_time)
                             .count();
    screen.clear().go_home();
    print(std::format("Source: {}:{}",
                      format_address(options.src_address),
                      format_port(options.src_port)));
    print(std::format("Destination: {}:{}",
                      format_address(options.dst_address),
                      format_port(options.dst_port)));
    print(std::format("Protocols: TCP:{} UDP:{}",
                      format_protocol(options.collect_tcp),
                      format_protocol(options.collect_udp)));
    print(std::format("Packets: {}", stats_data.received_packets));
    print(std::format("Bytes: {}", format_bytes(stats_data.received_bytes)));
    print(
      std::format("Average speed since start of the application: {}",
                  format_speed(stats_data.received_bytes, elapsed_seconds)));
    print(std::format("Time: {}sec", elapsed_seconds));
    print(std::format("Current speed: {}",
                      format_speed(bytes_sampler.distance(), 1)));
    print(std::format("Min speed: {}", format_speed(bytes_sampler.min(), 1)));
    print(std::format("Max speed (peak): {}",
                      format_speed(bytes_sampler.max(), 1)));
    flush();
  }
}
