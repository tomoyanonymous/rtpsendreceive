# CHANGELOG 


## 2020-10-28 v0.3.0

### Windows version is now available.

ffmpeg is now installed from vcpkg, registered as submodule.

### New Attributes

ringbuf_framenum: set the buffer size of internal ringbuffer to collect samples asynchrounously between audio thread and network. Note that the number will be multiplied to Signal Vector Size in Audio Setting. Default is x4.

### Packet Latency

"getlatency" message for `mc.rtpreceiver~` outputs packet latency from second outlet. Note that this latency just taken from RTP packets' timestamp and does not include a latency caused from buffering. Generally, large signal vector size may causes a large latency, too small buffer size causes audio glitch. 128~512 is recommended.



## 2020-10-28 v0.2.3

C++ class refactorings & cleeanups.

2 new Attributes are added.

use_rtsp(bool, default=true): you can choose either of using rtsp handshaking or raw rtp protocol.

Note that, when using raw rtp protocol, receiver assumes audio parameter from internal parameters because it does no handshaking. Thus different paramters between sender and receiver in this mode may cause audio glitch or some unexpected behaviour. 

Also, it has a bug that turning on&off of dsp quickly will cause temporary hang for 4~5 second.

## 2020-09-03 v0.2.2

Added GitHub workflow and funding pages.

### Fixed Bugs
Fixed hangs when receiver is reinstanciated before finishes connection.


## 2020-09-03 v0.2.1

### Fixed Bugs

Fixed hanging when sender/receiver could not connect to server.
Now sender also transmits packets asynchronously from audio/main thread.

## 2020-09-03 v0.2.0

This release contains many many refactorings.
Especially, receiving packets and pushing samples to object's output works asynchronously on `mc.rtpreceiver~`.
This resolved the problem that receiver blocks audio thread when no packet is arrived.

Also, an initial connection protocol has been changed from raw rtp to RTSP.

### working issue

- An attribute for choosing audio codec(PCM 16bit and Opus) has been added on this release but Opus is not working properly.
- Receiver's audio codec parameters have not been taken from an infomation given by RTSP initialization.

## 2020-08-02 v0.1.3 

- fixed a problem that the app crashes when a frame does not contain format information.

## 2020-07-30 v0.1.2

- working with multiple channels(confirmed up to 8ch)
- fixed some crashes

## 2020-05-25 v0.1.0

SUPER EARLY RELEASE BUILD for macOS.

Zip file includes all externals, XML for object infos and source codes as well.
To use the packages, drop unzipped folder into `~/Documents/Max 8/Packages`