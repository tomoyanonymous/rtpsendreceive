#pragma once
#include <regex>
#include <chrono>
#include "rtpsrbase.hpp"

struct SdpOpaque {
  using Vector = std::vector<uint8_t>;
  Vector data;
  Vector::iterator pos;
};
struct TimeoutOpaque{
  double timeout;
  std::chrono::time_point<std::chrono::system_clock> clock;
};

class RtpReceiver : public RtpSRBase {
 public:
  explicit RtpReceiver(int framesize = 128, int samplerate = 48000,
                       int channels = 1, std::string address = "127.0.0.1",
                       int port = 30000,rtpsr::Codec codec=rtpsr::Codec::PCM_s16BE,
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
  void setTimeout(double to);
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
  char* ts_string;
  uint8_t* avio_buffer;
  std::chrono::time_point<std::chrono::system_clock> clock;
  TimeoutOpaque* TO;
  std::vector<int16_t> avio_vector;
  std::string sdp_content;
  void makeDummySdp();
  int sampleRate(){return samplerate;}
  int frameSize(){return framesize;}
  auto getClock(){return clock;}
  static int writePacketSelf(void* userdata, uint8_t* avio_buf, int buf_size);
  static int readDummySdp(void* userdata, uint8_t* avio_buf, int buf_size);
  static int checkTimeout(void* opaque);
  template <typename T>
  void copyBufferFromframe(AVBufferRef* frameref, int offset,int size){
    auto dataptr = reinterpret_cast<T*>(frameref->data);
    for(int i=offset;i<size;i++){
    buffer.at(i) = (double)dataptr[i];
    }
  }
};