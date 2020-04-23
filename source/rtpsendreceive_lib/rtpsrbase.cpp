#include "rtpsrbase.hpp"

RtpSRBase::RtpSRBase(int framesize, int samplerate, int channels)
    : framesize(framesize),
      bufsize(sizeof(typeof(framesize))*framesize),
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
      packet(nullptr) {
        buffer.resize(framesize*channels);
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
  codec_enc = avcodec_find_encoder(AV_CODEC_ID_PCM_S16BE);

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
  auto layout =av_get_default_channel_layout(channels);
  codecctx_enc->channel_layout =   layout;
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