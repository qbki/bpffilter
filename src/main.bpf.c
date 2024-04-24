#include "vmlinux.h"
#include <bpf/bpf_helpers.h>

#include "common.h"

// BPF_MAP_TYPE_ARRAY is used for simplicity.
// A better solution is BPF_MAP_TYPE_PERCPU_HASH.
struct {
  __uint(type, BPF_MAP_TYPE_ARRAY);
  __type(key, __u32);
  __type(value, struct InterfaceData);
  __uint(max_entries, 1);
} xdp_stats_map SEC(".maps"); // NOLINT

SEC("xdp")
int xdp_prog(struct xdp_md* ctx)
{
  struct InterfaceData *result = bpf_map_lookup_elem(&xdp_stats_map, &KEY);
  if (!result) {
    return XDP_ABORTED;
  }

  // Package counter
  // I have to use synchronization because of BPF_MAP_TYPE_ARRAY. It is slow
  // but simple for this example.
  __sync_fetch_and_add(&result->received_packets, 1);
  return XDP_PASS;
}

const char LICENSE[] SEC("license") = "Dual MIT/GPL";
