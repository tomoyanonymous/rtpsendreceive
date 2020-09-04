#pragma once
#include "rtpsrbase.hpp"
#include "lockfree_ringbuffer.hpp"

namespace rtpsr {
	struct RtpSender : public RtpSRBase {
		explicit RtpSender(RtpSRSetting& s, Url& url, Codec codec, std::ostream& logger = std::cerr);
		~RtpSender() {
			loopstate.active = false;
			// loopstate.future.wait();
			av_write_trailer(output->ctx);
			if (output->ctx->pb != nullptr) {
				avio_close(output->ctx->pb);
			}
			av_dict_free(&params);
			avformat_close_input(&input->ctx);
		}

		Encoder          encoder;
		AVDictionary*    params    = nullptr;
		int64_t          timecount = 0;
		std::string      url_tmp;
		std::future<int> wait_connection;
		AsyncLoopState   loopstate;
		// communication entrypoint between max
		LockFreeRingbuf<sample_t> input_buf;
		std::vector<sample_t>     framebuf;
		void                      sendData();
		void                      sendDataLoop();
		AsyncLoopState&           launchLoop();
		duration_type pollingrate  =duration_type(static_cast<double>(setting.framesize)*0.5*48000/ setting.samplerate);
		static void setCtxParams(AVDictionary** dict);
		auto&       getInput() {
            return *std::dynamic_pointer_cast<CustomCbInFormat>(input);
		}
		auto& getBuffer() {
			return getInput().buffer;
		}
		auto* getBufPointer() {
			return reinterpret_cast<uint8_t*>(getBuffer().data());
		}

	private:
		bool fillFrame();
	};

}    // namespace rtpsr