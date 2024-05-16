#include <bpf/libbpf.h>
#include <cstddef>
#include <format>
#include <ranges>
#include <stdexcept>

#include "utils.hxx"

const static unsigned long GB = 1024 * 1024 * 1024;
const static unsigned long MB = 1024 * 1024;
const static unsigned long KB = 1024;

void
flush() {
  std::cout << std::flush;
}

std::vector<std::string>
split(const std::string& value, const std::string& delimiter)
{
  auto view = value | std::views::split(delimiter)
                    | std::views::transform([](const auto& v) {
                        return std::string(v.begin(), v.end());
                      });
  return {view.begin(), view.end()};
}

std::string format_mbitps(unsigned long bytes, unsigned long seconds) {
  const double value = (static_cast<double>(bytes) / MB) * 8
                       / static_cast<double>(seconds);
  return std::format("{:.2f}Mibit/s", value);
}

std::string format_mbyteps(unsigned long bytes, unsigned long seconds) {
  const double value = (static_cast<double>(bytes) / MB)
                       / static_cast<double>(seconds);
  return std::format("{:.2f}MiB/s", value);
}

std::string format_bytes(unsigned long size) {
  if (size >= GB) {
    return std::format("{:.2f}GiB", static_cast<double>(size) / GB);
  } else if (size >= MB) {
    return std::format("{:.2f}MiB", static_cast<double>(size) / MB);
  } else if (size >= KB) {
    return std::format("{:.2f}KiB", static_cast<double>(size) / KB);
  }
  return std::format("{}B", size);
}

std::string format_ipv4_address(uint32_t value) {
  const uint32_t byte_mask = 0b11111111;
  const int octet_offset_1 = 8;
  const int octet_offset_2 = 16;
  const int octet_offset_3 = 24;
  return std::format("{}.{}.{}.{}",
                     value & byte_mask,
                     (value >> octet_offset_1) & byte_mask,
                     (value >> octet_offset_2) & byte_mask,
                     (value >> octet_offset_3) & byte_mask);
};

std::vector<StatData> generate_stats_per_cpu_array() {
  StatData data {
    .received_packets = 0,
    .received_bytes = 0,
  };
  auto cpus_quantity = libbpf_num_possible_cpus();
  if (cpus_quantity <= 0) {
    throw std::runtime_error("Imposible CPUs quantity");
  }
  return {static_cast<size_t>(cpus_quantity), data};
}
