//  A simple ffmpeg wrapper to send/receive audio stream with rtp protocol
//  written in C++.
// Copyright 2020 Tomoya Matsuura All rights Reserved.
// This source code is released under LGPL lisence.
#include <iostream>
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"

namespace rtpsender {
using sample_t = int16_t;

struct buffer_data{
    sample_t* ptr;
    size_t size;
};

} // namespace rtpsender
class RtpSender {
public:
  RtpSender(size_t bufsize=128,int samplerate=48000,int channels=1);
  ~RtpSender();
  void init();
  const size_t getBufsizeInRawBytes(){
      return sizeof(rtpsender::sample_t)*bufsize;
  }
  void sendData(rtpsender::sample_t* input);
private:

  const int samplerate;
  const int channels;
  AVCodecContext *avcodecctx;
  AVFormatContext *avformatctx;
  AVStream* avstream;
  AVCodec* codec;
  static constexpr char* codec_name = "pcm_s16be";
  AVPacket* packet;
  AVIOContext *avioctx;
  rtpsender::buffer_data buffer;
  unsigned char* buf_address;
  void* userdata_address;
  const size_t bufsize;

  AVCodecID codecid;
  // for reading audio data from streaming, to pass avioctx.
  static int readPacket(void *opaque, uint8_t *buf, int buf_size);

};