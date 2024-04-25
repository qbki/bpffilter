#pragma once
#include <optional>

struct CmdLineOptions {
  std::optional<unsigned short> src_port {};
  std::optional<unsigned short> dst_port {};
  unsigned int interface_index {0};
};

CmdLineOptions parse_cmdline_options(int argc, char **argv);
