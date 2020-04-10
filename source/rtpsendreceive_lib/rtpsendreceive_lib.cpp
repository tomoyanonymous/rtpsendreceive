//  A simple ffmpeg wrapper to send/receive audio stream with rtp protocol
//  written in C++.
// Copyright 2020 Tomoya Matsuura All rights Reserved.
// This source code is released under LGPL lisence.
#include "rtpsendreceive_lib.hpp"

RtpSRBase::RtpSRBase(size_t bufsize, int samplerate, int channels)
    : bufsize(bufsize),
      samplerate(samplerate),
      channels(channels),
      input_format_ctx(nullptr),
      output_format_ctx(nullptr),
      avcodecctx(nullptr),
      codec(nullptr),
      instream(nullptr),
      outstream(nullptr),
      avioctx(nullptr),
      packet(nullptr)
{}
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
  instream->time_base = avcodecctx->time_base;
  auto in_setparams =
      avcodec_parameters_from_context(instream->codecpar, avcodecctx);

  auto res_setparams =
      avcodec_parameters_from_context(outstream->codecpar, avcodecctx);

  if (in_setparams < 0) {
    std::cerr << "avcodec_parameters_from_context failed\n";
  }
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
  avcodecctx->sample_fmt = AV_SAMPLE_FMT_S16;
  avcodecctx->codec_id = AV_CODEC_ID_PCM_S16BE;
  avcodecctx->codec_type = AVMEDIA_TYPE_AUDIO;
  avcodecctx->audio_service_type = AV_AUDIO_SERVICE_TYPE_MAIN;
  avcodecctx->channel_layout = AV_CH_LAYOUT_MONO;

  // avcodecctx->time_base = (AVRational){ 1, u8fps};
  avcodec_open2(avcodecctx, codec, &codecoptions);
}

void RtpSender::sendData(rtpsr::sample_t *input) {
  AVFrame *frame;
  // av_read_frame(avformatctx, packet);
  // packet->av_interleaved_write_frame(avformatctx, AVPacket *)
}
RtpSender::RtpSender(rtpsr::readfn_type callback_read,
                     rtpsr::seekfn_type callback_seek, void *userdata_address,
                     size_t bufsize, int samplerate, int channels)
    :
    rtp_ioctx(nullptr), 
    callback_read(callback_read),
      callback_seek(callback_seek),
      userdata_address(userdata_address),
      RtpSRBase() {
  // todo
  internalbuf.resize(sizeof(int16_t) * bufsize);
}
RtpSender::~RtpSender() {
  // todo
  avio_close(rtp_ioctx);
}

// void RtpSender::init() {
//   std::cerr<< "hogehoge"<<std::endl;
// }

void RtpSender::initFormatCtx() {
  // for audio driver->ffmpeg
  input_format_ctx = avformat_alloc_context();
  output_format_ctx = avformat_alloc_context();

  avioctx = avio_alloc_context(internalbuf.data(), sizeof(int16_t) * 512, 0,
                               userdata_address, callback_read, nullptr,
                               callback_seek);
  // Determine the input-format:
  AVProbeData probeData{"", internalbuf.data(),
                        static_cast<int>(sizeof(int16_t) * 512), "audio/L16"};
  probeData.buf = internalbuf.data();
  input_format_ctx->pb = avioctx;
  input_format_ctx->flags |= AVFMT_FLAG_CUSTOM_IO;

  input_format_ctx->iformat = av_probe_input_format(&probeData, 1);
  // input_format_ctx->iformat = av_find_input_format("s16be");
  usleep(100000);
  int inputres = avformat_open_input(&input_format_ctx, "", nullptr, nullptr);
  if (inputres < 0) {
    char buf[4096];
    std::cerr << av_make_error_string(buf, 4096, inputres) << "\n";
  }
  usleep(100000);
  fmt_output = av_guess_format("rtp", "rtp://127.0.0.1:30000", "audio/L16");
  auto res = avio_open(&rtp_ioctx, "rtp://127.0.0.1:30000", AVIO_FLAG_WRITE);
  if (res < 0) {
    std::cerr << "avio open error\n";
  }
  output_format_ctx->pb = rtp_ioctx;
  output_format_ctx->oformat = fmt_output;

  auto url = new char[]{"rtp://127.0.0.1:30000"};
  output_format_ctx->url = url;
}

void RtpSender::start() {
  packet = av_packet_alloc();
  av_new_packet(packet, sizeof(int16_t) * 512);
  int count= 0;
  while (true) {
    auto ret = av_read_frame(input_format_ctx, packet);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
      break;
    else if (ret < 0) {
      fprintf(stderr, "error encoding audio frame\n");
      exit(1);
    }
    std::cout << "buffer count : " << count++ << "\n";
    usleep(100000);
    av_interleaved_write_frame(output_format_ctx, packet);
    
    av_packet_unref(packet);
  }
}

int RtpSender::readPacket(void *opaque, uint8_t *buf, int buf_size) {
  return buf_size;
}