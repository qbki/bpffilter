#include <format>
#include <ranges>

#include "utils.hxx"

const static unsigned long GB = 1024 * 1024 * 1024;
const static unsigned long MB = 1024 * 1024;
const static unsigned long KB = 1024;

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
  return std::format("{}.{}.{}.{}",
                     value & byte_mask,
                     (value >> 8) & byte_mask,
                     (value >> 16) & byte_mask,
                     (value >> 24) & byte_mask);
};
