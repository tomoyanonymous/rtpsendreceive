#pragma once
#include "rtpsrbase.hpp"

namespace rtpsr{
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

}