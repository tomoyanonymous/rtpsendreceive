//  A simple ffmpeg wrapper to send/receive audio stream with rtp protocol
//  written in C++.
// Copyright 2020 Tomoya Matsuura All rights Reserved.
// This source code is released under LGPL lisence.
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/mathematics.h"

}
#include <array>
#include <iostream>
#include <vector>
#include <unistd.h>

namespace rtpsr {
using sample_t = int16_t;

struct buffer_data {
  sample_t* ptr;
  size_t size;
};
using readfn_type = int (*)(void*, uint8_t*, int);
using seekfn_type = int64_t(*)(void*, int64_t, int);

}  // namespace rtpsr
class RtpSRBase {
 public:
  RtpSRBase(size_t bufsize = sizeof(int16_t) * 512, int samplerate = 48000,
            int channels = 1);
  ~RtpSRBase();
  virtual void init();
  virtual void initFormatCtx() = 0;
  void initCodecCtx(AVDictionary* codecoptions = nullptr);

  const size_t getBufsizeInRawBytes() {
    return sizeof(rtpsr::sample_t) * bufsize;
  }

 protected:
  const int samplerate;
  const int channels;
  AVFormatContext* input_format_ctx=nullptr;
  AVFormatContext* output_format_ctx=nullptr;

  AVOutputFormat* fmt_output=nullptr;
  AVInputFormat* fmt_input=nullptr;

  AVCodecContext* avcodecctx=nullptr;

  AVStream* instream=nullptr;
  AVStream* outstream=nullptr;

  AVCodec* codec=nullptr;
  static constexpr char* codec_name = "pcm_s16be";
  AVPacket* packet=nullptr;
  AVIOContext* avioctx=nullptr;
  rtpsr::buffer_data buffer;
  unsigned char* buf_address;
  const size_t bufsize;

  // AVCodecID codecid;
  // for reading audio data from streaming, to pass avioctx.
};

class RtpSender : public RtpSRBase {
 public:
  RtpSender(rtpsr::readfn_type callback_read, rtpsr::seekfn_type,
            void* userdata_address, size_t bufsize = 128,
            int samplerate = 48000, int channels = 1);
  ~RtpSender();
  void initFormatCtx() override;
  void sendData(rtpsr::sample_t* input);
  void start();

 private:
  rtpsr::readfn_type callback_read;
  rtpsr::seekfn_type callback_seek;

  void* userdata_address;
  AVIOContext* rtp_ioctx=nullptr;
  std::vector<uint8_t> internalbuf;
  static int readPacket(void* opaque, uint8_t* buf, int buf_size);
};