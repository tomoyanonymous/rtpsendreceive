# mc.rtpsend~ / mc.rtpreceive~ 

[![Build Status](https://travis-ci.org/tomoyanonymous/rtpsendreceive.svg?branch=master)](https://travis-ci.org/tomoyanonymous/rtpsendreceive)

External objects for Cycling'74 Max to send MSP signal over network using rtp protocol. An modern alternative to legacy `netsend~` & `netreceive~` objects.


![](./screenshot.jpg)

## notes & todos

- Currently number of channels are fixed by an attribute "channels", an auto-adaptation depending on input channels is not available due to a limitation of min-api.
- A codec is fixed to Linear PCM 16bit(Big Endian). For future, Opus will be added.

## Download

Get latest version from [release](https://github.com/tomoyanonymous/rtpsendreceive/releases) page. Unzip downloaded folder and drop the root folder into Max package directory (for macOS, `~/Documents/Max 8/Packages`)if you don't need source anymore, you can delete files other than in `external`,`docs`,`help` directories.

## Build from Source

### ffmpeg

The objects use ffmpeg(libav) as its backend. To link it statically, you need to prepare static-library version of ffmpeg. Most of 3rd-party libraries are disabled in configuration except for Opus.

```bash
# brew install opus //if you do not have opus
git clone https://git.ffmpeg.org/ffmpeg.git ffmpeg && cd ffmpeg

./configure --prefix={enter installation path}  --disable-avfoundation --disable-iconv --disable-filters --disable-devices --disable-shared --enable-static  --disable-optimizations --disable-mmx --disable-audiotoolbox --disable-videotoolbox --disable-stripping --disable-appkit --disable-zlib --disable-coreimage  --disable-bzlib --disable-securetransport --disable-sdl2  --disable-lzma --enable-libopus --pkg-config-flags=--static --cc=/usr/bin/clang --cxx=/usr/bin/clang++

make -j

make install

```

For those who use homebrew in macOS, specifying installation prefix to `/usr/local/Cellar/ffmpeg/[version_name]_static` is convinient to switch between other version with a command like `brew switch ffmpeg 4.2.2_static`.

### Project Settings

Currently tested only on macOS 10.15.3. (probably works on Windows with small fixes).

You need cmake and C,C++ compiler.

```bash

# make sure to clone all submodules including min-devkit, min-api and Max api.
git clone https://github.com/tomoyanonymous/rtpsendreceive.git --recursive

cmake .

cmake --build . --target all

```

## Copyrights

Tomoya Matsuura　松浦知也 

https://matsuuratomoya.com/en 

## License

[LGPL v3.0](./License.md)

## Acknowledgements

The objects are originally made for works cooparated with [stu.inc](http://stu.inc/).


