#pragma once
#include <linux/ip.h>
#include <netinet/in.h>

#include "common.h"

inline __u8
remove_bits(__u8 value, __u8 bits_for_removing)
{
  return value & ~bits_for_removing;
}

inline __u8
collect_addresses(struct ConfigData* configs, __u32 saddress, __u32 daddress)
{
  __u8 filter_flags = 0;
  int is_src_address_collecting = configs->filter_flags & FILTER_SRC_ADDRESS;
  int is_dst_address_collecting = configs->filter_flags & FILTER_DST_ADDRESS;
  if (is_src_address_collecting && (configs->src_address == saddress)) {
    filter_flags |= FILTER_SRC_ADDRESS;
  }
  if (is_dst_address_collecting && (configs->dst_address == daddress)) {
    filter_flags |= FILTER_DST_ADDRESS;
  }
  return filter_flags;
}

inline __u8
collect_ports(struct ConfigData* configs, __u16 sport, __u16 dport)
{
  __u8 filter_flags = 0;
  int is_src_port_collecting = configs->filter_flags & FILTER_SRC_PORT;
  int is_dst_port_collecting = configs->filter_flags & FILTER_DST_PORT;
  if (is_src_port_collecting && (configs->src_port == sport)) {
    filter_flags |= FILTER_SRC_PORT;
  }
  if (is_dst_port_collecting && (configs->dst_port == dport)) {
    filter_flags |= FILTER_DST_PORT;
  }
  return filter_flags;
}

inline __u8
collect_proto(struct ConfigData* configs,
              struct iphdr* ip_header,
              __u8* expected_filter_flags)
{
  __u8 filter_flags = 0;
  int is_tcp_collecting = configs->filter_flags & FILTER_TCP;
  int is_udp_collecting = configs->filter_flags & FILTER_UDP;
  if (is_tcp_collecting && (ip_header->protocol == IPPROTO_TCP)) {
    filter_flags |= FILTER_TCP;
    // I have to remove the other port from the final comparison when TCP and
    // UDP ports are collecting, because both ports cannot occur in one
    // request.
    *expected_filter_flags = remove_bits(*expected_filter_flags, FILTER_UDP);
  } else if (is_udp_collecting && (ip_header->protocol == IPPROTO_UDP)) {
    filter_flags |= FILTER_UDP;
    // See comment about TCP port.
    *expected_filter_flags = remove_bits(*expected_filter_flags, FILTER_TCP);
  }
  return filter_flags;
}
