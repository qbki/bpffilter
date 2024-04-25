#pragma once
#include <cstdint>
#include <optional>

struct CmdLineOptions {
  std::optional<uint32_t> src_address {};
  std::optional<uint32_t> dst_address {};
  std::optional<uint16_t> src_port {};
  std::optional<uint16_t> dst_port {};
  unsigned int interface_index {0};
};

CmdLineOptions parse_cmdline_options(int argc, char **argv);
