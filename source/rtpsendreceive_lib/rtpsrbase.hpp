#pragma once
#include "rtpsendreceive_lib.hpp"

class RtpSRBase {
 public:
  RtpSRBase(int bufsize = 128, int samplerate = 48000,
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
  rtpsr::buffertype buffer;
  unsigned char* buf_address;
  const size_t bufsize; // buffer size in byte
  int framesize;//buffer size for each channels
  AVFormatContext* input_format_ctx;
  AVFormatContext* output_format_ctx;

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


  // AVCodecID codecid;
  // for reading audio data from streaming, to pass avioctx.
};

