//  A simple ffmpeg wrapper to send/receive audio stream with rtp protocol
//  written in C++.
// Copyright 2020 Tomoya Matsuura All rights Reserved.
// This source code is released under LGPL lisence.
#include <cmath>

#include "c74_min_unittest.h"  // required unit test header
#include "rtpreceiver.hpp"
#include "rtpsender.hpp"

bool loopback_test(int channels) {
  bool result = false;
  RtpSender sender(128, 48000, channels, "127.0.0.1", 30000);
  RtpReceiver receiver(128, 48000, channels, "127.0.0.1", 30000);

  sender.init();
  receiver.init();
  sender.writeHeader();
  auto *writebuf = reinterpret_cast<rtpsr::sample_t *>(sender.getBufferPtr());
  for (int i = 0; i < 128 * channels; i++) {
    writebuf[i] = i;
  }
  sender.sendData();  // send one frame
  receiver.receiveData();
  auto *readbuf = reinterpret_cast<rtpsr::sample_t *>(receiver.getBufferPtr());
  bool cmp_flag = true;
  for (int i = 0; i < 128 * channels; i++) {
    auto v = readbuf[i];
    cmp_flag &= (v == i);
  }
  result = cmp_flag;

  return result;
}

TEST_CASE("RTP sender instance") {
  // testSineWave sinewave;
  RtpSender sender(128, 48000, 1, "127.0.0.1", 30000);
  try {
    sender.init();
    // std::exit(EXIT_SUCCESS);
  } catch (std::exception &err) {
    std::cerr << err.what() << "\n";
    std::exit(EXIT_FAILURE);
  }
}
TEST_CASE("RTP receiver instance") {
  // testSineWave sinewave;
  RtpReceiver receiver(128, 48000, 1, "127.0.0.1", 30000);
  try {
    receiver.init();
    // std::exit(EXIT_SUCCESS);
  } catch (std::exception &err) {
    std::cerr << err.what() << "\n";
    std::exit(EXIT_FAILURE);
  }
}
TEST_CASE("Mono loopback test") { REQUIRE(loopback_test(1) == true); }
TEST_CASE("5ch loopback test") { REQUIRE(loopback_test(5) == true); }
TEST_CASE("Multi-packet per frame test") {
  REQUIRE(loopback_test(6) == true);
  REQUIRE(loopback_test(8) == true);
  REQUIRE(loopback_test(16) == true);
}