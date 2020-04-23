#pragma once
#include "rtpsrbase.hpp"

class RtpReceiver : public RtpSRBase {
 public:
  explicit RtpReceiver(int framesize = 128, int samplerate = 48000,
                     int channels = 1, std::string address = "127.0.0.1",
                     int port = 30000,
                     rtpsr::readfn_type callback_read = readPacketSelf,
                     rtpsr::seekfn_type callback_seek = nullptr,
                     void* userdata = nullptr);
  ~RtpReceiver();
  void initFormatCtx() override;
  void sendData();
  void start();
  void writeBuffer(double sample, int pos, int channel_idx);
  auto* getBufferPtr() { return reinterpret_cast<uint8_t*>(buffer.data()); }
  void setDestination(std::string& ad, int po);

 private:
  rtpsr::readfn_type callback_read;
  rtpsr::seekfn_type callback_seek;
  std::string address;
  int port;
  void* userdata_address;
  AVIOContext* rtp_ioctx;
  uint8_t* avio_buffer;
  static int readPacketSelf(void* userdata, uint8_t* avio_buf, int buf_size);
};