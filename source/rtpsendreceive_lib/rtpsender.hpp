#pragma once
#include "rtpsrbase.hpp"

namespace rtpsr {
	struct RtpSender : public RtpSRBase {
		explicit RtpSender(RtpSRSetting& s, Url& url, Codec codec, std::ostream& logger = std::cerr);
		~RtpSender();

		void launchLoop() override;
		// communication entrypoint between max
		bool writeToInput(std::vector<sample_t> const& input);
		bool writeToInput(std::vector<double> const& input);

	private:
		int64_t                   timecount = 0;
		std::vector<sample_t>     framebuf;
		AVOptionBase::container_t makeCtxParams();
		bool                      fillFrame();
		void                      sendData();
		std::vector<sample_t>     dtosbuffer;
		duration_type             pollingrate = duration_type(static_cast<double>(setting.framesize) * 0.5 * 48000 / setting.samplerate);
	};

}    // namespace rtpsr