#include "rtpsrbase.hpp"

RtpSRBase::RtpSRBase(int framesize, int samplerate, int channels)
    : framesize(framesize),
      bufsize(sizeof(typeof(int16_t)) * framesize * channels),
      samplerate(samplerate),
      channels(channels),
      input_format_ctx(nullptr),
      output_format_ctx(nullptr),
      fmt_output(nullptr),
      fmt_input(nullptr),
      codecctx_dec(nullptr),
      codecctx_enc(nullptr),
      codec_enc(nullptr),
      codec_dec(nullptr),
      instream(nullptr),
      outstream(nullptr),
      avioctx(nullptr),
      packet(nullptr),
      frame(nullptr) {
  buffer.resize(framesize * channels);
}
RtpSRBase::~RtpSRBase() {
  // avcodec_free_context(&codecctx_enc);
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
  initDecoder();
  initEncoder();
  // //  generate global header when the format requires it
  // if (fmt_output->flags & AVFMT_GLOBALHEADER) {
  //   codecctx_enc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
  // }
  // being freed by avformat_free_context
  instream = avformat_new_stream(input_format_ctx, nullptr);
  outstream = avformat_new_stream(output_format_ctx, nullptr);
  outstream->time_base = codecctx_enc->time_base;
  instream->time_base = codecctx_enc->time_base;
  codecctx_dec->frame_size=framesize;
  codecctx_enc->frame_size=framesize;

  instream->start_time = 0;
  outstream->start_time = 0;
  auto in_setparams =
      avcodec_parameters_from_context(instream->codecpar, codecctx_enc);
  auto res_setparams =
      avcodec_parameters_from_context(outstream->codecpar, codecctx_enc);

  // if (in_setparams < 0) {
  //   std::cerr << "avcodec_parameters_from_context failed\n";
  // }
  // if (res_setparams < 0) {
  //   std::cerr << "avcodec_parameters_from_context failed\n";
  // }
  AVDictionary* options = nullptr;
  av_dict_set(&options, "live", "1", 0);
  auto res_writeheader = avformat_write_header(output_format_ctx, &options);
  if (res_writeheader < 0) {
    std::cerr << "avformat_write_header failed\n";
  }
  // av_dump_format(output_format_ctx, 0, output_format_ctx->url, 1);
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
void RtpSRBase::initDecoder() {
  codec_dec = avcodec_find_decoder(AV_CODEC_ID_PCM_S16BE);
  codecctx_dec = avcodec_alloc_context3(codec_dec);
  initCodecCtx(codecctx_dec, codec_dec, nullptr);
}
void RtpSRBase::initEncoder() {
  codec_enc = avcodec_find_encoder(AV_CODEC_ID_PCM_S16BE);
  codecctx_enc = avcodec_alloc_context3(codec_enc);
  initCodecCtx(codecctx_enc, codec_enc, nullptr);
}
void RtpSRBase::initCodecCtx(AVCodecContext* ctx, AVCodec* codec,
                             AVDictionary* codecoptions) {

  if (ctx == nullptr) {
    std::cerr << "avcodec_alloc_context3 failed\n";
  }
  ctx->sample_rate = samplerate;
  ctx->channels = channels;
  ctx->frame_size = framesize;
  ctx->sample_fmt = AV_SAMPLE_FMT_S16;
  ctx->codec_type = AVMEDIA_TYPE_AUDIO;
  ctx->audio_service_type = AV_AUDIO_SERVICE_TYPE_MAIN;
  auto layout = av_get_default_channel_layout(channels);
  ctx->channel_layout = layout;
  // codecctx_dec->time_base = av_inv_q(samplerate);
  // codecctx_dec->time_base = (AVRational){ 1, u8fps};
  int ret = avcodec_open2(ctx, codec, &codecoptions);
  if (ret < 0) {
    dumpAvError(ret);
  }
}

// decode from packet and return one frame;

int RtpSRBase::decode() {
  auto res = avcodec_send_packet(codecctx_dec, packet);
  if (res < 0) {
    dumpAvError(res);
  }
  res = avcodec_receive_frame(codecctx_dec, frame);
  return res;
}

// encode single frame and create packet

int RtpSRBase::encode() {
  int response = avcodec_send_frame(codecctx_enc, frame);
  while (response >= 0) {
    response = avcodec_receive_packet(codecctx_enc, output_packet);
    if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
      break;
    }
    if (response < 0) {
      dumpAvError(response);
    } else {
      auto itb = instream->time_base;
      auto otb = outstream->time_base;
      output_packet->pts =
          av_rescale_q_rnd(output_packet->pts, itb, otb, AV_ROUND_PASS_MINMAX);
      output_packet->dts =
          av_rescale_q_rnd(output_packet->dts, itb, otb, AV_ROUND_PASS_MINMAX);
      output_packet->duration = av_rescale_q(output_packet->duration, itb, otb);
      output_packet->pos = -1;
      av_interleaved_write_frame(output_format_ctx, output_packet);
      av_packet_unref(packet);
    }
  }
  return response;
}

int RtpSRBase::getBytesFromSamples(int samples,int chs){
  return sizeof(int16_t)*chs*samples;
 }
