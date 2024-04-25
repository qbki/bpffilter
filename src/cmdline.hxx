#pragma once
#include <optional>

struct CmdLineOptions {
  std::optional<unsigned short>  dst_port {};
};

CmdLineOptions parse_cmdline_options(int argc, char **argv);
