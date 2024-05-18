# Network monitor utility

## Installation

The utility is destributed as a portable application. Just download it from
the release page and use it.

## Documentation

Run the utility without arguments for printing a help text:

```sh
$ ./bpffilter
  bpffilter if=<interface name> [sa=*] [da=*] [sp=*] [dp=*] [pr=udp|tcp|*]

    if - interface where an eBPF program will be installed
    sf - a speed format, bit or byte
    sa - a source IPv4 address or *
    da - a destination IPv4 address or *
    sp - a source port 0-65535 or *
    dp - a destination port 0-65535 or *
    pr - a protocol, possible values: udp, tcp or *
```

Possible output:
```sh
$ sudo ./bpffilter if=enp0s31f6 sf=byte
Source: any:any
Destination: any:any
Protocols: TCP:yes UDP:yes
Packets: 184750
Bytes: 263.39MiB
Average speed since start of the application: 17.56MiB/s
Time: 15sec
Current speed: 16.95MiB/s
Min speed: 15.37MiB/s
Max speed (peak): 17.98MiB/s
```

Output will be updated once per second.

The utility uses binary prefixes for unit formatting from ISO/IEC 80000
standard. So, one kibibit (Kibit) contains 1024 bits and one kibibyte (KiB)
contains 1024 bytes.

```sh
$ sudo ./bpffilter if=enp0s31f6 sf=byte
Max speed (peak): 1.69MiB/s
$ sudo ./bpffilter if=enp0s31f6 sf=bit
Max speed (peak): 19.50Mibit/s
```

## Build

### Compilation (Ubuntu 23.10)

Install dependencies:

```sh
$ sudo apt-get install \
    cmake \
    git \
    make \
    sudo \
    clang-17 \
    gcc-multilib \
    libbpf-dev \
    libxdp-dev \
    linux-tools-common \
    linux-tools-generic \
    linux-headers-$(uname -r)
$ mkdir build && cd build
$ cmake .. && make
```

If a compiler can't find `asm/types.h` header, you could try:
```sh
$ sudo ln -s /usr/include/x86_64-linux-gnu/asm /usr/include/asm
```
An executable will be placed at `./build-docker/bin/bpffilter`.

### Docker

The sequence of actions to build the utility with help of Docker:

1. Create `build-docker` directory;
2. Copy `.env.default` as `.env`;
3. Edit `.env` as explaned in this file;
4. Run `docker compose up`.

```sh
& mkdir build-docker
& cp .env.default .env
& editor .env
& docker compose up
```

An executable will be placed at `./build-docker/bin/bpffilter`.

## License

This code is [MIT licensed](./LICENSE).

Also, see [main.bpf.c](./src/main.bpf.c). It must be GPL compatible.
