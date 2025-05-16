# Dockerfile for QEMU testing
# docker build -t metacall/plthook .

FROM ubuntu:latest

RUN apt update \
    && apt install -y \
        make \
        qemu-user \
        gcc-arm-linux-gnueabi \
        gcc-arm-linux-gnueabihf \
        gcc-aarch64-linux-gnu \
        gcc-powerpc-linux-gnu \
        gcc-powerpc64le-linux-gnu \
        gcc-riscv64-linux-gnu \
        gcc-s390x-linux-gnu \
        gcc-14-loongarch64-linux-gnu \
        gcc-mips64-linux-gnuabi64 \
        gcc-mips64el-linux-gnuabi64 \
        gcc-sparc64-linux-gnu \
        libc6-dev-armhf-cross \
        libc6-dev-ppc64el-cross \
        libc6-dev-powerpc-cross \
        libc6-dev-armel-cross \
        libc6-dev-arm64-cross \
        libc6-dev-s390x-cross \
        libc6-dev-loong64-cross \
        libc6-dev-mips64-cross \
        libc6-dev-mips64el-cross \
        libc6-dev-sparc64-cross

WORKDIR /plthook

COPY . .

WORKDIR /plthook/test

ENV OPT_CFLAGS="-O2"

RUN echo "Running tests" \
    && make relro_pie_tests TARGET_PLATFORM=arm-linux-gnueabi \
    && make relro_pie_tests TARGET_PLATFORM=arm-linux-gnueabihf \
    && make relro_pie_tests TARGET_PLATFORM=aarch64-linux-gnu \
    && make relro_pie_tests TARGET_PLATFORM=aarch64-linux-gnu \
    && make relro_pie_tests TARGET_PLATFORM=powerpc-linux-gnu QEMU_ARCH=ppc \
    && make relro_pie_tests TARGET_PLATFORM=powerpc64le-linux-gnu QEMU_ARCH=ppc64le \
    && make relro_pie_tests TARGET_PLATFORM=riscv64-linux-gnu QEMU_ARCH=riscv64 \
    && make relro_pie_tests TARGET_PLATFORM=s390x-linux-gnu QEMU_ARCH=s390x \
    && make relro_pie_tests TARGET_PLATFORM=loongarch64-linux-gnu QEMU_ARCH=loongarch64 GCC_VERSION=14

    # TODO:
    #  \
    # && make relro_pie_tests TARGET_PLATFORM=mips64-linux-gnuabi64 QEMU_ARCH=mips64 \
    # && make relro_pie_tests TARGET_PLATFORM=mips64el-linux-gnuabi64 QEMU_ARCH=mips64el \
    # && make relro_pie_tests TARGET_PLATFORM=sparc64-linux-gnu QEMU_ARCH=sparc64
