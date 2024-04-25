#pragma once

const unsigned int KEY = 0; // __u32
const unsigned char FILTER_SRC_ADDRESS = 1u << 0;
const unsigned char FILTER_DST_ADDRESS = 1u << 1;
const unsigned char FILTER_SRC_PORT    = 1u << 2;
const unsigned char FILTER_DST_PORT    = 1u << 3;

// Types of this structure must be taken from "vmlinux.h". But I can't include
// "vmlinux.h" into C++ application because of absence of some types. So, I
// just copied real type from its definition. For instance:
// typedef long long unsigned int __u64;
//         ^real type             ^goes to comment
struct StatData {
	long long unsigned int received_packets; // __u64
	long long unsigned int received_bytes; // __u64
};

struct ConfigData {
  unsigned int src_address; // __u32
  unsigned int dst_address; // __u32
  unsigned short src_port; // __u16
  unsigned short dst_port; // __u16
  unsigned char filter_flags; // __u8
};
