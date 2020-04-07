//  A simple ffmpeg wrapper to send/receive audio stream with rtp protocol
//  written in C++.
// Copyright 2020 Tomoya Matsuura All rights Reserved.
// This source code is released under LGPL lisence.
extern "C"{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}
#include <iostream>

namespace rtpsr {
using sample_t = int16_t;

struct buffer_data {
  sample_t *ptr;
  size_t size;
};

} // namespace rtpsr
class RtpSRBase {
public:
  RtpSRBase(size_t bufsize = 128, int samplerate = 48000, int channels = 1);
  ~RtpSRBase();
  virtual void init();
  virtual void initFormatCtx() = 0;
  void initCodecCtx(AVDictionary* codecoptions=nullptr);

  const size_t getBufsizeInRawBytes() {
    return sizeof(rtpsr::sample_t) * bufsize;
  }

protected:
  const int samplerate;
  const int channels;
  AVFormatContext *avformatctx;
  AVOutputFormat *fmt_output;
  AVInputFormat *fmt_input;
  AVCodecContext *avcodecctx;
  AVStream *avstream;
  AVCodec *codec;
  static constexpr char *codec_name = "pcm_s16be";
  AVPacket* packet;
  AVIOContext *avioctx;
  rtpsr::buffer_data buffer;
  unsigned char *buf_address;
  void *userdata_address;
  const size_t bufsize;

  AVCodecID codecid;
  // for reading audio data from streaming, to pass avioctx.
  static int readPacket(void *opaque, uint8_t *buf, int buf_size);
};

class RtpSender : public RtpSRBase {
  public:
  RtpSender(size_t bufsize = 128, int samplerate = 48000, int channels = 1);
  ~RtpSender();
  void initFormatCtx() override;
  void sendData(rtpsr::sample_t *input);

};