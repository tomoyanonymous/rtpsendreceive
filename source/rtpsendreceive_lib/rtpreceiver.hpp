#pragma once
#include <optional>

#include "rtpsrbase.hpp"
#include "lockfree_ringbuffer.hpp"

namespace rtpsr {
	struct RtpReceiver : public RtpSRBase {
		explicit RtpReceiver(RtpSRSetting& s, Url& url, Codec codec, std::ostream& logger = std::cerr);
		~RtpReceiver();

		AVOptionBase::container_t makeCtxParams();
		std::future<bool>&        launchLoop() override;
		bool                      pushToOutput();
		bool                      readFromOutput(std::vector<sample_t>& dest);
		bool                      readFromOutput(std::vector<double>& dest);

	private:
		std::vector<int16_t> dtosbuffer;
		std::vector<int16_t> tmpbuf;
		bool                 receiveData();
	};
}    // namespace rtpsr
