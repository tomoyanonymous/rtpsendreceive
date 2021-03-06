#pragma once
#include "rtpsrbase.hpp"

namespace rtpsr {
	struct RtpSender : public RtpSRBase {
		explicit RtpSender(std::unique_ptr<RtpSRSetting> s, Url const& url, Codec codec,
			std::chrono::milliseconds init_retry_rate = std::chrono::milliseconds(init_retry_rate_int), double ringbuf_multiple = 4.2,
			std::ostream& logger = std::cerr);
		// launch with raw rtp version;
		explicit RtpSender(std::unique_ptr<RtpOutOption> option, Codec codec,
			std::chrono::milliseconds init_retry_rate = std::chrono::milliseconds(init_retry_rate_int), double ringbuf_multiple = 4.2,
			std::ostream& logger = std::cerr);
		// launch with rtsp version
		explicit RtpSender(std::unique_ptr<RtspOutOption> option, Codec codec,
			std::chrono::milliseconds init_retry_rate = std::chrono::milliseconds(init_retry_rate_int), double ringbuf_multiple = 4.2,
			std::ostream& logger = std::cerr);
		~RtpSender();

		void launchLoop() override;
		// communication entrypoint between max
		bool writeToInput(std::vector<sample_t> const& input);
		bool writeToInput(std::vector<double> const& input);
		bool isConnected() override;

	private:
		int64_t               timecount = 0;
		std::vector<sample_t> framebuf;

		// constructor must call init()
		void                            init();
		bool                            fillFrame();
		void                            sendData();
		std::vector<sample_t>           dtosbuffer;
		duration_type                   pollingrate;
		const std::chrono::milliseconds init_retry_rate;
		static constexpr int            init_retry_rate_int = 500;
	};

}    // namespace rtpsr