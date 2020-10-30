git clone https://git.ffmpeg.org/ffmpeg.git /tmp/ffmpeg


pacman --sync --noconfirm --needed base-devel
pacman --sync --noconfirm --needed p7zip
pacman --sync --noconfirm --needed zlib

wget http://www.tortall.net/projects/yasm/releases/yasm-1.3.0-win64.exe -O /bin/yasm.exe

pushd /tmp/ffmpeg

./configure \
# --disable-avfoundation \
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
--arch=amd64 \
--cpu=amd64 \
--toolchain=msvc 

make -j

make install

popd