//  A simple ffmpeg wrapper to send/receive audio stream with rtp protocol
//  written in C++.
// Copyright 2020 Tomoya Matsuura All rights Reserved.
// This source code is released under LGPL 3.0 lisence.
#include "rtpsender.hpp"
#include "rtpreceiver.hpp"

#include <cmath>
#define CATCH_CONFIG_NO_CPP17_UNCAUGHT_EXCEPTIONS
#include "c74_min_unittest.h"    // required unit test header

TEST_CASE("RTSP loopback") {
	//   av_log_set_level(AV_LOG_TRACE);
	auto setting_r = std::make_unique<rtpsr::RtpSRSetting>(rtpsr::RtpSRSetting{48000, 1, 128});
	auto setting_s = std::make_unique<rtpsr::RtpSRSetting>(rtpsr::RtpSRSetting{48000, 1, 128});

	rtpsr::Url          url     = {"127.0.0.1", 30000};

	auto receiver = std::make_unique<rtpsr::RtpReceiver>(std::move(setting_r), url, rtpsr::Codec::PCM_s16BE);
	auto sender   = std::make_unique<rtpsr::RtpSender>(std::move(setting_s), url, rtpsr::Codec::PCM_s16BE);

	std::vector<double> ref;
	std::vector<short>  input;
	for (int i = 0; i < 128; i++) {
		int16_t s  = std::rand() / RAND_MAX;
		auto    ds = ((double)s) * 2 - 1.0;
		ref.emplace_back(ds);
		input.emplace_back(s);
	}
	sender->writeToInput(input);
	sender->launchLoop();
	receiver->launchLoop();
	std::vector<int16_t> answer_s(input.size(), 0);
	std::vector<double>  answer;
	while (true) {
		auto res = receiver->readFromOutput(answer_s);
		if (res)
			break;
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
	sender.reset();
	receiver.reset();
	for (int i = 0; i < input.size(); i++) {
		answer.emplace_back(rtpsr::convertSampleToDouble(answer_s[i]));
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
