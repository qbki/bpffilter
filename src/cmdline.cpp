#include <algorithm>
#include <format>
#include <fstream>
#include <net/if.h>
#include <optional>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "./cmdline.hxx"

const std::string OPTION_SRC_PORT {"sp"};
const std::string OPTION_DST_PORT {"dp"};
const std::string OPTION_INTERFACE {"if"};

const std::unordered_set<std::string> KNOWN_OPTIONS {
  OPTION_SRC_PORT,
  OPTION_DST_PORT,
  OPTION_INTERFACE,
};

const std::unordered_set<std::string> REQUIRED_OPTIONS {
  OPTION_INTERFACE,
};

using RawOptionMappting = std::unordered_map<std::string, std::string>;
using PortType = unsigned short;

std::pair<std::string, std::string> string_to_pair(const std::string& value) {
  auto position = value.find("=");
  if (position == std::string::npos) {
    return { value, {} };
  }
  return { value.substr(0, position),
           value.substr(position + 1, value.size()) };
}

RawOptionMappting populate_options_map(int argc, char **argv) {
  std::unordered_map<std::string, std::string> result;
  // Using 1 helps skip the name of application
  for (int i = 1; i < argc; i++) {
    std::string option {argv[i]}; // NOLINT
    result.insert(string_to_pair(option));
  }
  return result;
}

template<typename T>
T parse_number(const std::string& option) {
  T result = 0;
  std::istringstream input_stream {option};
  input_stream.exceptions(std::ifstream::failbit);
  try {
    input_stream >> result;
  } catch(const std::ios_base::failure& exception) {
    throw std::runtime_error(std::format("Wrong option value: {}", option));
  }
  return result;
}

template<typename T>
std::optional<T> get_number_value(const RawOptionMappting& options,
                                  const std::string& option_name)
{
  return options.contains(option_name)
    ? std::make_optional(parse_number<T>(options.at(option_name)))
    : std::nullopt;
};

void check_option_names(const RawOptionMappting& options) {
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

unsigned int get_interface_index(const RawOptionMappting& options,
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

CmdLineOptions parse_cmdline_options(int argc, char **argv) {
  auto options = populate_options_map(argc, argv);
  check_option_names(options);
  check_required_options(options);
  return {
    .src_port = get_number_value<PortType>(options, OPTION_SRC_PORT),
    .dst_port = get_number_value<PortType>(options, OPTION_DST_PORT),
    .interface_index = get_interface_index(options, OPTION_INTERFACE),
  };
}
