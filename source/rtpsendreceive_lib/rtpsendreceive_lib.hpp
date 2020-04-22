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

using buffertype = std::vector<sample_t>;

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
  static void dumpAvError(int error_code);
 protected:
 int decode(AVFormatContext* fmtctx,AVStream* instream,AVStream* outstream,AVCodecContext* cdcctx,AVFrame* frame,AVPacket* packet,int stream_index);
  int encode(AVFormatContext* fmtctx,AVStream* instream,AVStream* outstream,AVCodecContext* cdcctx,AVFrame* frame,AVPacket* packet, int stream_index);
  const int samplerate;
  const int channels;
  AVFormatContext* input_format_ctx=nullptr;
  AVFormatContext* output_format_ctx=nullptr;

  AVOutputFormat* fmt_output=nullptr;
  AVInputFormat* fmt_input=nullptr;

  AVCodecContext* codecctx_dec=nullptr;
  AVCodecContext* codecctx_enc=nullptr;

  AVStream* instream=nullptr;
  AVStream* outstream=nullptr;

  AVCodec* codec_dec=nullptr;
  AVCodec* codec_enc=nullptr;
  AVPacket* packet=nullptr;
  AVFrame* frame = nullptr;
  AVIOContext* avioctx=nullptr;
  rtpsr::buffertype buffer;
  unsigned char* buf_address;
  const size_t bufsize;

  // AVCodecID codecid;
  // for reading audio data from streaming, to pass avioctx.
};

class RtpSender : public RtpSRBase {
 public:
  RtpSender(int bufsize = 128,
            int samplerate = 48000, int channels = 1,const std::string& address="127.0.0.1",const int port=30000 ,rtpsr::readfn_type callback_read = readPacketSelf,rtpsr::seekfn_type callback_seek=nullptr,
            void* userdata=nullptr);
  ~RtpSender();
  void initFormatCtx() override;
  void sendData();
  void start();
  void writeBuffer(double sample,int pos,int channel_idx);
  auto* getBuffer_ptr(){
    return reinterpret_cast<uint8_t*>(buffer.data());
  }
  void setDestination(std::string& ad,int po);
 private:
  rtpsr::readfn_type callback_read;
  rtpsr::seekfn_type callback_seek;
  std::string address;
  int port;
  void* userdata_address;
  AVIOContext* rtp_ioctx=nullptr;
  uint8_t* internalbuf;
  static int readPacketSelf(void* userdata, uint8_t* buf, int buf_size);
};