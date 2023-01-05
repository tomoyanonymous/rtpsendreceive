# mc.rtpsend~ / mc.rtpreceive~ 

master:[![build & test](https://github.com/tomoyanonymous/rtpsendreceive/workflows/build%20&%20test/badge.svg?branch=master)](https://github.com/tomoyanonymous/rtpsendreceive/actions?query=workflow%3A%22build+%26+test%22) dev: [![build & test](https://github.com/tomoyanonymous/rtpsendreceive/workflows/build%20&%20test/badge.svg?branch=dev)](https://github.com/tomoyanonymous/rtpsendreceive/actions?query=workflow%3A%22build+%26+test%22)

*2022/01/06: This project is not actively maintained. [jit.ndi](https://github.com/pixsper/jit.ndi) also supports the transmission of uncompressed audio over local network and it should be more stable now. If you want to continue to maintain or develop this project, I will support. Just send PR or contact me.*

External objects for Cycling'74 Max to send MSP signal over network using rtp protocol. An modern alternative to legacy `netsend~` & `netreceive~` objects.

![](./screenshot.jpg)

- Send mc audio signal through 16bit-integer PCM signals.
- Sending signals over NAT (= Global Network) is not supported. Consider use VPN.

## notes & todos

- **When use this external, in Max's "Audio Setting", "IO Vector Size" and "Signal Vector Size" must be the same. If signal vector size is too smaller than IO vector size, it fails to send audio correctly.**
- Currently number of channels are fixed by an attribute "channels", an auto-adaptation depending on input channels is not available due to a limitation of min-api.
- A codec is fixed to Linear PCM 16bit(Big Endian). For future, Opus will be added.

## Installation

Get latest version from [release](https://github.com/tomoyanonymous/rtpsendreceive/releases) page. 

Unzip downloaded folder and drop the root folder into Max package directory (for macOS, `~/Documents/Max 8/Packages`)
if you don't need source anymore, you can delete files other than in `externals`,`docs`,`help` directories.

## Build from Source

You need cmake and C,C++ compiler. This project uses ffmpeg library for rtp streaming.
This project installs ffmpeg using [vcpkg](https://github.com/microsoft/vcpkg), a cross-platform paackage manager registered as submodule at `./vcpkg`.

For a more detailed workflow, read GitHub Actions workflow in `./.github/workflows/build_and_test.yml`.

### macOS

Currently tested on macOS 10.15.3, XCode 11.4.1.

Open a terminal and type following commands.

```bash
git clone https://github.com/tomoyanonymous/rtpsendreceive.git --recursive
cd rtpsendreceive

./vcpkg/bootstrap-vcpkg.sh
./vcpkg/vcpkg install ffmpeg\[avformat,avdevice,avcodec,core\]

cmake . -B build
cmake --build build --target all
```
### Windows

Currently tested on Visual Studio Community 2019

Open "Developer Command Prompt for VS 2019" and type following commands.

```cmd

git clone https://github.com/tomoyanonymous/rtpsendreceive.git --recursive
cd rtpsendreceive

.\vcpkg\bootstrap-vcpkg.bat
.\vcpkg\vcpkg install ffmpeg[avformat,avdevice,avcodec,core]:x64-windows

cmake . -B build
cmake --build build --target all
```

## Copyrights

Tomoya Matsuura　松浦知也 

https://matsuuratomoya.com/en 

## License

[LGPL v3.0](./License.md)

## Acknowledgements

The objects are originally made for works cooparated with [stu.inc](http://stu.inc/).


