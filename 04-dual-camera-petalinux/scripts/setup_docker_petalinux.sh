#!/usr/bin/env bash
set -euo pipefail

# Run inside an Ubuntu 22.04 Docker container as root to prepare PetaLinux 2024.2 dependencies

apt-get update && apt-get install -y \
  sudo locales build-essential gawk gcc git make net-tools ncurses-dev \
  libncurses5-dev zlib1g-dev libssl-dev flex bison libselinux1 gnupg wget \
  diffstat chrpath socat xterm autoconf libtool texinfo unzip tar cpio \
  python3 python3-pip python3-pexpect xz-utils debianutils iputils-ping \
  python3-git python3-jinja2 libegl1-mesa libsdl1.2-dev x11-utils \
  libtinfo5 dnsutils rsync bc gcc-multilib lsb-release nano

locale-gen en_US.UTF-8
update-locale LANG=en_US.UTF-8

if ! id -u builder >/dev/null 2>&1; then
    useradd -ms /bin/bash builder
    echo "builder ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers
fi

mkdir -p /build
chown -R builder:builder /build

echo "Container setup complete. Switch to the builder user with: su - builder"
