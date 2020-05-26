# mc.rtpsend~ / mc.rtpreceive~ 

[![Build Status](https://travis-ci.org/tomoyanonymous/rtpsendreceive.svg?branch=master)](https://travis-ci.org/tomoyanonymous/rtpsendreceive)

An modern alternative to netsend~ & netreceive~ objects using rtp protocol.

![](./screenshot.jpg)

## notes

Currently number of channels are fixed by an attribute "channels", an auto-adaptation depending on input channels is not available due to a limitation of min-api.

A codec is also fixed to Linear PCM 16bit(Big Endian).

### ffmpeg

The objects use ffmpeg(libav) as its backend. To link it statically, you need to prepare static-library version of ffmpeg. Most of 3rd-party libraries are disabled in configuration.

```bash

git clone https://git.ffmpeg.org/ffmpeg.git ffmpeg && cd ffmpeg

./configure --prefix={enter installation path}  --disable-avfoundation --disable-iconv --disable-filters --disable-devices --disable-shared --enable-static  --disable-optimizations --disable-mmx --disable-audiotoolbox --disable-videotoolbox --disable-stripping --disable-appkit --disable-zlib --disable-coreimage  --disable-bzlib --disable-securetransport --disable-sdl2 --disable-encoder=opus --disable-decoder=opus --disable-lzma --pkg-config-flags=--static 

make -j

make install

```

For those who use homebrew in macOS, specifying installation prefix to `/usr/local/Cellar/ffmpeg/[version_name]_static` is convinient to switch between other version with a command like `brew switch ffmpeg 4.2.2_static`.

### build from source

You need to cmake and C,C++ compiler.

```bash

# make sure to clone all submodules including min-devkit, min-api and Max api.
git clone https://github.com/tomoyanonymous/rtpsendreceive.git --recursive

cmake .

cmake --build . --target all

```

After build process, copy whole directory into your Max package directory (for macOS, `~/Documents/Max 8/Packages`)if you don't need source anymore, you can delete files other than in `external`,`docs`,`help` directories.

## License

[LGPL v3.0](./License.md)

## Author

Tomoya Matsuura

https://matsuuratomoya.com

## Acknowledgements

The objects are originally made for works cooparated with [stu.inc](http://stu.inc/).


