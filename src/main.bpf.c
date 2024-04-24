#include "vmlinux.h"
#include <bpf/bpf_helpers.h>

SEC("xdp")
int xdp_prog(struct xdp_md* ctx)
{
  bpf_printk("I am here!\n");
  return XDP_PASS;
}

const char LICENSE[] SEC("license") = "Dual MIT/GPL";
