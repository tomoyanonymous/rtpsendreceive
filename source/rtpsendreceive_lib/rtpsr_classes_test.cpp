//  A simple ffmpeg wrapper to send/receive audio stream with rtp protocol
//  written in C++.
// Copyright 2020 Tomoya Matsuura All rights Reserved.
// This source code is released under LGPL lisence.
#include "rtpsr_classes.hpp"

#include <cmath>

#include "c74_min_unittest.h"  // required unit test header


TEST_CASE("RTSP loopback") {
  try {
    //   av_log_set_level(AV_LOG_TRACE);
    rtpsr::RtpSRSetting setting = {48000, 1, 128};
    rtpsr::Url url = {"127.0.0.1", 30000};

    rtpsr::RtpReceiver receiver(setting, url, rtpsr::Codec::PCM_s16BE);
    rtpsr::RtpSender sender(setting, url, rtpsr::Codec::PCM_s16BE);
    std::vector<double> ref;
    for (int i = 0; i < 128; i++) {
      auto r = ((double)std::rand() / RAND_MAX) * 2 - 1.0;
      ref.emplace_back(r);
      sender.getInput().setBuffer(r, i, 0);
    }
    sender.sendData();
    receiver.receiveData();
    std::vector<double> answer;

    for (int i = 0; i < 128; i++) {
      answer.emplace_back(receiver.getOutput().readBuffer<double>(i, 0));
    }

    REQUIRE(REQUIRE_VECTOR_APPROX(answer, ref) == true);

    // std::exit(EXIT_SUCCESS);
  } catch (std::exception &err) {
    std::cerr << err.what() << "\n";
    std::exit(EXIT_FAILURE);
  }
}
