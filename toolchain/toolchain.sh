#!/bin/bash

URL_BINUTILS="https://ftp.gnu.org/gnu/binutils/binutils-2.44.tar.xz"
URL_GCC="https://ftp.gnu.org/pub/gnu/gcc/gcc-12.2.0/gcc-12.2.0.tar.xz"
TARGET=x86_64-elf
PREFIX=$(pwd)/$TARGET

mkdir -p $PREFIX

curl -O $URL_BINUTILS
tar -xf binutils-2.44.tar.xz
mkdir -p binutils-2.44-build
cd binutils-2.44-build
../binutils-2.44/configure --target="$TARGET" --prefix="$PREFIX" --disable-nls --disable-werror
make -j$(nproc)
make install
cd ..
rm -rf binutils-2.44-build

curl -O $URL_GCC
tar -xf gcc-12.2.0.tar.xz
mkdir -p gcc-12.2.0-build
cd gcc-12.2.0-build
../gcc-12.2.0/configure --target="$TARGET" --prefix="$PREFIX"  --disable-nls --enable-languages=c --without-headers
make all-gcc -j$(nproc)
make all-target-libgcc -j$(nproc)
make install-gcc
make install-target-libgcc
cd ..
rm -rf gcc-12.2.0-build