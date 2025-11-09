#!/bin/bash

BINUTILS_VERSION=2.37
GCC_VERSION=11.2.0

BINUTILS_URL="https://ftp.gnu.org/gnu/binutils/binutils-${BINUTILS_VERSION}.tar.xz"
GCC_URL="https://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.xz"

# ---------------------------

set -e

TOOLCHAINS_DIR=
OPERATION='build'

while test $# -gt 0
do
    case "$1" in
        -c) OPERATION='clean'
            ;;
        *)  TOOLCHAINS_DIR="$1"
            ;;
    esac
    shift
done

if [ -z "$TOOLCHAINS_DIR" ]; then
    echo "Missing arg: toolchains directory"
    exit 1
fi

pushd "$TOOLCHAINS_DIR"

if [ "$OPERATION" = "build" ]; then

    # Download and build binutils
    BINUTILS_SRC="binutils-${BINUTILS_VERSION}"
    BINUTILS_BUILD="binutils-build-${BINUTILS_VERSION}"
    BINUTILS_ARCHIVE="binutils-${BINUTILS_VERSION}.tar.xz"

    if [ ! -d "${BINUTILS_SRC}" ]; then
        if [ ! -f "${BINUTILS_ARCHIVE}" ]; then
            wget -O "${BINUTILS_ARCHIVE}" ${BINUTILS_URL}
        fi
        tar -xf "${BINUTILS_ARCHIVE}"
    fi

    if [ ! -f "bin/i686-elf-as" ] || [ ! -f "bin/i686-elf-ld" ]; then
        mkdir -p ${BINUTILS_BUILD}
        cd ${BINUTILS_BUILD}
        CFLAGS= ASMFLAGS= CC= CXX= LD= ASM= LINKFLAGS= LIBS= ../binutils-${BINUTILS_VERSION}/configure \
            --prefix="$(pwd)/../"	\
            --target=i686-elf				\
            --with-sysroot					\
            --disable-nls					\
            --disable-werror
        make -j8
        make install
        cd ..
    else
        echo "Binutils already built, skipping..."
    fi

    # Download and build GCC
    GCC_SRC="gcc-${GCC_VERSION}"
    GCC_BUILD="gcc-build-${GCC_VERSION}"
    GCC_ARCHIVE="gcc-${GCC_VERSION}.tar.xz"

    if [ ! -d "${GCC_SRC}" ]; then
        if [ ! -f "${GCC_ARCHIVE}" ]; then
            wget -O "${GCC_ARCHIVE}" ${GCC_URL}
        fi
        tar -xf "${GCC_ARCHIVE}"
    fi
    if [ ! -f "bin/i686-elf-gcc" ] || [ ! -f "lib/gcc/i686-elf/${GCC_VERSION}/libgcc.a" ]; then
        mkdir -p ${GCC_BUILD}
        cd ${GCC_BUILD}
        CFLAGS= ASMFLAGS= CC= CXX= LD= ASM= LINKFLAGS= LIBS= ../gcc-${GCC_VERSION}/configure \
            --prefix="$(pwd)/../" 	\
            --target=i686-elf				\
            --disable-nls					\
            --enable-languages=c,c++		\
            --without-headers
        make -j8 all-gcc all-target-libgcc
        make install-gcc install-target-libgcc
        cd ..
    else
        echo "GCC already built, skipping..."
    fi

elif [ "$OPERATION" = "clean" ]; then
	rm -rf *
fi

popd