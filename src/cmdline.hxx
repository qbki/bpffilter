#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>

const std::string HELP_TEXT {
  "bpffilter if=<interface name> [sa=*] [da=*] [sp=*] [dp=*] [pr=udp|tcp|*]\n"
  "  if - interface where eBPF program will be installed\n"
  "  sa - a source IPv4 address or *\n"
  "  da - a destination IPv4 address or *\n"
  "  sp - a source port 0-65535 or *\n"
  "  dp - a destination port 0-65535 or *\n"
  "  pr - a protocol, possible values: udp, tcp or *\n"
  "Example:\n"
  "  $ sudo ./bpffilter if=lo sp=9002 dp=9000 pr=udp\n"
  "  It will watch network packages on a localhost interface.\n"
};

const std::string OPTION_SRC_ADDRESS {"sa"};
const std::string OPTION_DST_ADDRESS {"da"};
const std::string OPTION_SRC_PORT {"sp"};
const std::string OPTION_DST_PORT {"dp"};
const std::string OPTION_INTERFACE {"if"};
const std::string OPTION_PROTOCOL {"pr"};

const std::string PROTOCOL_UDP {"udp"};
const std::string PROTOCOL_TCP {"tcp"};

const std::unordered_set<std::string> CMDLINE_ANY_PLACEHOLDERS {
  "any",
  "*",
};

const std::unordered_set<std::string> KNOWN_OPTIONS {
  OPTION_SRC_ADDRESS,
  OPTION_DST_ADDRESS,
  OPTION_SRC_PORT,
  OPTION_DST_PORT,
  OPTION_INTERFACE,
  OPTION_PROTOCOL,
};

const std::unordered_set<std::string> REQUIRED_OPTIONS {
  OPTION_INTERFACE,
};

using RawOptionMappting = std::unordered_map<std::string, std::string>;

struct CmdLineOptions {
  std::optional<uint32_t> src_address {0};
  std::optional<uint32_t> dst_address {0};
  std::optional<uint16_t> src_port {0};
  std::optional<uint16_t> dst_port {0};
  bool collect_tcp {true};
  bool collect_udp {true};
  unsigned int interface_index {0};
};

CmdLineOptions parse_cmdline_options(int argc, char **argv);
