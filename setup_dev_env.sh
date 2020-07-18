#!/bin/zsh

xcode-select --install

/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install.sh)"

brew cask install visual-studio-code

#install ffmpeg built from source (debug&static)

brew install cmake pkg-config nasm freetype

git clone https://git.ffmpeg.org/ffmpeg.git /tmp/ffmpeg

pushd /tmp/ffmpeg

./configure --prefix=/usr/local/Cellar/ffmpeg/HEAD_Debug  --disable-avfoundation --disable-iconv --disable-filters --disable-devices --disable-shared --enable-static  --disable-optimizations  --disable-mmx --disable-audiotoolbox --disable-videotoolbox --disable-stripping   --disable-appkit --disable-zlib --disable-coreimage  --disable-bzlib --disable-securetransport --disable-sdl2 --disable-encoder=opus --disable-decoder=opus --disable-lzma --pkg-config-flags=--static --cc=clang --cxx=clang++

make -j

make install

popd

git clone https://github.com/tomoyanonymous/rtpsendreceive.git ~/Desktop/rtpsendreceive

pushd ~/Desktop/rtpsendreceive

mkdir build && cd build && cmake ..

cmake --build . --target all

popd

