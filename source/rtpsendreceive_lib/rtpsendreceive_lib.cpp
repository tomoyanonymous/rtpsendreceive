//  A simple ffmpeg wrapper to send/receive audio stream with rtp protocol
//  written in C++.
// Copyright 2020 Tomoya Matsuura All rights Reserved.
// This source code is released under LGPL lisence.
#include "rtpsendreceive_lib.hpp"

#include <utility>

RtpSRBase::RtpSRBase(size_t bufsize, int samplerate, int channels)
    : bufsize(bufsize),
      samplerate(samplerate),
      channels(channels),
      input_format_ctx(nullptr),
      output_format_ctx(nullptr),
      codecctx_dec(nullptr),
      codecctx_enc(nullptr),
      codec_enc(nullptr),
      codec_dec(nullptr),
      instream(nullptr),
      outstream(nullptr),
      avioctx(nullptr),
      packet(nullptr) {}
RtpSRBase::~RtpSRBase() {
  avcodec_free_context(&codecctx_enc);
  avformat_free_context(output_format_ctx);
  avformat_free_context(input_format_ctx);
  avio_context_free(&avioctx);
}
void RtpSRBase::dumpAvError(int error_code) {
  char str[4096];
  av_make_error_string(str, 4096, error_code);
  std::cerr << str << "\n";
}

void RtpSRBase::init() {
  packet = av_packet_alloc();
  frame = av_frame_alloc();
  initFormatCtx();
  initCodecCtx();
  //  generate global header when the format requires it
  if (fmt_output->flags & AVFMT_GLOBALHEADER) {
    codecctx_enc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
  }
  // being freed by avformat_free_context
  instream = avformat_new_stream(input_format_ctx, codec_enc);
  outstream = avformat_new_stream(output_format_ctx, codec_enc);
  outstream->time_base = codecctx_enc->time_base;
  instream->time_base = codecctx_enc->time_base;
  auto in_setparams =
      avcodec_parameters_from_context(instream->codecpar, codecctx_enc);

  auto res_setparams =
      avcodec_parameters_from_context(outstream->codecpar, codecctx_enc);

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
  av_sdp_create(&output_format_ctx, 1, testchar.data(), testchar.size());

  std::string teststr = testchar.data();
  std::cout << "----sdp-----\nSDP:\n" << teststr << "---------\n" << std::endl;
}

void RtpSRBase::initCodecCtx(AVDictionary* codecoptions) {
  codec_enc = avcodec_find_encoder(AV_CODEC_ID_PCM_S16LE);

  codecctx_enc = avcodec_alloc_context3(codec_enc);
  if (codecctx_enc == nullptr) {
    std::cerr << "avcodec_alloc_context3 failed\n";
  }
  // codec_dec = avcodec_find_decoder(AV_CODEC_ID_PCM_S16BE);
  // codecctx_dec = avcodec_alloc_context3(codec_dec);
  // if (codecctx_dec == nullptr) {
  //   std::cerr << "avcodec_alloc_context3 failed\n";
  // }
  codecctx_enc->sample_rate = samplerate;
  codecctx_enc->channels = channels;
  codecctx_enc->sample_fmt = AV_SAMPLE_FMT_S16;
  codecctx_enc->codec_type = AVMEDIA_TYPE_AUDIO;
  codecctx_enc->audio_service_type = AV_AUDIO_SERVICE_TYPE_MAIN;
  codecctx_enc->channel_layout = AV_CH_LAYOUT_MONO;
  // codecctx_dec->time_base = av_inv_q(samplerate);
  // codecctx_dec->time_base = (AVRational){ 1, u8fps};
  int ret = avcodec_open2(codecctx_enc, codec_enc, &codecoptions);
  if (ret < 0) {
    dumpAvError(ret);
  }
}

int RtpSRBase::decode(AVFormatContext* fmtctx, AVStream* instream,
                      AVStream* outstream, AVCodecContext* cdcctx,
                      AVFrame* frame, AVPacket* packet, int stream_index) {
  return 0;
}
int RtpSRBase::encode(AVFormatContext* fmtctx, AVStream* instream,
                      AVStream* outstream, AVCodecContext* cdcctx,
                      AVFrame* frame, AVPacket* packet, int stream_index) {
  AVPacket* output_packet = av_packet_alloc();
  int response = avcodec_send_frame(cdcctx, frame);

  while (response >= 0) {
    response = avcodec_receive_packet(cdcctx, output_packet);
    if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
      break;
    } else if (response < 0) {
      return -1;
    }
    output_packet->stream_index = stream_index;
    output_packet->duration =
        instream->time_base.den / outstream->time_base.num /
        instream->avg_frame_rate.num * instream->avg_frame_rate.den;

    av_packet_rescale_ts(output_packet, instream->time_base,
                         outstream->time_base);
    response = av_interleaved_write_frame(fmtctx, output_packet);
  }
  av_packet_unref(output_packet);
  av_packet_free(&output_packet);
  return 0;
}

RtpSender::RtpSender(int bufsize, int samplerate, int channels,
                     const std::string&  address, const int port,
                     rtpsr::readfn_type callback_read,
                     rtpsr::seekfn_type callback_seek, void* userdata)
    : rtp_ioctx(nullptr),
    address(std::move(address)),
    port(port),
      callback_read(callback_read),
      callback_seek(callback_seek),
      RtpSRBase() {
  if (userdata == nullptr) {
    userdata_address = (void*)this;
  }
  // todo
}
RtpSender::~RtpSender() {
  // todo
  av_packet_unref(packet);
  avio_close(rtp_ioctx);
  av_free(internalbuf);
}

// void RtpSender::init() {
//   std::cerr<< "hogehoge"<<std::endl;
// }

void RtpSender::initFormatCtx() {
  // for audio driver->ffmpeg
  std::string destination = "rtp://" + address + ":" + std::to_string(port);
  auto size = sizeof(int16_t) * 512;
  internalbuf = (uint8_t*)av_malloc(4096);
  avioctx = avio_alloc_context(internalbuf, 4096, 0, userdata_address,
                               callback_read, nullptr, callback_seek);
  // Determine the input-format:
  AVProbeData probeData{"", internalbuf, 512, "audio/L16"};
  probeData.buf = internalbuf;
  input_format_ctx = avformat_alloc_context();
  input_format_ctx->pb = avioctx;
  input_format_ctx->flags |= AVFMT_FLAG_CUSTOM_IO;
  input_format_ctx->iformat = av_probe_input_format(&probeData, 1);
  // input_format_ctx->iformat = av_find_input_format("audio/L16");
  int inputres = avformat_open_input(&input_format_ctx, "", nullptr, nullptr);
  if (inputres < 0) {
    dumpAvError(inputres);
  }
  // usleep(100000);
  fmt_output = av_guess_format("rtp", destination.c_str(), "audio/L16");
  output_format_ctx = avformat_alloc_context();
  output_format_ctx->pb = rtp_ioctx;
  auto res =
      avio_open(&output_format_ctx->pb, destination.c_str(), AVIO_FLAG_WRITE);
  if (res < 0) {
    std::cerr << "avio open error\n";
  }
  output_format_ctx->oformat = fmt_output;
  auto url = new char[destination.size() + 1];
  std::char_traits<char>::copy(url, destination.c_str(),
                               destination.size() + 1);
  output_format_ctx->url = url;
  // delete url;
  av_new_packet(packet, 4096);
}

void RtpSender::setDestination(std::string& ad, int po) {
  address = ad;
  port = po;
}

void RtpSender::start() {
  // av_new_packet(packet, 4096);
  int count = 0;
  while (true) {
    auto ret = av_read_frame(input_format_ctx, packet);
    // int response = avcodec_send_packet(codecctx_dec, packet);
    // dumpAvError(response);
    // while (response >= 0) {
    //   response = avcodec_receive_frame(codecctx_dec, frame);
    //   if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
    //     break;
    //   } else if (response < 0) {
    //     dumpAvError(response);
    //     // throw std::runtime_error(errbuf);
    //   }
    //   if (response >= 0) {
    //     encode(output_format_ctx, instream, outstream, codecctx_dec, frame,
    //            packet, 0);
    //     usleep(100000);
    //   }
    //   av_frame_unref(frame);
    // }

    // // auto* stream = input_format_ctx->streams[packet->stream_index];
    packet->pts = av_rescale_q_rnd(packet->pts, instream->time_base,
                                   outstream->time_base, AV_ROUND_PASS_MINMAX);
    packet->dts = av_rescale_q_rnd(packet->dts, instream->time_base,
                                   outstream->time_base, AV_ROUND_PASS_MINMAX);
    packet->duration = av_rescale_q(packet->duration, instream->time_base,
                                    outstream->time_base);
    packet->pos = -1;

    // std::cout << "buffer count : " << count++ << "\n";
    av_packet_rescale_ts(packet, instream->time_base, outstream->time_base);

    av_interleaved_write_frame(output_format_ctx, packet);
    usleep(512 * 1000000. / 48000);
  }
}

void RtpSender::writeBuffer(double sample, int pos, int channel_idx) {
  buffer[pos + channel_idx] = static_cast<rtpsr::sample_t>(sample);
}

void RtpSender::sendData() {
  av_init_packet(packet);
  auto ret = av_read_frame(input_format_ctx, packet);
  packet->pts = av_rescale_q_rnd(packet->pts, instream->time_base,
                                 outstream->time_base, AV_ROUND_PASS_MINMAX);
  packet->dts = av_rescale_q_rnd(packet->dts, instream->time_base,
                                 outstream->time_base, AV_ROUND_PASS_MINMAX);
  packet->duration =
      av_rescale_q(packet->duration, instream->time_base, outstream->time_base);
  packet->pos = -1;

  av_packet_rescale_ts(packet, instream->time_base, outstream->time_base);
  av_interleaved_write_frame(output_format_ctx, packet);

  av_packet_unref(packet);
}

int RtpSender::readPacketSelf(void* userdata, uint8_t* buf, int buf_size) {
  auto* sender = reinterpret_cast<RtpSender*>(userdata);
  auto* address = sender->getBuffer_ptr();
  memcpy(buf, address, buf_size);
  return 0;
}