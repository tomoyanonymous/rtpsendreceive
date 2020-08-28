#pragma once
#include <regex>
#include <chrono>
#include "rtpsrbase.hpp"

namespace rtpsr{
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
}
