//  A simple ffmpeg wrapper to send/receive audio stream with rtp protocol
//  written in C++.
// Copyright 2020 Tomoya Matsuura All rights Reserved.
// This source code is released under LGPL lisence.
#include "rtpsender.hpp"
#include "rtpreceiver.hpp"

#include <cmath>
#define CATCH_CONFIG_NO_CPP17_UNCAUGHT_EXCEPTIONS
#include "c74_min_unittest.h"  // required unit test header

#include "lockfree_ringbuffer_test.hpp"
TEST_CASE("RTSP loopback") {
    //   av_log_set_level(AV_LOG_TRACE);
    rtpsr::RtpSRSetting setting = {48000, 1, 128};
    rtpsr::Url url = {"127.0.0.1", 30000};

    rtpsr::RtpReceiver receiver(setting, url, rtpsr::Codec::PCM_s16BE);
    rtpsr::RtpSender sender(setting, url, rtpsr::Codec::PCM_s16BE);

    auto rres = receiver.wait_connection.wait_for(std::chrono::seconds(3));
    auto sres = sender.wait_connection.wait_for(std::chrono::seconds(3));
    if(rres ==std::future_status::timeout||sres==std::future_status::timeout){
      throw std::runtime_error("timeout");
    }
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

}
// TEST_CASE("RTSP Opus loopback") {
//   try {
//     //   av_log_set_level(AV_LOG_TRACE);
//     rtpsr::RtpSRSetting setting = {48000, 1, 256};
//     rtpsr::Url url = {"127.0.0.1", 30000};

//     rtpsr::RtpReceiver receiver(setting, url, rtpsr::Codec::OPUS);
//     rtpsr::RtpSender sender(setting, url, rtpsr::Codec::OPUS);
//     std::vector<double> ref;
//     for (int i = 0; i < 128; i++) {
//       auto r = ((double)std::rand() / RAND_MAX) * 2 - 1.0;
//       ref.emplace_back(r);
//       sender.getInput().setBuffer(r, i, 0);
//     }
//     sender.sendData();
//     receiver.receiveData();
//     std::vector<double> answer;

//     for (int i = 0; i < 128; i++) {
//       answer.emplace_back(receiver.getOutput().readBuffer<double>(i, 0));
//     }

//     REQUIRE(REQUIRE_VECTOR_APPROX(answer, ref) == true);

//     // std::exit(EXIT_SUCCESS);
//   } catch (std::exception &err) {
//     std::cerr << err.what() << "\n";
//     std::exit(EXIT_FAILURE);
//   }
// }

