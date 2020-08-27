//  A simple ffmpeg wrapper to send/receive audio stream with rtp protocol
//  written in C++.
// Copyright 2020 Tomoya Matsuura All rights Reserved.
// This source code is released under LGPL lisence.
#include <cmath>

#include "c74_min_unittest.h"  // required unit test header
#include "rtpsr_classes.hpp"

TEST_CASE("RTSP instance") {
  // testSineWave sinewave;
  try {
    //   av_log_set_level(AV_LOG_TRACE);
  rtpsr::RtpSRSetting setting = {48000,1,128};
  rtpsr::Url url= {"127.0.0.1",30000};

  rtpsr::RtpReceiver receiver(setting,url,rtpsr::Codec::PCM_s16BE);

  rtpsr::RtpSender sender(setting,url,rtpsr::Codec::PCM_s16BE);
  
    // std::exit(EXIT_SUCCESS);
  } catch (std::exception &err) {
    std::cerr << err.what() << "\n";
    std::exit(EXIT_FAILURE);
  }
}
