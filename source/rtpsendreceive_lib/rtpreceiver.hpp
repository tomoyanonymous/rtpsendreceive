#pragma once
#include <chrono>
#include <regex>

#include "rtpsrbase.hpp"

namespace rtpsr {
struct RtpReceiver : public RtpSRBase {
  explicit RtpReceiver(RtpSRSetting& s, Url& url, Codec codec,
                       std::ostream& logger=std::cerr);
  ~RtpReceiver();
  Decoder decoder;
  AVDictionary* params = nullptr;
  AVInputFormat* ifmt;
  std::future<int> wait_connection;
  std::string url_tmp;
  void receiveData();
  static void setCtxParams(AVDictionary** dict);
  auto& getOutput() {
    return *std::dynamic_pointer_cast<CustomCbOutFormat>(output);
  }

  auto& getBuffer() { return getOutput().buffer; }
  auto* getBufPointer() {
    return reinterpret_cast<uint8_t*>(getBuffer().data());
  }
};
}  // namespace rtpsr
