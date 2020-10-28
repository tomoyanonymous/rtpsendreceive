#pragma once
#include "rtpsrbase.hpp"

namespace rtpsr {
	struct RtpReceiver : public RtpSRBase {
		explicit RtpReceiver(RtpSRSetting& s, Url& url, Codec codec, std::ostream& logger = std::cerr);
		~RtpReceiver();

		void launchLoop() override;
		bool readFromOutput(std::vector<sample_t>& dest);
		bool readFromOutput(std::vector<double>& dest);

	private:
		std::vector<int16_t>      dtosbuffer;
		std::vector<int16_t>      tmpbuf;
		AVOptionBase::container_t makeCtxParams();
		bool                      pushToOutput();
		bool                      receiveData();
	};
}    // namespace rtpsr
