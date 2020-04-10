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
  int64_t pos=0;
  static constexpr int bufsize = 512;
  size_t buf_size_in_i8;
  static constexpr double freq = 440.0;
  std::array<double, bufsize> buffer;
  std::array<int16_t, bufsize> ibuffer;

  testSineWave() { buf_size_in_i8 = sizeof(int64_t) * bufsize; }
  void process(uint8_t *buf) {
    for (auto& elem : buffer) {
      phase = std::fmodl(phase + freq * M_PI * 2 / 48000, M_PI * 2);
      elem = sin(phase) * INT16_MAX;
    }
    std::copy(buffer.begin(), buffer.end(), ibuffer.begin());
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
    sinewave->pos+=pos;

  return sinewave->pos;
}

SCENARIO("instance is correctly created") {
  testSineWave sinewave;
  RtpSender sender(&testReadPacket,&testSeek, (void *)(&sinewave));
  sender.init();
  GIVEN("An instance of our object") {
    // check that default attr values are correct
    // REQUIRE((my_object.greeting == symbol("hello world")));
    // now proceed to testing various sequences of events
    WHEN("a 'bang' is received") {
      // my_object.bang();
      sender.start();
      THEN("our greeting is produced at the outlet") {
        // REQUIRE((output.size() == 1));
      }
    }
  }
}
