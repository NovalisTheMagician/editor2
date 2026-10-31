#!/bin/bash
CCFLAGS=-I../SDL2-2.32.10/build/include -I../SDL2-2.32.10/include/ SDL_INC=-I../SDL2-2.32.10/include/ LDFLAGS=-L../SDL2-2.32.10/build CC=gcc NO_PKGCONFIG=1 mingw32-make "$@"
