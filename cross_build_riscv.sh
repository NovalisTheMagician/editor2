#!/bin/bash
ARCH=riscv64 CC=gcc CROSS_COMPILE=riscv64-linux-gnu- make "$@"
