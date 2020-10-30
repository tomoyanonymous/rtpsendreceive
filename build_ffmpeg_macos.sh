#!/bin/zsh

git clone https://git.ffmpeg.org/ffmpeg.git /tmp/ffmpeg

pushd /tmp/ffmpeg

./configure \
--disable-avfoundation \
--disable-iconv \
--disable-filters \
--disable-devices \
--disable-shared \
--enable-static  \
--disable-optimizations  \
--disable-mmx \
--disable-audiotoolbox \
--disable-videotoolbox \
--disable-stripping   \
--disable-appkit \
--disable-zlib \
--disable-coreimage  \
--disable-bzlib \
--disable-securetransport \
--disable-sdl2 \
--disable-lzma \
# --enable-libopus \
--disable-libopus \
--pkg-config-flags=--static \
--prefix=/usr/local \
--cc=/usr/bin/clang \
--cxx=/usr/bin/clang++

make -j

make install

popd