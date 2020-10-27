#pragma once
#include "rtpsrbase.hpp"
#include "lockfree_ringbuffer.hpp"

namespace rtpsr {
	struct RtpSender : public RtpSRBase {
		explicit RtpSender(RtpSRSetting& s, Url& url, Codec codec, std::ostream& logger = std::cerr);
		~RtpSender() {
			// loopstate.future.wait();
			if (asynclooper.halt()) {
				av_write_trailer(output->ctx);
				if (output->ctx->pb != nullptr) {
					avio_close(output->ctx->pb);
				}
				av_dict_free(&params);
				avformat_close_input(&input->ctx);
			}
		}
		AVDictionary* params    = nullptr;
		int64_t       timecount = 0;
		std::string   url_tmp;
		// communication entrypoint between max

		LockFreeRingbuf<sample_t> input_buf;
		std::vector<sample_t>     framebuf;
		std::future<bool>&        launchLoop() override;
		duration_type             pollingrate = duration_type(static_cast<double>(setting.framesize) * 0.5 * 48000 / setting.samplerate);
		static void               setCtxParams(AVDictionary** dict);
		auto&                     getInput() {
            return *dynamic_cast<CustomCbInFormat*>(input.get());
		}
		auto& getBuffer() {
			return getInput().buffer;
		}
		auto* getBufPointer() {
			return reinterpret_cast<uint8_t*>(getBuffer().data());
		}

	private:
		bool fillFrame();
		void sendData();
	};

}    // namespace rtpsr