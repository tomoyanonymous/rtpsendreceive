//  A simple ffmpeg wrapper to send/receive audio stream with rtp protocol
//  written in C++.
// Copyright 2020 Tomoya Matsuura All rights Reserved.
// This source code is released under LGPL lisence.
#include "rtpsendreceive_lib.hpp"

#include <cmath>

#include "c74_min_unittest.h"  // required unit test header

class testSineWave {
 public:
  double phase = 0;
  int64_t pos = 0;
  static constexpr int bufsize = 2048;
  size_t buf_size_in_i8;
  static constexpr double freq = 10.0;
  std::array<double, bufsize> buffer;
  std::array<int16_t, bufsize> ibuffer;

  testSineWave() { buf_size_in_i8 = sizeof(int16_t) * bufsize; }
  void process(uint8_t *buf) {
    int count = 0;
    for (auto &elem : buffer) {
      phase = std::fmodl(phase + freq * M_PI * 2 / 48000, M_PI * 2);
      ibuffer[count] = (int16_t) (sin(phase) * INT16_MAX);
      // std::cerr<<phase << " :  "<< ibuffer[count] << "\n";
      count++;
    }
    // std::copy(buffer.begin(), buffer.end(), ibuffer.begin());
    memcpy(buf, ibuffer.data(), buf_size_in_i8);
  }
};
int testReadPacket(void *opaque, uint8_t *buf, int buf_size) {
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

TEST_CASE("instance is correctly created") {
  testSineWave sinewave;
  RtpSender sender(&testReadPacket, &testSeek, (void *)(&sinewave));
  try {
    sender.init();
  } catch (std::exception &err) {
    std::cerr << err.what() << "\n";
    std::exit(EXIT_FAILURE);
  }
 
    // check that default attr values are correct
    // REQUIRE((my_object.greeting == symbol("hello world")));
    // now proceed to testing various sequences of events
      // my_object.bang();
      try {
        sender.start();
      } catch (std::exception &err) {
        std::cerr << err.what() << "\n";
        exit(9);
      }
        // REQUIRE((output.size() == 1));
  }
