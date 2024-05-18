#include <arpa/inet.h>
#include <linux/bpf.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <netinet/in.h>

// Keep this header separated because of code autoformatting issues
#include <bpf/bpf_helpers.h>

#include "common.h"
#include "utils.bpf.h"

struct
{
  __uint(type, BPF_MAP_TYPE_PERCPU_ARRAY);
  __type(key, __u32);
  __type(value, struct StatData);
  __uint(max_entries, 1);
} xdp_stats_map SEC(".maps"); // NOLINT

struct
{
  __uint(type, BPF_MAP_TYPE_ARRAY);
  __type(key, __u32);
  __type(value, struct ConfigData);
  __uint(max_entries, 1);
} xdp_config_map SEC(".maps"); // NOLINT

static __u64 START_TIME = 0; // NOLINT

SEC("xdp")
int
xdp_prog(struct xdp_md* ctx)
{
  void* data_end = (void*)(long)ctx->data_end;
  void* data = (void*)(long)ctx->data;

  struct ethhdr* eth_header = data;
  if (eth_header + 1 > data_end) { // NOLINT
    return XDP_PASS;
  }

  struct ConfigData* configs = bpf_map_lookup_elem(&xdp_config_map, &KEY);
  if (!configs) {
    return XDP_PASS;
  }
  __u8 current_filter_flags = 0;
  __u8 expected_filter_flags = configs->filter_flags;

  if (eth_header->h_proto == htons(ETH_P_IP)) {
    struct iphdr* ip_header = data + sizeof(struct ethhdr);
    void* proto = data + sizeof(struct ethhdr) + sizeof(struct iphdr);

    if (ip_header + 1 > (struct iphdr*)data_end) { // NOLINT
      return XDP_PASS;
    }
    __u32 saddress = (__u32)ip_header->saddr;
    __u32 daddress = (__u32)ip_header->daddr;
    current_filter_flags |= collect_addresses(configs, saddress, daddress);
    current_filter_flags |=
      collect_proto(configs, ip_header, &expected_filter_flags);
    if (ip_header->protocol == IPPROTO_TCP) {
      struct tcphdr* tcp_header = proto;
      if (tcp_header + 1 > (struct tcphdr*)data_end) { // NOLINT
        return XDP_PASS;
      }
      __u16 sport = (__u16)tcp_header->source;
      __u16 dport = (__u16)tcp_header->dest;
      current_filter_flags |= collect_ports(configs, sport, dport);
    } else if (ip_header->protocol == IPPROTO_UDP) {
      struct udphdr* udp_header = proto;
      if (udp_header + 1 > (struct udphdr*)data_end) { // NOLINT
        return XDP_PASS;
      }
      __u16 sport = (__u16)udp_header->source;
      __u16 dport = (__u16)udp_header->dest;
      current_filter_flags |= collect_ports(configs, sport, dport);
    }
  }

  if (current_filter_flags == expected_filter_flags) {
    struct StatData* result = bpf_map_lookup_elem(&xdp_stats_map, &KEY);
    if (!result) {
      return XDP_PASS;
    }
    result->received_packets += 1;
    result->received_bytes += ctx->data_end - ctx->data;
  }
  return XDP_PASS;
}

const char LICENSE[] SEC("license") = "Dual MIT/GPL";
