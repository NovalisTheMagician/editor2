#!/bin/bash
ARCH=x86_64 PLATFORM=Windows MINGW=1 CC=x86_64-w64-mingw32-gcc CXX=x86_64-w64-mingw32-g++ RES=x86_64-w64-mingw32-windres PKG_CONFIG=x86_64-w64-mingw32-pkg-config make "$@"
