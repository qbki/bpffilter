FROM ubuntu:23.10

RUN apt-get update \
  && apt-get install -y \
    cmake \
    make \
    sudo \
    clang-17 \
    gcc-multilib \
    libbpf-dev \
    libxdp-dev \
    linux-tools-common \
    linux-tools-generic \
    linux-headers-$(uname -r) \
  && ln -s /usr/bin/clang-17 /usr/bin/clang \
  && ln -s /usr/bin/clang++-17 /usr/bin/clang++ \
  && ln -s /usr/include/x86_64-linux-gnu/asm /usr/include/asm \
  && sudo --user=ubuntu mkdir /home/ubuntu/project

USER ubuntu
WORKDIR /home/ubuntu/project

CMD ["sh", "./docker-build.sh"]
