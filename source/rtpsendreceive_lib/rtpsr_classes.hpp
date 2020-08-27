#pragma once
#include <regex>

#include "rtpsendreceive_lib.hpp"

namespace rtpsr {

inline void checkAvError(int error_code) {
  if (error_code < 0) {
    char str[4096];
    av_make_error_string(str, 4096, error_code);
    throw std::runtime_error(str);
  }
}

struct RtpSRSetting {
  int samplerate;
  int channels;
  int framesize;
};

inline size_t getBufSize(RtpSRSetting& s) {
  // how sample format?
  return sizeof(double) * s.framesize * s.channels;
}

struct IOFormat {
  explicit IOFormat(RtpSRSetting& setting) : setting(setting) {
    ctx = avformat_alloc_context();
  }
  ~IOFormat() { avformat_free_context(ctx); }
  AVFormatContext* ctx;
  RtpSRSetting& setting;
};

struct CustomCbFormat {
  rtpsr::buffertype buffer;
};
struct InFormat : public IOFormat {
  explicit InFormat(RtpSRSetting& s) : IOFormat(s){};
  virtual void startInput() = 0;
};

struct CustomCbInFormat : public InFormat, public CustomCbFormat {
  explicit CustomCbInFormat(RtpSRSetting& s) : InFormat(s) {
    auto bufsize = getBufSize(s);
    auto* aviobuf = static_cast<uint8_t*>(av_malloc(bufsize));
    auto avioctx = avio_alloc_context(aviobuf, bufsize, 0, this, readPacket,
                                      nullptr, nullptr);
    ctx->flags |= AVFMT_FLAG_CUSTOM_IO;
    ctx->pb = avioctx;
    ctx->iformat->flags |= AVFMT_NOFILE;
  }
  ~CustomCbInFormat() {}

  static int readPacket(void* userdata, uint8_t* avio_buf, int buf_size);
  void setBuffer(rtpsr::sample_t sample, int pos, int channel);
};

struct SdpOpaque {
  using Vector = std::vector<uint8_t>;
  Vector data;
  Vector::iterator pos;
};

inline std::string getSdpUrl(std::string const& address,int port){
    return "rtsp://" + address + ":" + std::to_string(port) + "/live.sdp";
}

struct RtpInFormat : public InFormat {
  RtpInFormat(std::string& address, int port, RtpSRSetting& s) :host_address(address),port(port), InFormat(s) {
    auto bufsize = getBufSize(s);
    avformat_network_init();
    // auto* sdpio_buffer = static_cast<uint8_t*>(av_malloc(bufsize));
    // auto opaque = (SdpOpaque*)av_malloc(sizeof(SdpOpaque));
    // auto sdp = makeDummySdp().c_str();
    // opaque->data = SdpOpaque::Vector(sdp, sdp + strlen(sdp));
    // opaque->pos = opaque->data.begin();

    // auto* sdp_ioctx = avio_alloc_context(sdpio_buffer, bufsize, 0, opaque,
    //                                      readDummySdp, nullptr, nullptr);
    // ctx->pb = sdp_ioctx;
    auto infmt = av_find_input_format("rtsp");
    auto host_url = getSdpUrl(host_address, port);
    AVDictionary* params = nullptr;
    av_dict_set_int(&params, "reorder_queue_size", 50000, 0);  // 0.05sec
        av_dict_set(&params, "rtsp_flags", "listen", 0);
    av_dict_set(&params, "allowed_media_types", "audio", 0);
    ctx->max_delay = 1000;
    checkAvError(
        avformat_open_input(&ctx, host_address.c_str(), infmt, &params));
  }
  ~RtpInFormat() {}
  std::string host_address = "127.0.0.1";
  int port = 30000;
//   std::string dummysdp_content;
//   std::string& makeDummySdp();
//   static int readDummySdp(void* userdata, uint8_t* avio_buf, int buf_size);
};

struct OutFormat : public IOFormat {
  explicit OutFormat(RtpSRSetting& s) : IOFormat(s){};
  virtual void startOutput() = 0;
};

struct CustomCbOutFormat : public OutFormat, public CustomCbFormat {
  explicit CustomCbOutFormat(RtpSRSetting& s) : OutFormat(s) {
    auto bufsize = getBufSize(s);
    auto* aviobuf = static_cast<uint8_t*>(av_malloc(bufsize));
    auto* avioctx = avio_alloc_context(aviobuf, bufsize, 1, this, nullptr,
                                       writePacket, nullptr);
    ctx->flags |= AVFMT_FLAG_CUSTOM_IO;
    ctx->pb = avioctx;
    ctx->iformat->flags |= AVFMT_NOFILE;
  }
  ~CustomCbOutFormat() {}
  static int writePacket(void* userdata, uint8_t* avio_buf, int buf_size);
  rtpsr::sample_t getBuffer(int pos, int channel);
};
struct RtpOutFormat : public OutFormat {
  RtpOutFormat(std::string& address, int port, RtpSRSetting& s) :dest_address(address),port(port), OutFormat(s) {
    auto dest = getSdpUrl(dest_address, port);
    auto* fmt_output = av_guess_format("rtsp", dest.c_str(), nullptr);

    checkAvError(avio_open(&ctx->pb, dest.c_str(), AVIO_FLAG_WRITE));
    ctx->oformat = fmt_output;
    // std::char_traits<char>::copy(ctx->url, dest.c_str(), dest.size() + 1);
    AVDictionary* params = nullptr;
    av_dict_set_int(&params, "timeout", 2, 0);
    checkAvError(avformat_write_header(ctx, &params));
    //debug
    std::string sdp;
    sdp.reserve(4096);
    av_sdp_create(&ctx, 1, sdp.data(), sdp.size());
  }
  std::string dest_address = "127.0.0.1";
  int port = 30000;
};

struct CodecBase {
  explicit CodecBase(Codec c = Codec::PCM_s16BE) : codec(c) {}
  AVCodecContext* ctx;
  rtpsr::Codec codec;
  int64_t bitrate = 192000;  // bitrate for lossy compression
  static bool checkIsErrAgain(int error_code);
};
struct Decoder : public CodecBase {
  explicit Decoder(Codec c) : CodecBase(c) {
    auto* avcodec = avcodec_find_decoder_by_name(getCodecName(codec).c_str());
    ctx = avcodec_alloc_context3(avcodec);
    ctx->bit_rate = bitrate;
  }
  ~Decoder() { avcodec_free_context(&ctx); }
  bool sendPacket(AVPacket* packet);
  bool receiveFrame(AVFrame* frame);
};
struct Encoder : public CodecBase {
  explicit Encoder(Codec c) : CodecBase(c) {
    auto* avcodec = avcodec_find_encoder_by_name(getCodecName(codec).c_str());
    ctx = avcodec_alloc_context3(avcodec);
    ctx->bit_rate = bitrate;
  }
  ~Encoder() { avcodec_free_context(&ctx); }
  bool sendFrame(AVFrame* frame);
  bool receivePacket(AVPacket* packet);
};

struct RtpSRBase {
  explicit RtpSRBase(RtpSRSetting& s) : setting(s) {
    frame = av_frame_alloc();
    frame->nb_samples = setting.framesize;
    packet = av_packet_alloc();
    av_new_packet(packet, getBufSize(setting));
  }
  RtpSRSetting setting;
  std::shared_ptr<InFormat> input;
  std::shared_ptr<OutFormat> output;
  std::shared_ptr<CodecBase> codec;
  AVPacket* packet;
  AVFrame* frame;
};

struct RtpSender : public RtpSRBase {
  explicit RtpSender(RtpSRSetting& s, Codec codec)
      : RtpSRBase(s), encoder(codec) {
    input = std::static_pointer_cast<InFormat>(
        std::make_shared<CustomCbInFormat>(setting));
    output = std::static_pointer_cast<OutFormat>(
        std::make_shared<RtpOutFormat>(setting));

    auto* instream = avformat_new_stream(input->ctx, encoder.ctx->codec);
    auto* outstream = avformat_new_stream(output->ctx, encoder.ctx->codec);
    checkAvError(
        avcodec_parameters_from_context(instream->codecpar, encoder.ctx));
    checkAvError(
        avcodec_parameters_from_context(outstream->codecpar, encoder.ctx));
    instream->start_time = 0;
    outstream->start_time = 0;
  }

  Encoder encoder;
  int64_t timecount;
  void sendData();
  auto& getBuffer() {
    return std::dynamic_pointer_cast<CustomCbInFormat>(input)->buffer;
  }
  auto* getBufPointer() {
    return reinterpret_cast<uint8_t*>(getBuffer().data());
  }
};

struct RtpReciever : public RtpSRBase {
  RtpReciever(RtpSRSetting& s, Codec codec) : RtpSRBase(s), decoder(codec) {
    input = std::static_pointer_cast<InFormat>(
        std::make_shared<RtpInFormat>(setting));
    output = std::static_pointer_cast<OutFormat>(
        std::make_shared<CustomCbOutFormat>(setting));
    auto* instream = avformat_new_stream(input->ctx, decoder.ctx->codec);
    auto* outstream = avformat_new_stream(output->ctx, decoder.ctx->codec);
    checkAvError(
        avcodec_parameters_from_context(instream->codecpar, decoder.ctx));
    checkAvError(
        avcodec_parameters_from_context(outstream->codecpar, decoder.ctx));
    instream->start_time = 0;
    outstream->start_time = 0;
  }
  Decoder decoder;
  void receiveData();
  auto& getBuffer() {
    return std::dynamic_pointer_cast<CustomCbOutFormat>(output)->buffer;
  }
  auto* getBufPointer() {
    return reinterpret_cast<uint8_t*>(getBuffer().data());
  }
};

}  // namespace rtpsr