#include <algorithm>
#include <arpa/inet.h>
#include <cstdint>
#include <format>
#include <fstream>
#include <net/if.h>
#include <optional>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <unordered_map>
#include <unordered_set>

#include "cmdline.hxx"
#include "utils.hxx"

RawOptionMappting
populate_options_map(int argc, char **argv)
{
  std::unordered_map<std::string, std::string> result;
  // Using 1 helps skip the name of the application
  for (int i = 1; i < argc; i++) {
    std::string option {argv[i]}; // NOLINT
    auto parts = split(option, "=");
    if (parts.size() < 2) {
      throw std::runtime_error(std::format("Wrong option: ", option));
    }
    result.insert({ parts.at(0), parts.at(1) });
  }
  return result;
}

template<typename T>
T
parse_number(const std::string& option)
{
  T result = 0;
  std::istringstream input_stream {option};
  input_stream.exceptions(std::ifstream::failbit);
  try {
    input_stream >> result;
  } catch(const std::ios_base::failure& exception) {
    throw std::runtime_error(std::format("Wrong option value: {}", option));
  }
  return htons(result);
}

std::optional<uint32_t>
get_address(const RawOptionMappting& options, const std::string& option_name)
{
  if (!options.contains(option_name)) {
    return std::nullopt;
  }
  auto option = options.at(option_name);
  if (option.empty()) {
    throw std::runtime_error("IP address was not specified");
  }
  if (CMDLINE_ANY_PLACEHOLDERS.contains(option)) {
    return std::nullopt;
  }
  uint32_t result = 0;
  auto error = inet_pton(AF_INET, option.c_str(), &result);
  if (error <= 0) {
    throw std::runtime_error(std::format("Wrong address: {}", option));
  }
  return {result};
}

std::optional<uint16_t>
get_port(const RawOptionMappting& options, const std::string& option_name)
{
  if (!options.contains(option_name)) {
    return std::nullopt;
  }
  auto option = options.at(option_name);
  if (option.empty()) {
    throw std::runtime_error("Port was not specified");
  }
  if (CMDLINE_ANY_PLACEHOLDERS.contains(option)) {
    return std::nullopt;
  }
  return std::make_optional(parse_number<uint16_t>(option));
};

bool
get_protocol(const RawOptionMappting& options,
             const std::string& option_name,
             const std::string& expected_protocol)
{
  // By default we are collecting all protocols
  if (!options.contains(option_name)) {
    return true;
  }
  auto option = options.at(option_name);
  if (option.empty()) {
    throw std::runtime_error("Protocol was not specified");
  }
  if (CMDLINE_ANY_PLACEHOLDERS.contains(option)) {
    return true;
  }
  return options.at(option_name) == expected_protocol;
}

void
check_option_names(const RawOptionMappting& options)
{
  auto keys = options | std::views::keys;
  std::ranges::for_each(keys, [](const auto& option) {
    if (!KNOWN_OPTIONS.contains(option)) {
      throw std::runtime_error(std::format("Unknown option name: {}", option));
    }
  });
}

void check_required_options(const RawOptionMappting& options) {
  for (auto& required_option : REQUIRED_OPTIONS) {
    if (!options.contains(required_option)) {
      throw std::runtime_error(std::format("Option \"{}\" is required", required_option));
    }
  }
}

unsigned int
get_interface_index(const RawOptionMappting& options,
                    const std::string& option_name)
{
  if (!options.contains(option_name)) {
    throw std::runtime_error(std::format("Can't find an interface option: {}", option_name));
  }
  auto interface_name = options.at(option_name);
  auto result = if_nametoindex(interface_name.c_str());
  if (result == 0) {
    throw std::runtime_error(std::format("Can't find an interface by name: {}", interface_name));
  }
  return result;
}

CmdLineOptions
parse_cmdline_options(int argc, char **argv)
{
  auto options = populate_options_map(argc, argv);
  check_option_names(options);
  check_required_options(options);
  return {
    .src_address = get_address(options, OPTION_SRC_ADDRESS),
    .dst_address = get_address(options, OPTION_DST_ADDRESS),
    .src_port = get_port(options, OPTION_SRC_PORT),
    .dst_port = get_port(options, OPTION_DST_PORT),
    .collect_tcp = get_protocol(options, OPTION_PROTOCOL, PROTOCOL_TCP),
    .collect_udp = get_protocol(options, OPTION_PROTOCOL, PROTOCOL_UDP),
    .interface_index = get_interface_index(options, OPTION_INTERFACE),
  };
}
