#include <linux/bpf.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <bpf/bpf_helpers.h>
#include <arpa/inet.h>

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

SEC("xdp")
int xdp_prog(struct xdp_md* ctx)
{
  __u64 length = ctx->data_end - ctx->data;
  void *data_end = (void *)(long)ctx->data_end;
  void *data = (void *)(long)ctx->data;
  int is_collected = 0;

  struct ethhdr *eth_header = data;
  if (eth_header + 1 > data_end) { // NOLINT
    return XDP_PASS;
  }

  if (eth_header->h_proto == htons(ETH_P_IP)) {
    struct iphdr *ip_header = (struct iphdr *) ((uint8_t *) eth_header + ETH_HLEN);
    if (ip_header + 1 > (struct iphdr *)data_end)
    {
        return XDP_PASS;
    }
    if(ip_header->protocol == IPPROTO_TCP) {
      struct tcphdr *tcp_header = data + sizeof(struct ethhdr) + (ip_header->ihl * 4);
      if (tcp_header + 1 > (struct tcphdr *)data_end)
      {
          return XDP_DROP;
      }
      unsigned int sport = htons((unsigned short int) tcp_header->source);
      unsigned int dport = htons((unsigned short int) tcp_header->dest);
      struct ConfigData *result = bpf_map_lookup_elem(&xdp_config_map, &KEY);
      if (!result) {
        return XDP_ABORTED;
      }
      if (dport == result->dst_port) {
        is_collected = 1;
      }
    }
  }

  struct StatData *result = bpf_map_lookup_elem(&xdp_stats_map, &KEY);
  if (!result) {
    return XDP_ABORTED;
  }

  if (is_collected) {
    // I have to use synchronization because of BPF_MAP_TYPE_ARRAY. It is slow
    // but simple for this example.
    __sync_fetch_and_add(&result->received_packets, 1);
    __sync_fetch_and_add(&result->received_bytes, length);
  }
  return XDP_PASS;
}

const char LICENSE[] SEC("license") = "Dual MIT/GPL";
