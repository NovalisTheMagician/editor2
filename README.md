![Preview](image.png)

# Dependencies
- SDL2

#### Archlinux
`# pacman -S sdl2-compat`

#### Debian/Ubuntu
`# apt install libsdl2-dev`

# Building
```
$ git clone --recurse-submodules https://github.com/NovalisTheMagician/editor2
$ cd editor2
$ make -j$(nproc) CONFIG=release
```

