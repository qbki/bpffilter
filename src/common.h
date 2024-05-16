#pragma once
#include <linux/bpf.h>

const __u32 KEY = 0;

const unsigned char FILTER_SRC_ADDRESS = 1u << 0;
const unsigned char FILTER_DST_ADDRESS = 1u << 1;
const unsigned char FILTER_SRC_PORT    = 1u << 2;
const unsigned char FILTER_DST_PORT    = 1u << 3;
const unsigned char FILTER_TCP         = 1u << 4;
const unsigned char FILTER_UDP         = 1u << 5;

struct StatData {
	__u64 received_packets;
	__u64 received_bytes;
};

struct ConfigData {
  __u32 src_address;
  __u32 dst_address;
  __u16 src_port;
  __u16 dst_port;
  __u8 filter_flags;
};

struct BpfFileDescriptor {
  int descriptor;
};
