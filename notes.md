List all XDP programs in kernel.
```bash
sudo bpftool net list dev lo
```

Unload all XDP programs from kernel.
```bash
sudo ip lint net list dev lo
```

Install linux headers
```bash
sudo apt-get install linux-headers-$(uname -r)
```

For using stdint.h
```bash
sudo apt-get install gcc-multilib
```

Netcat server:
```bash
nc -lv 9000
```

Netcat client:
```bash
nc -v localhost 1234
```
