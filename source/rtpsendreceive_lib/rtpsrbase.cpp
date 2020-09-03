#include "rtpsrbase.hpp"

namespace rtpsr {
CustomCbInFormat::CustomCbInFormat(RtpSRSetting& s) : InFormat(s) {
  buffer.resize(setting.framesize*setting.channels);
  auto bufsize = getBufSize(s);
  auto* aviobuf = static_cast<uint8_t*>(av_malloc(bufsize));
  auto* avioctx = avio_alloc_context(aviobuf, bufsize, 0, this, readPacket,
                                     nullptr, nullptr);
  ctx->flags |= AVFMT_FLAG_CUSTOM_IO;
  ctx->pb = avioctx;
}
int CustomCbInFormat::readPacket(void* userdata, uint8_t* avio_buf,
                                 int buf_size) {
  auto* sender = reinterpret_cast<CustomCbInFormat*>(userdata);
  auto* address = sender->buffer.data();
  getBufSize(sender->setting);
  memcpy(avio_buf, address, buf_size);
  return buf_size;
}
CustomCbOutFormat::CustomCbOutFormat(RtpSRSetting& s) : OutFormat(s) {
  buffer.resize(setting.framesize);

  auto bufsize = getBufSize(s);
  auto* aviobuf = static_cast<uint8_t*>(av_malloc(bufsize));
  auto* avioctx = avio_alloc_context(aviobuf, bufsize, 1, this, nullptr,
                                     writePacket, nullptr);
  ctx->flags |= AVFMT_FLAG_CUSTOM_IO;
  ctx->pb = avioctx;
}
int CustomCbOutFormat::writePacket(void* userdata, uint8_t* avio_buf,
                                   int buf_size) {
  auto* receiver = reinterpret_cast<CustomCbOutFormat*>(userdata);
  auto* address = receiver->buffer.data();
  memcpy(address, avio_buf, buf_size);
  return buf_size;
}
RtpOutFormat::RtpOutFormat(Url& url, RtpSRSetting& s) : url(url), OutFormat(s) {
  avformat_network_init();
  auto dest = getSdpUrl(url);
  auto* fmt_output = av_guess_format("rtsp", dest.c_str(), nullptr);
  ctx->oformat = fmt_output;
}
CodecBase::CodecBase(RtpSRSetting& s, Codec c, bool isencoder) : codec(c) {
  auto* avcodec =
      (isencoder) ? avcodec_find_encoder_by_name(getCodecName(codec).c_str())
                  : avcodec_find_decoder_by_name(getCodecName(codec).c_str());
  ctx = avcodec_alloc_context3(avcodec);
  ctx->bit_rate = bitrate;
  ctx->sample_rate = s.samplerate;
  ctx->frame_size = s.framesize;
  ctx->channels = s.channels;
  ctx->channel_layout = av_get_default_channel_layout(s.channels);
  ctx->sample_fmt = AV_SAMPLE_FMT_S16;
  avcodec_open2(ctx, avcodec, nullptr);
}
bool CodecBase::checkIsErrAgain(int error_code) {
  if (error_code == AVERROR(EAGAIN)) {
    return true;  // return isempty
  }
  checkAvError(error_code);
  return false;
}

Decoder::Decoder(RtpSRSetting& s, Codec c) :CodecBase(s,c,false){}
Encoder::Encoder(RtpSRSetting& s, Codec c) :CodecBase(s,c,true){}
bool Decoder::sendPacket(AVPacket* packet) {
  return checkIsErrAgain(avcodec_send_packet(ctx, packet));
}
bool Decoder::receiveFrame(AVFrame* frame) {
  return checkIsErrAgain(avcodec_receive_frame(ctx, frame));
}

bool Encoder::sendFrame(AVFrame* frame) {
  return checkIsErrAgain(avcodec_send_frame(ctx, frame));
}
bool Encoder::receivePacket(AVPacket* packet) {
  return checkIsErrAgain(avcodec_receive_packet(ctx, packet));
}

RtpSRBase::RtpSRBase(RtpSRSetting& s,std::ostream& logger): setting(s),logger(logger) {
    frame = av_frame_alloc();
    frame->nb_samples = setting.framesize;
    packet = av_packet_alloc();
    av_new_packet(packet, getBufSize(setting));
  }
RtpSRBase::~RtpSRBase(){
  av_frame_free(&frame);
  av_packet_free(&packet);
}
}  // namespace rtpsr