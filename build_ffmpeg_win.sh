git clone https://git.ffmpeg.org/ffmpeg.git /tmp/ffmpeg


pacman --sync --noconfirm --needed base-devel
pacman --sync --noconfirm --needed p7zip
pacman --sync --noconfirm --needed zlib

wget http://www.tortall.net/projects/yasm/releases/yasm-1.3.0-win64.exe -O /bin/yasm.exe

pushd /tmp/ffmpeg

./configure \
    --disable-gpl \
    --disable-filters \
    --disable-devices \
    --disable-shared \
    --enable-static  \
    --enable-x86asm \
    --arch=amd64 \
    --cpu=amd64 \
    --prefix=/usr \
    --toolchain=msvc 

make -j

make install

popd