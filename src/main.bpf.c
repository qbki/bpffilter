#include <linux/bpf.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <bpf/bpf_helpers.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "common.h"

// BPF_MAP_TYPE_ARRAY is used for simplicity.
// A better solution is BPF_MAP_TYPE_PERCPU_HASH.
struct {
  __uint(type, BPF_MAP_TYPE_ARRAY);
  __type(key, __u32);
  __type(value, struct StatData);
  __uint(max_entries, 1);
} xdp_stats_map SEC(".maps"); // NOLINT
                              //
struct {
  __uint(type, BPF_MAP_TYPE_ARRAY);
  __type(key, __u32);
  __type(value, struct ConfigData);
  __uint(max_entries, 1);
} xdp_config_map SEC(".maps"); // NOLINT

static __u64 START_TIME = 0; // NOLINT

SEC("xdp")
int xdp_prog(struct xdp_md* ctx)
{
  void *data_end = (void *)(long)ctx->data_end;
  void *data = (void *)(long)ctx->data;

  struct ethhdr *eth_header = data;
  if (eth_header + 1 > data_end) { // NOLINT
    return XDP_ABORTED;
  }

  struct ConfigData *configs = bpf_map_lookup_elem(&xdp_config_map, &KEY);
  if (!configs) {
    return XDP_PASS;
  }
  __u8 filter_flags = 0;
  __u8 filter_flags_cmp = configs->filter_flags;

  int is_src_address_collecting = configs->filter_flags & FILTER_SRC_ADDRESS;
  int is_dst_address_collecting = configs->filter_flags & FILTER_DST_ADDRESS;
  int is_src_port_collecting = configs->filter_flags & FILTER_SRC_PORT;
  int is_dst_port_collecting = configs->filter_flags & FILTER_DST_PORT;
  int is_tcp_collecting = configs->filter_flags & FILTER_TCP;
  int is_udp_collecting = configs->filter_flags & FILTER_UDP;

  if (eth_header->h_proto == htons(ETH_P_IP)) {
    struct iphdr *ip_header = data + sizeof(struct ethhdr);
    if (ip_header + 1 > (struct iphdr *)data_end) {
        return XDP_ABORTED;
    }
    __u32 saddress = (__u32) ip_header->saddr;
    __u32 daddress = (__u32) ip_header->daddr;
    if (is_src_address_collecting && (configs->src_address == saddress)) {
      filter_flags |= FILTER_SRC_ADDRESS;
    }
    if (is_dst_address_collecting && (configs->dst_address == daddress)) {
      filter_flags |= FILTER_DST_ADDRESS;
    }
    if(is_tcp_collecting && (ip_header->protocol == IPPROTO_TCP)) {
      struct tcphdr *tcp_header = data + sizeof(struct ethhdr) + sizeof(struct iphdr);
      filter_flags |= FILTER_TCP;
      // I have to remove the other port from the final comparison
      // because both ports cannot occure in one request.
      filter_flags_cmp &= ~FILTER_UDP;
      if (tcp_header + 1 > (struct tcphdr *)data_end) {
        return XDP_ABORTED;
      }
      __u16 sport = (__u16) tcp_header->source;
      __u16 dport = (__u16) tcp_header->dest;
      if (is_src_port_collecting && (configs->src_port == sport)) {
        filter_flags |= FILTER_SRC_PORT;
      }
      if (is_dst_port_collecting && (configs->dst_port == dport)) {
        filter_flags |= FILTER_DST_PORT;
      }
    } else if (is_udp_collecting && (ip_header->protocol == IPPROTO_UDP)) {
      struct udphdr *udp_header = (data + sizeof(struct ethhdr) + sizeof(struct iphdr));
      filter_flags |= FILTER_UDP;
      filter_flags_cmp &= ~FILTER_TCP;
      if (udp_header + 1 > (struct udphdr *)data_end) {
        return XDP_ABORTED;
      }
      __u16 sport = (__u16) udp_header->source;
      __u16 dport = (__u16) udp_header->dest;
      if (is_src_port_collecting && (configs->src_port == sport)) {
        filter_flags |= FILTER_SRC_PORT;
      }
      if (is_dst_port_collecting && (configs->dst_port == dport)) {
        filter_flags |= FILTER_DST_PORT;
      }
    }
  }

  struct StatData *result = bpf_map_lookup_elem(&xdp_stats_map, &KEY);
  if (!result) {
    return XDP_PASS;
  }

  if (filter_flags == filter_flags_cmp) {
    __u64 length = ctx->data_end - ctx->data;
    // I have to use synchronization because of BPF_MAP_TYPE_ARRAY.
    // It is slow but simple for this example.
    __sync_fetch_and_add(&result->received_packets, 1);
    __sync_fetch_and_add(&result->received_bytes, length);
  }
  return XDP_PASS;
}

const char LICENSE[] SEC("license") = "Dual MIT/GPL";
