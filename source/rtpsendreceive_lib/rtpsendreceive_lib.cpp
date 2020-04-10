//  A simple ffmpeg wrapper to send/receive audio stream with rtp protocol
//  written in C++.
// Copyright 2020 Tomoya Matsuura All rights Reserved.
// This source code is released under LGPL lisence.
#include "rtpsendreceive_lib.hpp"

RtpSRBase::RtpSRBase(size_t bufsize, int samplerate, int channels)
    : bufsize(bufsize), samplerate(samplerate), channels(channels) {}
RtpSRBase::~RtpSRBase() {
  avcodec_free_context(&avcodecctx);
  avformat_free_context(output_format_ctx);
  avformat_free_context(input_format_ctx);
  avio_context_free(&avioctx);
}

void RtpSRBase::init() {
  initFormatCtx();
  initCodecCtx();
  //  generate global header when the format requires it
  if (fmt_output->flags & AVFMT_GLOBALHEADER) {
    avcodecctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
  }
  // being freed by avformat_free_context
  instream = avformat_new_stream(input_format_ctx, codec);
  outstream = avformat_new_stream(output_format_ctx, codec);
  outstream->time_base = avcodecctx->time_base;
  auto res_setparams =
      avcodec_parameters_from_context(outstream->codecpar, avcodecctx);
  if (res_setparams < 0) {
    std::cerr << "avcodec_parameters_from_context failed\n";
  }
  auto res_writeheader = avformat_write_header(output_format_ctx, nullptr);
  if (res_writeheader < 0) {
    std::cerr << "avformat_write_header failed\n";
  }
  av_dump_format(output_format_ctx, 0, output_format_ctx->url, 1);
  //   int ret = avformat_open_input(&avformatctx, nullptr, nullptr, nullptr);
  // if (ret < 0) {
  //     std::cerr << "Could not open input\n";
  //   }
  // ret = avformat_find_stream_info(avformatctx, nullptr);
  //   if (ret < 0) {
  //     std::cerr << "Could not find stream infos\n";
  //   }
  std::array<char, 4096> testchar{};
  packet = av_packet_alloc();
  av_sdp_create(&output_format_ctx, 1, testchar.data(), testchar.size());
  std::string teststr = testchar.data();
  std::cout << teststr << std::endl;
}

void RtpSRBase::initCodecCtx(AVDictionary *codecoptions) {
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

void RtpSender::sendData(rtpsr::sample_t *input) {
  AVFrame *frame;
  // av_read_frame(avformatctx, packet);
  // packet->av_interleaved_write_frame(avformatctx, AVPacket *)
}
RtpSender::RtpSender(rtpsr::readfn_type callback_read, void *userdata_address,
                     size_t bufsize, int samplerate, int channels)
    : callback_read(callback_read),
      userdata_address(userdata_address),
      RtpSRBase() {
  // todo
}
RtpSender::~RtpSender() {
  // todo
}

// void RtpSender::init() {
//   std::cerr<< "hogehoge"<<std::endl;
// }

void RtpSender::initFormatCtx() {
  // for audio driver->ffmpeg
  input_format_ctx = avformat_alloc_context();
  avioctx = avio_alloc_context(buf_address, bufsize, 0, userdata_address,
                               callback_read, nullptr, nullptr);
  input_format_ctx->pb = avioctx;

  output_format_ctx = avformat_alloc_context();
  fmt_output = av_guess_format("rtp", "rtp://127.0.0.1/30000", "audio/wav");
  auto res = avio_open(&rtp_ioctx,"rtp://127.0.0.1/30000",AVIO_FLAG_WRITE);
  if(res<0){
    std::cerr<<"avio open error\n";
  }
  output_format_ctx->pb = rtp_ioctx;
  output_format_ctx->oformat = fmt_output;
  auto url  =  new char[]{"rtp://127.0.0.1/30000"};
  output_format_ctx->url = url;

}

void RtpSender::start() {
  av_init_packet(packet);

  while (true) {
    auto ret = av_read_frame(input_format_ctx, packet);
    if (ret < 0) {
      break;
    }
    av_interleaved_write_frame(output_format_ctx, packet);
    av_packet_unref(packet);
  }
}

int RtpSender::readPacket(void *opaque, uint8_t *buf, int buf_size) {
  return buf_size;
}