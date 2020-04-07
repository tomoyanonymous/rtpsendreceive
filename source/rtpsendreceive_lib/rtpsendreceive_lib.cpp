//  A simple ffmpeg wrapper to send/receive audio stream with rtp protocol
//  written in C++.
// Copyright 2020 Tomoya Matsuura All rights Reserved.
// This source code is released under LGPL lisence.
#include "rtpsendreceive_lib.hpp"

RtpSRBase::RtpSRBase(size_t bufsize, int samplerate, int channels)
    : bufsize(bufsize), samplerate(samplerate), channels(channels) {}
RtpSRBase::~RtpSRBase(){
  
}

void RtpSRBase::init() {
initFormatCtx();
  initCodecCtx();
  // generate global header when the format requires it
  if (fmt_output->flags & AVFMT_GLOBALHEADER) {
    avcodecctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
  }

  snprintf(avformatctx->url, sizeof(avformatctx->url), "rtp://%s:%d",
           "127.0.0.1", 30000);
  avstream = avformat_new_stream(avformatctx, codec);
  auto res_setparams =
      avcodec_parameters_from_context(avstream->codecpar, avcodecctx);
  if (res_setparams < 0) {
    std::cerr << "avcodec_parameters_from_context failed\n";
  }
  auto res_writeheader = avformat_write_header(avformatctx, nullptr);
  if (res_writeheader < 0) {
    std::cerr << "avformat_write_header failed\n";
  }
  av_dump_format(avformatctx, 0, avformatctx->url, 1);
  //   int ret = avformat_open_input(&avformatctx, nullptr, nullptr, nullptr);
  //   if (ret < 0) {
  //     std::cerr << "Could not open input\n";
  //   }
  //   ret = avformat_find_stream_info(avformatctx, nullptr);
  //   if (ret < 0) {
  //     std::cerr << "Could not find stream infos\n";
  //   }
  av_dump_format(avformatctx, 0, "test_filename", 0);
  av_init_packet(packet);
}

void RtpSRBase::initCodecCtx(AVDictionary* codecoptions) {
  codec = avcodec_find_encoder(AV_CODEC_ID_PCM_S16BE);
  avcodecctx = avcodec_alloc_context3(codec);
  if (avcodecctx == nullptr) {
    std::cerr << "avcodec_alloc_context3 failed\n";
  }
  avcodecctx->sample_rate = samplerate;
  avcodecctx->channels = channels;
  avcodecctx->audio_service_type = AV_AUDIO_SERVICE_TYPE_MAIN;
  avcodec_open2(avcodecctx, codec, &codecoptions);
}
int RtpSRBase::readPacket(void *opaque, uint8_t *buf, int buf_size) {
  return buf_size;
}
void RtpSender::sendData(rtpsr::sample_t *input) {
  AVFrame *frame;
  av_read_frame(avformatctx, packet);
  // packet->av_interleaved_write_frame(avformatctx, AVPacket *)
}
  RtpSender::RtpSender(size_t bufsize , int samplerate, int channels):RtpSRBase(){
//todo
  }
  RtpSender::~RtpSender(){
//todo
  }

void RtpSender::init() {
  std::cerr<< "hogehoge"<<std::endl;
}

void RtpSender::initFormatCtx() {
  avformatctx = avformat_alloc_context();
  avioctx = avio_alloc_context(buf_address, bufsize, 0, userdata_address,
                               RtpSRBase::readPacket, nullptr, nullptr);
  avformatctx->pb = avioctx;
  fmt_output = av_guess_format("rtp", nullptr, "audio/wav");
  avformatctx->oformat = fmt_output;

}