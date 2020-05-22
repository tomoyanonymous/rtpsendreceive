#pragma once
#include <regex>

#include "rtpsrbase.hpp"
struct SdpOpaque {
  using Vector = std::vector<uint8_t>;
  Vector data;
  Vector::iterator pos;
};

class RtpReceiver : public RtpSRBase {
 public:
  explicit RtpReceiver(int framesize = 128, int samplerate = 48000,
                       int channels = 1, std::string address = "127.0.0.1",
                       int port = 30000,
                       rtpsr::readfn_type callback_write = writePacketSelf,
                       rtpsr::seekfn_type callback_seek = nullptr,
                       void* userdata = nullptr);
  ~RtpReceiver();
  void initFormatCtx() override;
  void initInputFormat();
  void initOutputFormat();
  void receiveData();
  // void writeBuffer(double sample, int pos, int channel_idx);
  double readBuffer(int pos, int channel_idx);
  auto* getBufferPtr() { return reinterpret_cast<uint8_t*>(buffer.data()); }
  void setSource(std::string& ad, int po);
  void play();
  void pause();
 private:
  rtpsr::readfn_type callback_write;
  rtpsr::seekfn_type callback_seek;
  std::string address;
  int port;
  void* userdata_address;
  AVIOContext* sdp_ioctx;
  uint8_t* sdpio_buffer;

  uint8_t* avio_buffer;
  std::string sdp_content;
  void makeDummySdp();
  static int writePacketSelf(void* userdata, uint8_t* avio_buf, int buf_size);
  static int readDummySdp(void* userdata, uint8_t* avio_buf, int buf_size);
};