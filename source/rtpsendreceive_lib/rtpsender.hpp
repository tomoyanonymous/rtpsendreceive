#pragma once
#include "rtpsrbase.hpp"

namespace rtpsr {
	struct RtpSender : public RtpSRBase {
		explicit RtpSender(std::unique_ptr<RtpSRSetting> s, Url const& url, Codec codec,
			std::chrono::milliseconds init_retry_rate = std::chrono::milliseconds(500), std::ostream& logger = std::cerr);
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
		duration_type             pollingrate = duration_type(static_cast<double>(setting->framesize) * 0.5 * 48000 / setting->samplerate);
		const std::chrono::milliseconds init_retry_rate;
	};

}    // namespace rtpsr