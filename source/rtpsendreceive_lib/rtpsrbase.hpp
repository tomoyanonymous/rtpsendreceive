#pragma once
#include "rtpsendreceive_lib.hpp"


class RtpSRBase {
 public:
  explicit RtpSRBase(int framesize = 128, int samplerate = 48000,
            int channels = 1,rtpsr::Codec codec=rtpsr::Codec::PCM_s16BE);
  ~RtpSRBase();
  virtual void init();
  virtual void initFormatCtx() = 0;
  void initCodecCtx(AVCodecContext* ctx,AVCodec* codec,AVDictionary* codecoptions = nullptr);
  void initEncoder();
  void initDecoder();
  static void dumpAvError(int error_code);
 protected:
 static int getBytesFromSamples(int samples,int chs);
 int decode();
  int encode();
  auto getCodecSampleFormat(){
    return (this->codec==rtpsr::Codec::PCM_s16BE)?AV_SAMPLE_FMT_S16:AV_SAMPLE_FMT_DBL;
  }
  const int samplerate;
  const int channels;
  rtpsr::buffertype buffer;
  const size_t bufsize; // buffer size in byte
  int framesize;//buffer size for each channels
  rtpsr::Codec codec;
  int64_t bitrate = 192000; //bitrate for lossy compression 

  AVFormatContext* input_format_ctx;
  AVFormatContext* output_format_ctx;

  AVOutputFormat* fmt_output;
  AVInputFormat* fmt_input;

  AVCodecContext* codecctx_dec;
  AVCodecContext* codecctx_enc;

  AVStream* instream;
  AVStream* outstream;

  AVCodec* codec_dec;
  AVCodec* codec_enc;

  AVPacket* packet;
  AVPacket* output_packet;

  AVFrame* frame;
  AVIOContext* avioctx;


  // AVCodecID codecid;
  // for reading audio data from streaming, to pass avioctx.
};

