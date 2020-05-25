//  A simple ffmpeg wrapper to send/receive audio stream with rtp protocol
//  written in C++.
// Copyright 2020 Tomoya Matsuura All rights Reserved.
// This source code is released under LGPL lisence.
#include "rtpsender.hpp"
#include "rtpreceiver.hpp"

#include <cmath>

#include "c74_min_unittest.h"  // required unit test header

class testSineWave {
 public:
  double phase = 0;
  int64_t pos = 0;
  int16_t testcount=365;
  static constexpr int bufsize = 2048;
  size_t buf_size_in_i8;
  static constexpr double freq = 4400.0;
  std::array<double, bufsize> buffer;
  std::array<int16_t, bufsize> ibuffer;

  testSineWave() { buf_size_in_i8 = sizeof(int16_t) * bufsize; }
  void process(uint8_t *buf) {
    int count = 0;
    for (auto &elem : buffer) {
      phase = std::fmodl(phase + freq * M_PI * 2 / 48000, M_PI * 2);
      ibuffer[count] = (int16_t) (0.3* sin(phase) * (double)INT16_MAX);
      // std::cerr<<phase << " :  "<< ibuffer[count] << "\n";
      // ibuffer[count] = (testcount % INT16_MAX);
      // testcount++;
      count++;
    }
    // std::copy(buffer.begin(), buffer.end(), ibuffer.begin());
    // av_samples_fill_arrays((uint8_t **)(ibuffer.data())), int *linesize, const uint8_t *buf, int nb_channels, int nb_samples, enum AVSampleFormat sample_fmt, int align)
    memcpy(buf, ibuffer.data(), buf_size_in_i8);
  }
};
int testReadPacket(void *opaque, unsigned char *buf, int buf_size) {
  auto *sinewave = reinterpret_cast<testSineWave *>(opaque);
  sinewave->process(buf);
  return buf_size;
}
int64_t testSeek(void *ptr, int64_t pos, int whence) {
  auto *sinewave = reinterpret_cast<testSineWave *>(ptr);

  if (whence == AVSEEK_SIZE) {
    return -1;
  }
  sinewave->pos += pos;

  return sinewave->pos;
}

TEST_CASE("RTP sender instance") {
  // testSineWave sinewave;
  RtpSender sender(128,48000, 1,"127.0.0.1",30000);
  try {
    sender.init();
    std::exit(EXIT_SUCCESS);
  } catch (std::exception &err) {
    std::cerr << err.what() << "\n";
    std::exit(EXIT_FAILURE);
  }

  }
TEST_CASE("RTP receiver instance") {
  // testSineWave sinewave;
  RtpReceiver receiver(128,48000, 1,"127.0.0.1",30000);
  try {
    receiver.init();
    std::exit(EXIT_SUCCESS);
  } catch (std::exception &err) {
    std::cerr << err.what() << "\n";
    std::exit(EXIT_FAILURE);
  }

  }
