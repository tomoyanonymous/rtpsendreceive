#pragma once
#include "rtpsrbase.hpp"

namespace rtpsr {
	struct RtpReceiver : public RtpSRBase {
		explicit RtpReceiver(std::unique_ptr<RtpSRSetting> s, Url const& url, Codec codec, std::ostream& logger = std::cerr);
		explicit RtpReceiver(std::unique_ptr<RtpInOption> s,  Codec codec, std::ostream& logger = std::cerr);
		explicit RtpReceiver(std::unique_ptr<RtspInOption> s,  Codec codec, std::ostream& logger = std::cerr);

		~RtpReceiver();

		void launchLoop() override;
		bool readFromOutput(std::vector<sample_t>& dest);
		bool readFromOutput(std::vector<double>& dest);
		static int timeoutCallback(void * opaque);
		std::chrono::high_resolution_clock::time_point time_cache;
		bool isConnected() override;

	private:
		std::vector<int16_t>      dtosbuffer;
		std::vector<int16_t>      tmpbuf;
		int64_t timecount;
		void init();
		bool                      pushToOutput();
		bool                      receiveData();
	};
}    // namespace rtpsr
