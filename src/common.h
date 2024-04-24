#pragma once

// Types of this structure must be taken from "vmlinux.h". But I can't include
// "vmlinux.h" into C++ application because of absence of some types. So, I
// just copied real type from its definition. For instance:
// typedef long long unsigned int __u64;
//         ^real type             ^goes to comment
struct InterfaceData {
	long long unsigned int received_packets; // __u64
	long long unsigned int received_bytes; // __u64
};

const unsigned int KEY = 0; // __u32
