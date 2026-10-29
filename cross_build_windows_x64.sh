#!/bin/bash
ARCH=x86_64 PLATFORM=Windows MINGW=1 CROSS_COMPILE=x86_64-w64-mingw32- make "$@"
