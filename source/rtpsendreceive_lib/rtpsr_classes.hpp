#pragma once
#include <future>
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
  virtual void startInput(){};
};

struct CustomCbInFormat : public InFormat, public CustomCbFormat {
  explicit CustomCbInFormat(RtpSRSetting& s) : InFormat(s) {
    buffer.resize(setting.framesize);
    auto bufsize = getBufSize(s);
    auto* aviobuf = static_cast<uint8_t*>(av_malloc(bufsize));
    auto avioctx = avio_alloc_context(aviobuf, bufsize, 0, this, readPacket,
                                      nullptr, nullptr);
    ctx->flags |= AVFMT_FLAG_CUSTOM_IO;
    ctx->pb = avioctx;
  }
  ~CustomCbInFormat() {}

  static int readPacket(void* userdata, uint8_t* avio_buf, int buf_size);

  void setBuffer(double sample, int pos, int channel) {
    buffer.at(pos * setting.channels + channel) =
        static_cast<rtpsr::sample_t>(sample * INT16_MAX);
  }
};

struct SdpOpaque {
  using Vector = std::vector<uint8_t>;
  Vector data;
  Vector::iterator pos;
};

struct Url {
  std::string address = "127.0.0.1";
  int port = 30000;
};
inline std::string getSdpUrl(std::string const& address, int port) {
  return "udp://" + address + ":" + std::to_string(port) + "/live.sdp";
}

inline std::string getSdpUrl(Url& url) {
  return getSdpUrl(url.address, url.port);
}
struct RtpInFormat : public InFormat {
  RtpInFormat(Url& url, RtpSRSetting& s) : url(url), InFormat(s) {
    avformat_network_init();
  }
  ~RtpInFormat() {}
  Url url;
  //   std::string dummysdp_content;
  //   std::string& makeDummySdp();
  //   static int readDummySdp(void* userdata, uint8_t* avio_buf, int buf_size);
};

struct OutFormat : public IOFormat {
  explicit OutFormat(RtpSRSetting& s) : IOFormat(s){};
  virtual void startOutput(){};
};

struct CustomCbOutFormat : public OutFormat, public CustomCbFormat {
  explicit CustomCbOutFormat(RtpSRSetting& s) : OutFormat(s) {
    buffer.resize(setting.framesize);

    auto bufsize = getBufSize(s);
    auto* aviobuf = static_cast<uint8_t*>(av_malloc(bufsize));
    auto* avioctx = avio_alloc_context(aviobuf, bufsize, 1, this, nullptr,
                                       writePacket, nullptr);
    ctx->flags |= AVFMT_FLAG_CUSTOM_IO;
    ctx->pb = avioctx;
  }
  ~CustomCbOutFormat() {}
  static int writePacket(void* userdata, uint8_t* avio_buf, int buf_size);
  template <typename T>
  T readBuffer(int pos, int channel) {
    return static_cast<T>(buffer.at(pos * setting.channels + channel));
  };
  template <>
  rtpsr::sample_t readBuffer(int pos, int channel) {
    return buffer.at(pos * setting.channels + channel);
  };
  template <>
  double readBuffer(int pos, int channel) {
    return static_cast<double>(buffer.at(pos * setting.channels + channel)) /
           INT16_MAX;
  }
};
struct RtpOutFormat : public OutFormat {
  RtpOutFormat(Url& url, RtpSRSetting& s) : url(url), OutFormat(s) {
    avformat_network_init();
    auto dest = getSdpUrl(url);
    auto* fmt_output = av_guess_format("rtsp", dest.c_str(), nullptr);
    ctx->oformat = fmt_output;
  }
  Url url;
};

struct CodecBase {
  explicit CodecBase(Codec c = Codec::PCM_s16BE) : codec(c) {}
  AVCodecContext* ctx;
  rtpsr::Codec codec;
  int64_t bitrate = 192000;  // bitrate for lossy compression
  static bool checkIsErrAgain(int error_code);
};
struct Decoder : public CodecBase {
  explicit Decoder(RtpSRSetting& s, Codec c) : CodecBase(c) {
    auto* avcodec = avcodec_find_decoder_by_name(getCodecName(codec).c_str());
    ctx = avcodec_alloc_context3(avcodec);
    ctx->bit_rate = bitrate;
    ctx->sample_rate = s.samplerate;
    ctx->frame_size = s.framesize;
    ctx->channels = s.channels;
    ctx->channel_layout = av_get_default_channel_layout(s.channels);
    ctx->sample_fmt = AV_SAMPLE_FMT_S16;
    avcodec_open2(ctx, avcodec, nullptr);
  }
  ~Decoder() { avcodec_free_context(&ctx); }
  bool sendPacket(AVPacket* packet);
  bool receiveFrame(AVFrame* frame);
};
struct Encoder : public CodecBase {
  explicit Encoder(RtpSRSetting& s, Codec c) : CodecBase(c) {
    auto* avcodec = avcodec_find_encoder_by_name(getCodecName(codec).c_str());
    ctx = avcodec_alloc_context3(avcodec);
    ctx->bit_rate = bitrate;
    ctx->sample_rate = s.samplerate;
    ctx->frame_size = s.framesize;
    ctx->channels = s.channels;
    ctx->channel_layout = av_get_default_channel_layout(s.channels);
    ctx->sample_fmt = AV_SAMPLE_FMT_S16;
    avcodec_open2(ctx, avcodec, nullptr);
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

struct ConnectionCb{
    void* opaque=nullptr;
    std::function<void(void*)> cb=[](void* v){};
};
struct RtpSender : public RtpSRBase {
  explicit RtpSender(RtpSRSetting& s, Url& url, Codec codec,ConnectionCb cb=ConnectionCb())
      : RtpSRBase(s), encoder(s, codec),connectioncb(std::move(cb)) {
    input = std::static_pointer_cast<InFormat>(
        std::make_shared<CustomCbInFormat>(setting));
    output = std::static_pointer_cast<OutFormat>(
        std::make_shared<RtpOutFormat>(url, setting));
    auto* instream = avformat_new_stream(input->ctx, encoder.ctx->codec);
    auto* outstream = avformat_new_stream(output->ctx, encoder.ctx->codec);
    checkAvError(
        avcodec_parameters_from_context(instream->codecpar, encoder.ctx));
    checkAvError(
        avcodec_parameters_from_context(outstream->codecpar, encoder.ctx));
    instream->start_time = 0;
    outstream->start_time = 0;
    setCtxParams(&params);
    url_tmp = getSdpUrl(url);
    char* urlctx = (char*)av_malloc(url_tmp.size() + sizeof(char));
    av_strlcpy(urlctx, url_tmp.c_str(), url_tmp.size() + sizeof(char));
    output->ctx->url = urlctx;
    checkAvError(avformat_init_output(output->ctx, &params));
    checkAvError(avio_open(&output->ctx->pb, urlctx, AVIO_FLAG_WRITE));
    waitvar = std::async(std::launch::async,[&](){
    while (true) {
      int res = avformat_write_header(output->ctx, nullptr);
      if (res >= 0) {
        break;
      }
      if(res==-61 || res==-22){
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
      }else{
          checkAvError(res);
          break;
      }
    }
    connectioncb.cb(connectioncb.opaque);
    return 1;
    });
  }
  ~RtpSender() {
    if (output->ctx->pb != nullptr) {
      avio_close(output->ctx->pb);
    }
  }

  Encoder encoder;
  AVDictionary* params = nullptr;
  int64_t timecount = 0;
  std::string url_tmp;
  ConnectionCb connectioncb;
  std::future<int> waitvar;
  void sendData();
  static void setCtxParams(AVDictionary** dict);
  auto& getInput() {
    return *std::dynamic_pointer_cast<CustomCbInFormat>(input);
  }
  auto& getBuffer() { return getInput().buffer; }
  auto* getBufPointer() {
    return reinterpret_cast<uint8_t*>(getBuffer().data());
  }
};

struct RtpReceiver : public RtpSRBase {
  RtpReceiver(RtpSRSetting& s, Url& url, Codec codec,ConnectionCb cb=ConnectionCb())
      : RtpSRBase(s), decoder(s, codec),connectioncb(std::move(cb)) {
    input = std::static_pointer_cast<InFormat>(
        std::make_shared<RtpInFormat>(url, setting));
    output = std::static_pointer_cast<OutFormat>(
        std::make_shared<CustomCbOutFormat>(setting));

    ifmt = av_find_input_format("rtsp");
    url_tmp = getSdpUrl(url);

    setCtxParams(&params);
    input->ctx->max_delay = 1000;
    wait_connection = std::async(std::launch::async, [&]() -> int {
      int res = avformat_open_input(&input->ctx, url_tmp.c_str(), ifmt,
                                    &params);  // this start blocking...
      auto* instream = avformat_new_stream(input->ctx, decoder.ctx->codec);
      auto* outstream = avformat_new_stream(output->ctx, decoder.ctx->codec);
      checkAvError(
          avcodec_parameters_from_context(instream->codecpar, decoder.ctx));
      checkAvError(
          avcodec_parameters_from_context(outstream->codecpar, decoder.ctx));
      instream->start_time = 0;
      outstream->start_time = 0;
    connectioncb.cb(connectioncb.opaque);
      return res;
    });
  }
  ~RtpReceiver() {
    //   av_dict_free(&params);
  }
  Decoder decoder;
  AVDictionary* params = nullptr;
  AVInputFormat* ifmt;
  ConnectionCb connectioncb;

  std::future<int> wait_connection;
  std::string url_tmp;
  void receiveData();
  void setCtxParams(AVDictionary** dict);
  auto& getOutput() {
    return *std::dynamic_pointer_cast<CustomCbOutFormat>(output);
  }
  auto& getBuffer() { return getOutput().buffer; }
  auto* getBufPointer() {
    return reinterpret_cast<uint8_t*>(getBuffer().data());
  }
};

}  // namespace rtpsr