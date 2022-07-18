#!/bin/bash

arch=${1:-"RISCV"}
build_mode=${2:-"opt"}

/usr/bin/env python3 $(which scons) \
                            -C ./gem5 \
                            CCFLAGS_EXTRA="-Wall -Werror" \
                            EXTRAS=./src \
                            --linker=mold \
                            ./build/$arch/gem5.$build_mode \
                            -j $(($(nproc --all)))