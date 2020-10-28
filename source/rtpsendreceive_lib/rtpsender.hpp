#pragma once
#include "rtpsrbase.hpp"
#include "lockfree_ringbuffer.hpp"

namespace rtpsr {
	struct RtpSender : public RtpSRBase {
		explicit RtpSender(RtpSRSetting& s, Url& url, Codec codec, std::ostream& logger = std::cerr);
		~RtpSender() {
			std::cerr << "rtpsender destructor called" << std::endl;
			bool res1 = init_asyncloop.halt();
			bool res2 = asynclooper.halt();
			if (res1&&res2) {
				av_write_trailer(output->ctx);
				if (output->ctx->pb != nullptr) {
					avio_close(output->ctx->pb);
				}
				avformat_close_input(&input->ctx);
			}
		}
		int64_t timecount = 0;

		std::vector<sample_t>     framebuf;
		void       launchLoop() override;
		duration_type             pollingrate = duration_type(static_cast<double>(setting.framesize) * 0.5 * 48000 / setting.samplerate);
		AVOptionBase::container_t makeCtxParams();
		// communication entrypoint between max
		bool writeToInput(std::vector<sample_t> const& input);
		bool writeToInput(std::vector<double> const& input);

	private:
		bool                  fillFrame();
		void                  sendData();
		std::vector<sample_t> dtosbuffer;
	};

}    // namespace rtpsr