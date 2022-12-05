#!/bin/bash

arch=${1:-"X86"}
build_mode=${2:-"opt"}

/usr/bin/env python3 $(which scons) \
                            -C ./gem5 \
                            CCFLAGS_EXTRA="-Wall" \
                            EXTRAS=./src \
                            ./build/$arch/gem5.$build_mode \
                            -j $(($(nproc --all)/2))